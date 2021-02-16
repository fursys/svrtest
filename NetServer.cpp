#include "NetServer.hpp"

NetServer::NetServer()
{
    ss = socket(AF_INET, SOCK_STREAM, 0);
    
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT);

    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);



    if(bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))==-1) {
        perror("bind");
        exit(1);
    }
    if(listen(ss, NET_SERVER_CLIENTS_QUEUE) == -1) {
        perror("listen");
        exit(1);
    }

    NetConnectorThread = std::thread(&NetServer::ConnectorThreadProc, this);
}

NetServer::~NetServer()
{
    cout << "Stop network server task" << endl;
    ConnectionExitFlag = true;
    NetConnectorThread.join();

    //iterate all items in active_clients_list and stop threads
    list<NetClientCon*>::iterator p = active_clients_list.begin();
    while(p != active_clients_list.end()) {
        delete (*p);
        p++;
    }

    active_clients_list.clear();
}

void NetServer::ConnectorThreadProc()
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
            list<NetClientCon*>::iterator p = active_clients_list.begin();
            while (p != active_clients_list.end()) {
                if ((*p)->isRunning() == false)
                {
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

                NetClientCon * nClient = new NetClientCon (strClientAdress, conn);
                nClient->SetRcvQueue(rcvMsgQueue);
                active_clients_list.push_back (nClient);


                printf("Waiting for clients\n");
            }
        }

    }
    close(ss);
}

void NetServer::SetRcvQueue (SafeQueue<MSP_Packet> * q)
{
    rcvMsgQueue = q;
}