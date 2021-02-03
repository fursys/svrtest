//Server

#include <iostream>
#include "drone_sockets.hpp"
#include <thread>


int main() {

    NetStartServer();
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
            NetStopServer();
            exitflag = true;
        }
    }
    
    return 0;
}