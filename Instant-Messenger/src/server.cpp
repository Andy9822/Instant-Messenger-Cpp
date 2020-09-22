#include "../include/server.h"



Server::Server()
{
	sockfd = 0;
}



int Server::prepareConnection(struct sockaddr_in serv_addr)
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ 
        std::cout << "ERROR opening socket\n" << std::endl;
        return -1;
	}

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
		std::cout << "ERROR on binding\n" << std::endl;
		return -1;
    }

	if (listen(sockfd, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
	{ 
		std::cout << "ERROR on listening\n" << std::endl;
		return -1;
	}

    return 0;
}



int Server::printPortNumber(struct sockaddr_in serv_addr)
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



int Server::ConnectToClient(pthread_t *tid)
{
	int newsockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
	{
		std::cout << "ERROR on accept\n" << std::endl;
		return -1;
	}

	pthread_create(tid, NULL, clientCommunication , &newsockfd);

	return 0;
}



void* Server::clientCommunication(void *newsocket)
{
	char buffer[256];
	int n;
	int newsockfd = *(int *)newsocket;

	while(1)
	{
		bzero(buffer, 256);
		n = read(newsockfd, buffer, 256);
		
		if (n < 0) 
			std::cout << "ERROR reading from socket\n" << std::endl;
		else if(n == 0)
		{
			std::cout << "End of connection with socket " << newsockfd << std::endl;
			break;
		}
	
		std::cout << "Here is the message: " << buffer << std::endl;
		
		n = write(newsockfd,"I got your message\n", 18);
		
		if (n < 0) 
			std::cout << "ERROR writing to socket\n" << std::endl;
	}

	close(newsockfd);

	return 0;
}