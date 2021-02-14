#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <queue>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define NET_CLIENT_CON_BUF_SIZE 1024

class NetClientCon
{
    private:
        string address;
        thread client_thread;
        bool exitflag;
        int conn;

        void ReaderThreadProc ();
    public:
        NetClientCon ();
        NetClientCon (string _address, int conn);
        ~NetClientCon();

        bool Status ();
};