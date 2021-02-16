#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "safequeue.hpp"
#include "MSP_Packet.hpp"

using namespace std;

#define NET_CLIENT_CON_BUF_SIZE 1024

class NetClientCon
{
    private:
        string address;
        thread client_thread;
        bool exitflag;
        int conn;
        SafeQueue<MSP_Packet> * rcvMsgQueue;

        void ReaderThreadProc ();
    public:
        NetClientCon ();
        NetClientCon (string _address, int conn);
        ~NetClientCon();

        void SetRcvQueue (SafeQueue<MSP_Packet> * q);

        bool isRunning ();
};