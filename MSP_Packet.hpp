#pragma once

#include <iostream>
#include <unistd.h>

#define MSP_V2_FRAME_ID         255
#define MSP_MAX_PACKET_SAIZE 255

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

    MSP_COMMAND_RECEIVED,
    MSP_COMMAND_READY_TO_SEND
} mspState_e;

typedef struct __attribute__((packed)) {
    uint8_t  flags;
    uint16_t cmd;
    uint16_t size;
} mspHeaderV2_t;

class MSP_Packet
{
    private:
        uint8_t * ptr;
        uint8_t headerBuff[8];
        uint8_t msgType = 0;
        uint_fast16_t offset;
        uint_fast16_t dataSize;
        mspVersion_e mspVersion;
        uint8_t cmdFlags;
        uint16_t cmdMSP;
        uint8_t checksum2;
        int conn;
        mspState_e c_state;

        uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
        uint8_t crc8_dvb_s2_update(uint8_t crc, const void *data, uint32_t length);
        
    public:

        MSP_Packet ();
        MSP_Packet (int connection);
        MSP_Packet (uint16_t cmd, uint8_t * payload, uint16_t payloadSize, mspMessageType_e msgType);
        ~MSP_Packet();

        mspState_e GetStatus();
        uint16_t GetPacketStream (uint8_t *& buf);
        bool SetMessageType (mspMessageType_e mType);
        int AddRcvBytes (uint8_t * _buff, int _lenght);
        uint16_t GetCommand();
        void SendPacket();
        void Clear ();
        void Fill (uint16_t cmd, uint8_t * payload, uint16_t payloadSize, mspMessageType_e msgType);

};