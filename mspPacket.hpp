#pragma once

#include <iostream>
#include "msp.hpp"

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