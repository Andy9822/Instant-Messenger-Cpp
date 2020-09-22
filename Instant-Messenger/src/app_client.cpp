#include "../include/client.h"



int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    Client client;

    if(argc < 3)
    {
    	std::cout << "You forgot to include IP address and PORT for the server connection!" << std::endl;
    	return -1;
    }

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(atoi(argv[2]));    
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) // "127.0.0.1" for local connection
	{ 
		std::cout << "\nInvalid address/ Address not supported \n" << std::endl;
		return -1; 
	} 
	bzero(&(serv_addr.sin_zero), 8);     
	
  
	if(client.ConnectToServer(serv_addr) < 0)
		return -1;
    
    if(client.clientCommunication() < 0)
    	return -1;   

    return 0;
}