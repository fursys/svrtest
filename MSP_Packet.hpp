#pragma once

#include <iostream>


#define MSP_DEFAULT_PACKET_SIAZE 255
typedef enum {
    MSP_V1          = 0,
    MSP_V2_OVER_V1  = 1,
    MSP_V2_NATIVE   = 2,
    MSP_VERSION_COUNT
} mspVersion_e;

typedef enum {
    MSP_MSG_TYPE_REQUEST =   0x3C,
    MSP_MSG_TYPE_RESPONCE =  0x3E,
    MSP_MSG_TYPE_ERROR =     0x21
} mspMessageType_e;

typedef enum {
    MSP_FLAG_DONT_REPLY           = (1 << 0),
} mspFlags_e;

typedef enum {
    MSP_MSG_STATUS_NEW = 0,
    MSP_MSG_STATUS_PROCECCING = 1,
    MSP_MSG_STATUS_COMPLETED = 2,
} mspStatus_e;

// MSP parser states enum
typedef enum {
    MSP_IDLE,
    MSP_HEADER_START,
    MSP_HEADER_M,
    MSP_HEADER_X,

    MSP_HEADER_V1,
    MSP_PAYLOAD_V1,
    MSP_CHECKSUM_V1,

    MSP_HEADER_V2_OVER_V1,
    MSP_PAYLOAD_V2_OVER_V1,
    MSP_CHECKSUM_V2_OVER_V1,

    MSP_HEADER_V2_NATIVE,
    MSP_PAYLOAD_V2_NATIVE,
    MSP_CHECKSUM_V2_NATIVE,

    MSP_COMMAND_RECEIVED
} mspState_e;

class MSP_Packet
{
    private:
        uint8_t * ptr;
        uint8_t * payload_begin;
        uint8_t * payload_end;

        uint_fast16_t offset;
        uint_fast16_t dataSize;
        mspVersion_e mspVersion;
        uint8_t cmdFlags;
        uint16_t cmdMSP;
        uint8_t checksum1;
        uint8_t checksum2;

        mspStatus_e status;
        mspState_e c_state;

        
    public:
        uint8_t leadChar = '$';
        uint8_t protocolVersion = 'X'; // 'M' - for V1, 'X' - for V2
        MspMessageType_e MessageType;
        uint8_t flags;
        uint16_t cmd;
        
        uint8_t crc [2];
        int16_t result;
        MSP_Packet ();
        MSP_Packet (mspVersion_e version, mspMessageType_e msgtype, int buffSize);
        ~MSP_Packet();

        //sbuf_t getBuffer();
        int Lenght ();
        mspStatus_e GetStatus ();
        int AddBytes (uint8_t _buff, int _lenght);
        uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a)
};