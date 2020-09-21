#include "../include/client.h"



int ConnectToServer(struct sockaddr_in serv_addr)
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
        printf("ERROR connecting\n");
        return -1;
	}

	return 0;
}



void* clientCommunication()
{
	char buffer[256];
	int n;

	while(1)
    {
    	printf("Enter the message: ");
    	bzero(buffer, 256);
    	fgets(buffer, 256, stdin);

		n = write(sockfd, buffer, strlen(buffer));
	    
	    if (n < 0) 
	    {
			printf("ERROR writing to socket\n");
	    }

	    bzero(buffer,256);	
	    n = read(sockfd, buffer, 256);

	    if (n < 0) 
			printf("ERROR reading from socket\n");

	    printf("%s\n",buffer);
	}

	return 0;
}





int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(atoi(argv[1]));    
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 
	bzero(&(serv_addr.sin_zero), 8);     
	
  
	ConnectToServer(serv_addr);
    
    clientCommunication();

	close(sockfd);
    

    return 0;
}