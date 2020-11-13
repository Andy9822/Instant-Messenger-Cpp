#include "../../include/server/server.hpp"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//#define PORT 4040

using namespace std;
using namespace server;

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

void read_args(int argc, char *argv[], int *port, int *maxNumberOfMessagesInHistory, int *rmNumber, bool *isPrimaryServer)
{	
	*port = RANDOM_PORT_NUMBER; //Set any available port as default
	*maxNumberOfMessagesInHistory = DEFAULT_NUMBER_OF_RECORDED_MESSAGES;

	if(argc == 5)
    {
			try {
				*port = stoi(argv[1]); // In case it's received a specific port as argv
                *maxNumberOfMessagesInHistory = stoi(argv[2]);
                *rmNumber = stoi(argv[3]);
                *isPrimaryServer = stoi(argv[4]);
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
    int i = 0;
    int port, maxNumberOfMessagesInHistory;
    int rmNumber;
    bool isPrimaryServer;
    // Capture and process SO signals
	capture_signals();

	pthread_t tid[MAXBACKLOG];

	read_args(argc, argv, &port, &maxNumberOfMessagesInHistory, &rmNumber, &isPrimaryServer);
    // serverApp.setPort(port);
    // serverApp.configureFilesystemManager(maxNumberOfMessagesInHistory);
    // serverApp.prepareConnection();
    // serverApp.printPortNumber();

	// while(1)
	// {
	// 	if(serverApp.handleFrontEndConnection(&tid[i++]) < 0)
	// 		return -1;
	// }

	//TODO sorry for the mess, WIP


	//serverApp.ConnectToFE();
	serverApp.setIsPrimaryServer(isPrimaryServer);
	serverApp.prepareReplicationManager(rmNumber);
    serverApp.handleFrontEndConnection(&tid[i++], &tid[i++]);

	//I'm not proud of this, I swear
	while (true)
	{
		sleep(60);
	}
	

	return 0;
}
