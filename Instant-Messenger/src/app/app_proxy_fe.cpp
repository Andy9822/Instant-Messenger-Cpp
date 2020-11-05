
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

void read_args(int argc, char *argv[], int *port, int *maxNumberOfMessagesInHistory)
{	
	*port = RANDOM_PORT_NUMBER; //Set any available port as default
	*maxNumberOfMessagesInHistory = DEFAULT_NUMBER_OF_RECORDED_MESSAGES;

	if(argc == 2)
    {
			try {
				*port = stoi(argv[1]); // In case it's received a specific port as argv
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
    int port, maxNumberOfMessagesInHistory;
    // Capture and process SO signals
	capture_signals();

	pthread_t tid[MAXBACKLOG];

	read_args(argc, argv, &port, &maxNumberOfMessagesInHistory);
    proxy_fe.setPort(port);
    proxy_fe.prepareConnection();
    proxy_fe.printPortNumber();

	//while(1)
	{
		if(proxy_fe.handleServerConnection() < 0)
			return -1;
		
		proxy_fe.monitorKeepAlives();
		sleep(60);
	}

	return 0;
}
