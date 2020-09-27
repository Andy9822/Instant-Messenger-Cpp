#include "../../include/server/server.h"

//#define PORT 4040

int main()
{

	Server server;
	pthread_t tid[MAXBACKLOG];
	int i = 0;

	server.prepareConnection();
	server.printPortNumber();

	while(1)
	{
		if(server.ConnectToClient(&tid[i++]) < 0)
			return -1;
	}

	return 0;
}
