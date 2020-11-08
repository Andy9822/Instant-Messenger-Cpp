
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../../include/proxy_fe/ProxyFE.hpp"
#include "../../include/util/definitions.hpp"

using namespace std;


ProxyFE proxy_fe;
struct sigaction sigIntHandler;

void my_handler(int signal){
	if (signal == 2)
	{
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

void read_args(int argc, char *argv[], int *portServerRM, int *portClients)
{	
	*portServerRM = RANDOM_PORT_NUMBER; //Set any available port as default
	*portClients = RANDOM_PORT_NUMBER; //Set any available port as default

	if(argc == 3)
    {
			try {
				*portServerRM = stoi(argv[1]); // In case it's received a specific port as argv
			}
			catch(std::invalid_argument e) {
                cout << "[ERROR] Invalid arguments" << endl;
			}

			try {
				*portClients = stoi(argv[2]); // In case it's received a specific port as argv
			}
			catch(std::invalid_argument e) {
                cout << "[ERROR] Invalid arguments" << endl;
			}
    } else {
	    cout << "[WARNING] using random port" << endl;
	}
}

int main(int argc, char *argv[])
{
    int i = 0;
    int portServerRM, portClients;
	pthread_t tid[MAXBACKLOG];
    
	// Capture and process SO signals
	capture_signals();

	read_args(argc, argv, &portServerRM, &portClients);
    proxy_fe.setPortServerRM(portServerRM);
    proxy_fe.setPortClients(portClients);
    proxy_fe.prepareConnection();
    proxy_fe.printPortNumber();

	// Wait for initial server connection
	proxy_fe.handleServerConnection(&tid[i++]);
	
	// Trigger Server Reconnection processment in case initial connection downs
	proxy_fe.handleServerReconnect(&tid[i++]);

	// Trigger processing message consumer thread
	proxy_fe.activateMessageConsumer(&tid[i++]);
	
	while(true)
	{
		proxy_fe.handleClientConnection(&tid[i++]);
	}

	return 0;
}
