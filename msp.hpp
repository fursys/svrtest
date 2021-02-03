/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "streambuf.h"

#define MSP_V2_FRAME_ID         255

typedef enum {
    MSP_V1          = 0,
    MSP_V2_OVER_V1  = 1,
    MSP_V2_NATIVE   = 2,
    MSP_VERSION_COUNT
} mspVersion_e;

#define MSP_VERSION_MAGIC_INITIALIZER { 'M', 'M', 'X' }

//#define MSP_MSG_TYPE_REQUEST    0x3C
//#define MSP_MSG_TYPE_RESPONCE   0x3E
//#define MSP_MSG_TYPE_ERROR      0x21

typedef enum {
    MSP_MSG_TYPE_REQUEST =   0x3C,
    MSP_MSG_TYPE_RESPONCE =  0x3E,
    MSP_MSG_TYPE_ERROR =     0x21
} MspMessageType_e;


// return positive for ACK, negative on error, zero for no reply
typedef enum {
    MSP_RESULT_ACK = 1,
    MSP_RESULT_ERROR = -1,
    MSP_RESULT_NO_REPLY = 0
} mspResult_e;

typedef struct mspPacket_s {
    sbuf_t buf;
    uint16_t cmd;
    uint8_t flags;
    int16_t result;
} mspPacket_t;

typedef enum {
    MSP_FLAG_DONT_REPLY           = (1 << 0),
} mspFlags_e;

struct serialPort_s;
typedef void (*mspPostProcessFnPtr)(struct serialPort_s *port); // msp post process function, used for gracefully handling reboots, etc.
typedef mspResult_e (*mspProcessCommandFnPtr)(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn);


class mspPacket_c
{
    private:
        uint8_t * buff_beg;
        uint8_t * buff_end;
        uint8_t * payload_begin;
        uint8_t * payload_end;
        uint8_t * crc_ptr;
    public:
        uint8_t leadChar = '$';
        uint8_t protocolVersion = 'X'; // 'M' - for V1, 'X' - for V2
        MspMessageType_e MessageType;
        uint8_t flags;
        uint16_t cmd;
        
        uint8_t crc [2];
        int16_t result;

        mspPacket_c (int buffSize);
        mspPacket_c (uint8_t * beg, int len);
        mspPacket_c (uint8_t * beg, uint8_t * end);
        ~mspPacket_c();

        sbuf_t getBuffer();
        int Lenght ();
};
