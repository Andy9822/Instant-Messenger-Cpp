#include "../../include/server/server.hpp"
#include "../../include/server/ServersRing.hpp"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>

//#define PORT 4040

using namespace std;
using namespace server;
using namespace servers_ring;

ServersRing serverRing;
Server serverApp;
struct sigaction sigIntHandler;

void my_handler(int signal){
	if (signal == 2)
	{
        serverApp.closeServer();
		exit(2);
	}	
}

void capture_signals()
{
	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
}

void read_args(int argc, char *argv[], int *maxNumberOfMessagesInHistory, int *rmNumber, bool *isPrimaryServer)
{
	*maxNumberOfMessagesInHistory = DEFAULT_NUMBER_OF_RECORDED_MESSAGES;

	if(argc == 4)
    {
			try {
                *maxNumberOfMessagesInHistory = stoi(argv[1]);
                *rmNumber = stoi(argv[2]);
                *isPrimaryServer = stoi(argv[3]);
			}
			catch(std::invalid_argument e) {
                cout << "[ERROR] Invalid arguments" << endl;
			}
    } else {
	    cout << "[WARNING] using random port and " << DEFAULT_NUMBER_OF_RECORDED_MESSAGES << "messages on history" << endl;
	}
}

int main(int argc, char *argv[])
{
    int maxNumberOfMessagesInHistory;
    int rmNumber;
    bool isPrimaryServer;

    // TODO: change this to a parameter o configuration file
    vector<int> fePortList{ 6969, 6970, 6971 };
    vector<std::string> feAddressList {"127.0.0.1", "127.0.0.1", "127.0.0.1"};

    // Capture and process SO signals
	capture_signals();

    serverRing = ServersRing();
    serverRing.connectServersRing(serverApp);

	read_args(argc, argv, &maxNumberOfMessagesInHistory, &rmNumber, &isPrimaryServer);

	if ( fePortList.size() != feAddressList.size() ) {
	    cout << "[ERROR] FE address and socket list need to have the same size";
	    return -1;
	}

	if(isPrimaryServer) {
        for (int i = 0; i < fePortList.size(); i++) {
            cout << "[DEBUG] I'll connect to FE address:port " << feAddressList.at(i) << "/" << fePortList.at(i) << endl;
            serverApp.connectToFE(feAddressList.at(i), fePortList.at(i));
        }
	}

    cout << "[DEBUG] number of sockets waiting for connection " << serverApp.socketFeList.size() << endl;
    serverApp.handleFrontEndsConnections();

    Server::isPrimaryServer = isPrimaryServer;
    serverApp.prepareReplicationManager(rmNumber);
	
	//I'm not proud of this, I swear
	while (true)
	{
		sleep(60);
	}

	return 0;
}
