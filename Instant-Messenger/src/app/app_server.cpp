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

void read_args(int argc, char *argv[], int *port, int *maxNumberOfMessagesInHistory, bool *isPrimaryServer)
{	
	*port = RANDOM_PORT_NUMBER; //Set any available port as default
	*maxNumberOfMessagesInHistory = DEFAULT_NUMBER_OF_RECORDED_MESSAGES;
	*isPrimaryServer = IS_PRIMARY_SERVER;

	if(argc == 4)
    {
			try {
				*port = stoi(argv[1]); // In case it's received a specific port as argv
                *maxNumberOfMessagesInHistory = stoi(argv[2]);
                *isPrimaryServer = stoi(argv[3]);
                cout << "Is Primary Server? " << *isPrimaryServer << endl;
			}
			catch(std::invalid_argument e) {
                cout << "[ERROR] Invalid arguments" << endl;
			}
    } else {
	    cout << "[WARNING] using random port, " << DEFAULT_NUMBER_OF_RECORDED_MESSAGES << " messages on history and server is backup" << endl;
	}
}

int main(int argc, char *argv[])
{
    int i = 0;
    int port, maxNumberOfMessagesInHistory;
    bool isPrimaryServer;
    // Capture and process SO signals
	capture_signals();

	pthread_t tid[MAXBACKLOG];

	read_args(argc, argv, &port, &maxNumberOfMessagesInHistory, &isPrimaryServer);
    serverApp.setPort(port);
    serverApp.setPrimaryServer(isPrimaryServer);
    serverApp.configureFilesystemManager(maxNumberOfMessagesInHistory);
    serverApp.prepareConnection();
    serverApp.printPortNumber();

    if(serverApp.getPrimaryServer())
    {
        cout << "starting connections with backup server" << endl;
        serverApp.connectToBackupServers();
    }

	while(1)
	{
		if(serverApp.handleClientConnection(&tid[i++]) < 0)
			return -1;
	}

	return 0;
}
