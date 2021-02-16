#include "MSP_Packet.hpp"
#include <cstring>

MSP_Packet::MSP_Packet ()
{
    ptr = nullptr;
    c_state = MSP_IDLE;
}

MSP_Packet::MSP_Packet (uint16_t cmd, uint8_t * payload, uint16_t payloadSize, mspMessageType_e msgType)
{
    ptr = new uint8_t [payloadSize + 9];
    ptr[0] = '$';
    ptr[1] = 'X'; // MSP version 2
    ptr[2] = msgType;
    ptr[3] = 0;
    memcpy (ptr + 4, &cmd, 2);
    memcpy (ptr + 6, &payloadSize, 2);
    memcpy (ptr + 8, payload, payloadSize);
    checksum2 = crc8_dvb_s2_update(0, ptr+3, payloadSize+5);
    ptr[8+payloadSize] =  checksum2;

    dataSize = payloadSize;
    cmdMSP = cmd;
    mspVersion = MSP_V2_NATIVE;
    cmdFlags = 0;

    c_state = MSP_COMMAND_READY_TO_SEND;

}

MSP_Packet::~MSP_Packet()
{
    delete ptr;
}

bool MSP_Packet::SetMessageType (mspMessageType_e mType)
{
    if (ptr == nullptr) return false;
    ptr[2] = mType;
    return true;
}

uint8_t MSP_Packet::crc8_dvb_s2_update(uint8_t crc, const void *data, uint32_t length)
{
    const uint8_t *p = (const uint8_t *)data;
    const uint8_t *pend = p + length;

    for (; p != pend; p++) {
        crc = MSP_Packet::crc8_dvb_s2(crc, *p);
    }
    return crc;
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

mspState_e MSP_Packet::GetStatus()
{
    return c_state;
}

uint16_t MSP_Packet::GetPacketStream (uint8_t *& buf)
{
    if (ptr == nullptr) return 0;
    buf = ptr;
    return dataSize + 9;
}


int MSP_Packet::AddRcvBytes (uint8_t *_buff, int _lenght)
{
    if ((c_state == MSP_COMMAND_RECEIVED ) || (c_state == MSP_COMMAND_READY_TO_SEND)) return 0;

    bool exitFlag = false;
    int i = 0;
    while ((i != _lenght) && (!exitFlag))
    //for (int i = 0; i<_lenght;i++)
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
                case 'X':
                    c_state = MSP_HEADER_X;
                    mspVersion = MSP_V2_NATIVE;
                    break;
                default:
                    c_state = MSP_IDLE;
                    exitFlag = true;
                    break;
            }
            break;

        case MSP_HEADER_X:
            if ((c == '<') || (c == '>')) {
                offset = 0;
                checksum2 = 0;
                mspVersion = MSP_V2_NATIVE;
                c_state = MSP_HEADER_V2_NATIVE;
            }
            else {
                c_state = MSP_IDLE;
                exitFlag = true;
            }
            break;

        case MSP_HEADER_V2_NATIVE:
            headerBuff[offset++] = c;
            checksum2 = MSP_Packet::crc8_dvb_s2(checksum2, c);
            if (offset == sizeof(mspHeaderV2_t)) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&headerBuff[0];

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_MAX_PACKET_SAIZE) {
                    c_state = MSP_IDLE;
                    exitFlag = true;
                }
                else {
                    dataSize = hdrv2->size;
                    cmdMSP = hdrv2->cmd;
                    cmdFlags = hdrv2->flags;
                    offset = 8;                // re-use buffer
                    ptr = new uint8_t [dataSize+9];
                    ptr[0] = '$';
                    ptr[1] = 'X'; // MSP version 2
                    //ptr[2] = msgType;
                    memcpy (ptr + 3, headerBuff, 5);
                    if (dataSize > 0)
                    {
                        c_state = MSP_PAYLOAD_V2_NATIVE;
                    }
                    else
                    {
                        c_state = MSP_CHECKSUM_V2_NATIVE;
                    }
                }
            }
            break;

        case MSP_PAYLOAD_V2_NATIVE:
            checksum2 = MSP_Packet::crc8_dvb_s2(checksum2, c);
            ptr[offset++] = c;

            if (offset == dataSize) {
                c_state = MSP_CHECKSUM_V2_NATIVE;
            }
            break;

        case MSP_CHECKSUM_V2_NATIVE:
            if (checksum2 == c) {
                c_state = MSP_COMMAND_RECEIVED;
                ptr[8+dataSize] =  checksum2;
            } else {
                c_state = MSP_IDLE;
            }
            exitFlag = true;
            break;
        }
        i++;
    }
    return i;
}