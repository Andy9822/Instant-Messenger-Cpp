#include "../../include/server/server.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//#define PORT 4040

using namespace std;

Server server;
struct sigaction sigIntHandler;

void my_handler(int signal){
	if (signal == 2)
	{
		//TODO have to investigate why even closing sockets still hang outs if there are open connections
		server.closeServer();
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

int read_port(int argc, char *argv[])
{	
	int port = 0; //Set any available port as default

	if(argc == 2)
    {
			try {
				port = stoi(argv[1]); // In case it's received a specific port as argv
			}
			catch(std::invalid_argument e) {
			}
    }

	return port;
}

int main(int argc, char *argv[])
{
	// Capture and process SO signals
	capture_signals();

	pthread_t tid[MAXBACKLOG];
	int i = 0;


	int port = read_port(argc, argv);
	server.setPort(port);
	server.prepareConnection();
	server.printPortNumber();

	while(1)
	{
		if(server.handleClientConnection(&tid[i++]) < 0)
			return -1;
	}

	return 0;
}
