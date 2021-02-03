#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <queue>

using namespace std;

#define MSG_DATA_TYPE int
#define PORT 7000
#define QUEUE 20
#define NET_MESSAGE_BUF_SIZE 1024


struct net_client_params
{
    string address;
    thread client_thread;
    bool exitflag;
    int conn;
};




void NetStartServer();
void NetStopServer();
void NetSetRcvQueue(queue<MSG_DATA_TYPE> * mQueue);
void NetSend (MSG_DATA_TYPE * msg);