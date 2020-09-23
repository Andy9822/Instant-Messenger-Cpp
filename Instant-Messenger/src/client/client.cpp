#include "../../include/client/client.h"



Client::Client()
{
	sockfd = 0;
}



int Client::ConnectToServer(struct sockaddr_in serv_addr)
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		std::cout << "\n Socket creation error \n" << std::endl; 
		return -1; 
	} 

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
        std::cout << "ERROR connecting\n" << std::endl;;
        return -1;
	}

	return 0;
}



int Client::clientCommunication()
{
	char buffer[256];
	int n;

	while(1)
    {
    	std::cout << "Enter the message: ";
    	
    	bzero(buffer, 256);
    	
    	if(fgets(buffer, 256, stdin) == NULL) // ctrl+d
    	{
    		break;
    	}

		n = write(sockfd, buffer, strlen(buffer));
	    
	    if (n < 0) 
	    {
			std::cout << "ERROR writing to socket\n" << std::endl;
			return -1;
	    }

	    bzero(buffer,256);	
	    n = read(sockfd, buffer, 256);

	    if (n < 0) 
	    {
			std::cout << "ERROR reading from socket\n" << std::endl;
			return -1;
	    }

	    std::cout << buffer << std::endl;;
	}

	close(sockfd);

	return 0;
}