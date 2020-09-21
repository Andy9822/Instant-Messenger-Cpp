#include "../include/server.h"



/*Server::Server()
{
	sockfd = 0;
}*/



int prepareConnection(struct sockaddr_in serv_addr)
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ 
        printf("ERROR opening socket\n");
        return -1;
	}

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
		printf("ERROR on binding\n");
		return -1;
    }

	if (listen(sockfd, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
	{ 
		printf("ERROR on listening\n");
		return -1;
	}

    return 0;
}



int printPortNumber(struct sockaddr_in serv_addr)
{
	socklen_t len = sizeof(serv_addr);
	
	if(getsockname(sockfd, (struct sockaddr *)&serv_addr, &len) < 0)
	{
		std::cout << "Unable to print port Number!" << std::endl;
		return -1;
	}
	
	std::cout << "Using PORT " << ntohs(serv_addr.sin_port) << std::endl;

	return 0;
}



void* clientCommunication(void *newsocket)
{
	char buffer[256];
	int n;
	int newsockfd = *(int *)newsocket;

	while(1)
	{
		bzero(buffer, 256);
		n = read(newsockfd, buffer, 256);
		
		if (n < 0) 
			printf("ERROR reading from socket\n");
		else if(n == 0)
		{
			std::cout << "End of connection with socket " << newsockfd << std::endl;
			break;
		}
	
		std::cout << "Here is the message: " << buffer << std::endl;
		
		n = write(newsockfd,"I got your message\n", 18);
		
		if (n < 0) 
			printf("ERROR writing to socket\n");
	}

	close(newsockfd);

	return 0;
}




int ConnectToClient(pthread_t *tid)
{
	int newsockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
	{
		printf("ERROR on accept\n");
		return -1;
	}

	pthread_create(tid, NULL, clientCommunication , &newsockfd);

	return 0;
}








int main()
{
	int i = 0; 
	struct sockaddr_in serv_addr;

	pthread_t tid[SOMAXCONN];
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = 0;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
  

	prepareConnection(serv_addr);

	printPortNumber(serv_addr);

	while(1)
		ConnectToClient(&tid[i++]);

	close(sockfd);
	
	return 0; 
}