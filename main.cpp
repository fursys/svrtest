//Server

#include <iostream>
//#include "drone_sockets.hpp"
#include <thread>
#include "NetServer.hpp"
#include <iomanip>

SafeQueue<MSP_Packet> rcvQueue;
std::thread mspReceiver;

bool exitflag = false;
void MSPReceiverProc ();


int main() {

    //NetStartServer();
    mspReceiver = std::thread(MSPReceiverProc);

    NetServer * srv = new NetServer();
    srv->SetRcvQueue(&rcvQueue);

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
    rcvQueue.push (new MSP_Packet());
    mspReceiver.join();
    return 0;
}

void MSPReceiverProc ()
{
    MSP_Packet * msg;
    uint8_t * buff;
    uint16_t len;
    //std::cout << "MSP Receiver proc" << std::endl;
    while (!exitflag)
    {
        msg = rcvQueue.pop();
        len = msg->GetPacketStream(buff);
        if (len >0)
        {

            // Hex print and restore default afterwards.
            std::ios cout_state(nullptr);
            cout_state.copyfmt(std::cout);
            std::cout << std::hex << std::setfill('0') << std::setw(2);
            for (size_t i = 0; i < len; ++i)
                std::cout << (int) buff[i] << " ";
            std::cout << std::endl;
            std::cout.copyfmt(cout_state);
        }


    }
}