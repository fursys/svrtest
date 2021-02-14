#include "MSP_Packet.hpp"

MSP_Packet::MSP_Packet ()
{
    status = MSP_MSG_STATUS_NEW;
    ptr = nullptr;
}

MSP_Packet::MSP_Packet (int payloadSize)
{
    ptr = new uint8_t [buffsize + 9];
    payload_begin = ptr + 8;
    payload_end = ptr + 7 + buffsize;
    crc_ptr = payload_end + 1;
}

MSP_Packet::~MSP_Packet()
{
    delete ptr;
}

uint8_t MSP_Packet::crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}


int MSP_Packet::AddBytes (uint8_t _buff, int _lenght)
{
    for (int i = 0; i<_lenght;i++)
    {
        uint8_t c = _buff [i];
        
        switch (c_state) {
        default:
        case MSP_IDLE:      // Waiting for '$' character
            if (c == '$') {
                c_state = MSP_HEADER_START;
            }
            break;

        case MSP_HEADER_START:  // Waiting for 'M' (MSPv1 / MSPv2_over_v1) or 'X' (MSPv2 native)
            switch (c) {
                case 'M':
                    c_state = MSP_HEADER_M;
                    mspVersion = MSP_V1;
                    break;
                case 'X':
                    c_state = MSP_HEADER_X;
                    mspVersion = MSP_V2_NATIVE;
                    break;
                default:
                    c_state = MSP_IDLE;
                    break;
            }
            break;

        case MSP_HEADER_M:      // Waiting for '<'
            if (c == '<') {
                offset = 0;
                checksum1 = 0;
                checksum2 = 0;
                c_state = MSP_HEADER_V1;
            }
            else {
                c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_X:
            if (c == '<') {
                offset = 0;
                checksum2 = 0;
                mspVersion = MSP_V2_NATIVE;
                c_state = MSP_HEADER_V2_NATIVE;
            }
            else {
                c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V1:     // Now receive v1 header (size/cmd), this is already checksummable
            inBuf[offset++] = c;
            checksum1 ^= c;
            if (offset == sizeof(mspHeaderV1_t)) {
                mspHeaderV1_t * hdr = (mspHeaderV1_t *)&inBuf[0];
                // Check incoming buffer size limit
                if (hdr->size > MSP_PORT_INBUF_SIZE) {
                    c_state = MSP_IDLE;
                }
                else if (hdr->cmd == MSP_V2_FRAME_ID) {
                    // MSPv1 payload must be big enough to hold V2 header + extra checksum
                    if (hdr->size >= sizeof(mspHeaderV2_t) + 1) {
                        mspVersion = MSP_V2_OVER_V1;
                        c_state = MSP_HEADER_V2_OVER_V1;
                    }
                    else {
                        c_state = MSP_IDLE;
                    }
                }
                else {
                    dataSize = hdr->size;
                    cmdMSP = hdr->cmd;
                    cmdFlags = 0;
                    offset = 0;                // re-use buffer
                    c_state = dataSize > 0 ? MSP_PAYLOAD_V1 : MSP_CHECKSUM_V1;    // If no payload - jump to checksum byte
                }
            }
            break;

        case MSP_PAYLOAD_V1:
            inBuf[offset++] = c;
            checksum1 ^= c;
            if (offset == dataSize) {
                c_state = MSP_CHECKSUM_V1;
            }
            break;

        case MSP_CHECKSUM_V1:
            if (checksum1 == c) {
                c_state = MSP_COMMAND_RECEIVED;
            } else {
                c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V2_OVER_V1:     // V2 header is part of V1 payload - we need to calculate both checksums now
            inBuf[offset++] = c;
            checksum1 ^= c;
            checksum2 = MSP_Packet::crc8_dvb_s2(checksum2, c);
            if (offset == (sizeof(mspHeaderV2_t) + sizeof(mspHeaderV1_t))) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&inBuf[sizeof(mspHeaderV1_t)];
                dataSize = hdrv2->size;

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_PORT_INBUF_SIZE) {
                    c_state = MSP_IDLE;
                }
                else {
                    cmdMSP = hdrv2->cmd;
                    cmdFlags = hdrv2->flags;
                    offset = 0;                // re-use buffer
                    c_state = dataSize > 0 ? MSP_PAYLOAD_V2_OVER_V1 : MSP_CHECKSUM_V2_OVER_V1;
                }
            }
            break;

        case MSP_PAYLOAD_V2_OVER_V1:
            checksum2 = MSP_Packet::crc8_dvb_s2(checksum2, c);
            checksum1 ^= c;
            inBuf[offset++] = c;

            if (offset == dataSize) {
                c_state = MSP_CHECKSUM_V2_OVER_V1;
            }
            break;

        case MSP_CHECKSUM_V2_OVER_V1:
            checksum1 ^= c;
            if (checksum2 == c) {
                c_state = MSP_CHECKSUM_V1; // Checksum 2 correct - verify v1 checksum
            } else {
                c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V2_NATIVE:
            //inBuf[offset++] = c;
            checksum2 = MSP_Packet::crc8_dvb_s2(checksum2, c);
            if (offset == sizeof(mspHeaderV2_t)) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&inBuf[0];

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_PORT_INBUF_SIZE) {
                    c_state = MSP_IDLE;
                }
                else {
                    dataSize = hdrv2->size;
                    cmdMSP = hdrv2->cmd;
                    cmdFlags = hdrv2->flags;
                    offset = 0;                // re-use buffer
                    c_state = dataSize > 0 ? MSP_PAYLOAD_V2_NATIVE : MSP_CHECKSUM_V2_NATIVE;
                }
            }
            break;

        case MSP_PAYLOAD_V2_NATIVE:
            checksum2 = MSP_Packet::crc8_dvb_s2(checksum2, c);
            inBuf[offset++] = c;

            if (offset == dataSize) {
                c_state = MSP_CHECKSUM_V2_NATIVE;
            }
            break;

        case MSP_CHECKSUM_V2_NATIVE:
            if (checksum2 == c) {
                c_state = MSP_COMMAND_RECEIVED;
            } else {
                c_state = MSP_IDLE;
            }
            break;
        }
    }
}