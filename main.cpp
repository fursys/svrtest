//Server

#include <iostream>
//#include "drone_sockets.hpp"
#include <thread>
#include "NetServer.hpp"


int main() {

    //NetStartServer();
    NetServer * srv = new NetServer();
    
    bool exitflag = false;

    while (!exitflag)
    {
        char n;
        /* code */
        //if (std::cin)
        //std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "Enter n: "; // no flush needed
        std::cin >> n;
        if (n == 'q') 
        {
            //NetStopServer();

            exitflag = true;
        }
    }
    delete (srv);
    
    return 0;
}