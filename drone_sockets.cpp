#include "drone_sockets.hpp"
//#include <sys/types.h>
//#include <stdio.h>
//#include <netinet/in.h>
//#include <stdlib.h>
//#include <fcntl.h>
//#include <sys/shm.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <list>

using namespace std;

std::list<net_client_params*> active_clients_list;

struct sockaddr_in server_sockaddr;
int ss;
thread NetConnectorThread;
bool ConnectionExitFlag = false;
queue<MSG_DATA_TYPE> * send_queue;


void ReaderThreadProc (net_client_params * cl_params);
void NetSetRcvQueue(queue<MSG_DATA_TYPE> * mQueue)
{
    send_queue = mQueue;
}

void ConnectorThreadProc()
{
    fd_set socket_fd_set;
    int retval, maxfd;
    struct timeval tv;
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    printf("Waiting for clients\n");
    while (!ConnectionExitFlag)
    {
        //printf("Waiting for clients\n");
        FD_ZERO(&socket_fd_set);
        FD_SET(ss, &socket_fd_set);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(ss+1, &socket_fd_set, NULL, NULL, &tv);
        if(retval == -1)
        {
            printf("Select Error\n");
            continue;
        }
        else if (retval == 0)
        {
            //Garbage collection, remove 
            list<net_client_params*>::iterator p = active_clients_list.begin();
            while (p != active_clients_list.end()) {
                if ((*p)->exitflag == true)
                {
                    (*p)->client_thread.join();
                    delete (*p);
                    p=active_clients_list.erase(p);
                }
                else p++;
            }
        }
        else if(retval > 0)
        {
            if(FD_ISSET(ss,&socket_fd_set))
            {
                cout << "Accepting new client." << endl;
                int conn = accept(ss, (struct sockaddr*)&client_addr, &length);
                if( conn < 0 ) {
                    perror("connect");
                    continue;
                }
                
                // Print client address
                char strClientAdress[INET_ADDRSTRLEN]; 
                inet_ntop(AF_INET, &(client_addr.sin_addr), strClientAdress, INET_ADDRSTRLEN);
                printf("Client %s connected.\n", strClientAdress);
                //======================

                net_client_params * nClient = new net_client_params;
                nClient->address = strClientAdress;
                nClient->conn = conn;
                nClient->exitflag = false;
                nClient->client_thread = std::thread(ReaderThreadProc, nClient);
                active_clients_list.push_back (nClient);


                printf("Waiting for clients\n");
            }
        }

    }
    close(ss);
}

void ReaderThreadProc (net_client_params * cl_params)
{
    
    cout << "Tread for client " << cl_params->address << " started." << endl; 
    bool selfExitFlag = false;
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd;
    char buffer[NET_MESSAGE_BUF_SIZE];
    while(!cl_params->exitflag) {
        /*Clear the collection of readable file descriptors*/
        FD_ZERO(&rfds);
        /*Add standard input file descriptors to the collection*/
        //FD_SET(0, &rfds);
        maxfd = 0;
        /*Add the currently connected file descriptor to the collection*/
        FD_SET(cl_params->conn, &rfds);
        /*Find the largest file descriptor in the file descriptor set*/    
        if(maxfd < cl_params->conn)
            maxfd = cl_params->conn;
        /*Setting timeout time*/
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        /*Waiting for chat*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if(retval == -1){
            printf("select Error, client program exit\n");
            cl_params->exitflag = true;
            selfExitFlag = true;
        }else if(retval == 0){
            //printf("Waiting client message...\n");
            continue;
        }
        else
        {
            /*The client sent a message.*/
            if(FD_ISSET(cl_params->conn,&rfds)){
                memset(buffer, 0 ,sizeof(buffer));
                int len = recv(cl_params->conn, buffer, sizeof(buffer), 0);
                if (len == 0)
                {
                    FD_CLR (cl_params->conn,&rfds);
                    cl_params->exitflag = true;
                    selfExitFlag = true;
                    cout << "Client " << cl_params->address << " disconnected." << endl;
                    //printf("Client %s disconnected.\n", cl_params->address);
                }
                else 
                {
                    //TODO: Proceed received data
                    printf("%s\n", buffer);
                }
            }
        }
    }
    close(cl_params->conn);
    //if (selfExitFlag) RemoveActiveClient (cl_params);
}

void NetStopServer()
{
    cout << "Stop network server task" << endl;
    ConnectionExitFlag = true;
    NetConnectorThread.join();

    //iterate all items in active_clients_list and stop threads
    list<net_client_params*>::iterator p = active_clients_list.begin();
    while(p != active_clients_list.end()) {
        (*p)->exitflag = true;
        (*p)->client_thread.join();
        delete (*p);
        p++;
    }

    active_clients_list.clear();
}
void NetStartServer()
{

    ss = socket(AF_INET, SOCK_STREAM, 0);
    
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT);
    //printf("%d\n",INADDR_ANY);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);



    if(bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))==-1) {
        perror("bind");
        exit(1);
    }
    if(listen(ss, QUEUE) == -1) {
        perror("listen");
        exit(1);
    }

    NetConnectorThread = std::thread(ConnectorThreadProc);
}