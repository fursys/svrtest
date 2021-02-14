#include "NetClientCon.hpp"



NetClientCon::NetClientCon()
{

}
NetClientCon::NetClientCon(string _address, int _conn)
{
    address = _address;
    conn = _conn;
    client_thread = std::thread(&NetClientCon::ReaderThreadProc, this);
}
NetClientCon::~NetClientCon()
{
    exitflag = true;
    client_thread.join();
    
}

void NetClientCon::ReaderThreadProc ()
{
    
    cout << "Tread for client " << address << " started." << endl; 
    bool selfExitFlag = false;
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd;
    char buffer[NET_CLIENT_CON_BUF_SIZE];
    while(!exitflag) {
        /*Clear the collection of readable file descriptors*/
        FD_ZERO(&rfds);
        /*Add standard input file descriptors to the collection*/
        //FD_SET(0, &rfds);
        maxfd = 0;
        /*Add the currently connected file descriptor to the collection*/
        FD_SET(conn, &rfds);
        /*Find the largest file descriptor in the file descriptor set*/    
        if(maxfd < conn)
            maxfd = conn;
        /*Setting timeout time*/
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        /*Waiting for chat*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if(retval == -1){
            printf("select Error, client program exit\n");
            exitflag = true;
            selfExitFlag = true;
        }else if(retval == 0){
            //printf("Waiting client message...\n");
            continue;
        }
        else
        {
            /*The client sent a message.*/
            if(FD_ISSET(conn,&rfds)){
                memset(buffer, 0 ,sizeof(buffer));
                int len = recv(conn, buffer, sizeof(buffer), 0);
                if (len == 0)
                {
                    FD_CLR (conn,&rfds);
                    exitflag = true;
                    selfExitFlag = true;
                    cout << "Client " << address << " disconnected." << endl;
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
    close(conn);
    //if (selfExitFlag) RemoveActiveClient (cl_params);
}

bool NetClientCon::Status ()
{
    return exitflag;
}