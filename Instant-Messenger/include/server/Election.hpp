#ifndef ELECTION_HPP
#define ELECTION_HPP


//#include "ServersRing.hpp"
#include "../../include/util/Socket.hpp"
#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
//#include <fstream>
#include <sstream>
#include <string>

using namespace std;

//using namespace election;

namespace election {
    class Election : public Socket {
    
    private:
    	bool isParticipating;


    public:
        Election();
        void startElection(int server_ID, int clientSocket);
        void sendMessageToNextServer(int server_ID, int clientSocket, string status = "ELECTION");
        Packet buildPacket(int server_ID, string status);
        int processElectionInfo(string Receivedmessage, int server_ID, int sockfd);
    };
}




















#endif
