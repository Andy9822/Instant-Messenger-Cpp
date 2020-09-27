#include "../../include/server/server.h"

//#define PORT 4040

using namespace std;

Server server;

int read_port(int argc, char *argv[])
{	int port = 0; //Set any available port as default

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

	Server server;
	pthread_t tid[MAXBACKLOG];
	int i = 0;


	int port = read_port(argc, argv); //Set default port to 0
	server.setPort(port);
	server.prepareConnection();
	server.printPortNumber();

	while(1)
	{
		if(server.ConnectToClient(&tid[i++]) < 0)
			return -1;
	}

	return 0;
}
