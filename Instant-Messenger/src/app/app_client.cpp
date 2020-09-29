#include "../../include/client/client.h"
//#include "../../include/util/message.hpp"



int validateName(char *name);



int main(int argc, char *argv[])
{
	pthread_t writeThread, readThread;
	int *socket;

	if(argc < 5)
    {
    	std::cout << "You forgot to include usename / group / IP address / PORT for the server connection!" << std::endl;
    	return -1;
    }
    
    char *user = argv[1];
    char *group = argv[2];
    char *ip_address = argv[3]; // "127.0.0.1" for local connection
    char *port = argv[4];
    
    if(validateName(user) < 0 || validateName(group) < 0)
    	return -1;   

    Client client(ip_address, port);
    Message userInfo((char*)"", user, group, 0);
  
	if(*(socket = client.ConnectToServer(userInfo)) < 0)
		return -1;
    
    //if(client.clientCommunication() < 0)
    	//return -1; 
    pthread_create(&writeThread, NULL, client.writeToServer , socket);  
    pthread_create(&readThread, NULL, client.ReadFromServer , socket);

    pthread_join(writeThread, NULL);
    pthread_join(readThread, NULL);

    return 0;
}



int validateName(char *name)
{
	int nameLength = strlen(name);

	if(nameLength > 3 && nameLength < 21)
    {
    	for(int i=0; i<nameLength; i++)
    	{

    		if((name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= 'a' && name[i] <= 'z') || (name[i] == '.') || (name[i] >= '0' && name[i] <= '9'))
    		{
    			if(i == 0 && (name[i] == '.'))
    			{
    				std::cout << "Invalid user/group name -> Name must start with a letter" << std::endl;
    				return -1;
    			}
    		}
    		else
    		{
    			std::cout << "Invalid user/group name -> Name must have only letters, numbers and '.' " << std::endl;
    			return -1;
    		}
    	}
    }
    else
    {
    	std::cout << "Invalid user/group name -> Name must have between 4 and 20 characters" << std::endl;
    	return -1;
    }

    return 0;
}


