#include "../../include/server/server.h"



int main()
{
	int i = 0; 
	struct sockaddr_in serv_addr;
	Server server;

	pthread_t tid[SOMAXCONN];
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = 0;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
  

	if(server.prepareConnection(serv_addr) < 0)
		return -1;

	if(server.printPortNumber(serv_addr) < 0)
		return -1;

	while(1)
	{
		if(server.ConnectToClient(&tid[i++]) < 0)
			return -1;
	}
	
	return 0; 
}