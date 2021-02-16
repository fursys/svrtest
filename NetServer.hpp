#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <queue>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <list>

#include "NetClientCon.hpp"
#include "safequeue.hpp"

#define NET_SERVER_CLIENTS_QUEUE 20
#define PORT 7000

class NetServer
{
    private:
        std::list<NetClientCon*> active_clients_list;
        struct sockaddr_in server_sockaddr;
        int ss;
        thread NetConnectorThread;
        bool ConnectionExitFlag = false;

        SafeQueue<MSP_Packet> * rcvMsgQueue;

        void ConnectorThreadProc();
    public:
    NetServer();
    ~NetServer();

    void SetRcvQueue (SafeQueue<MSP_Packet> * q);
};