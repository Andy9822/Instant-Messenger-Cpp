#include "../../include/client/client.h"



int validateName(char *name);



int main(int argc, char *argv[])
{
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
  
	if(client.ConnectToServer() < 0)
		return -1;
    
    if(client.clientCommunication() < 0)
    	return -1;   

    return 0;
}



int validateName(char *name)
{
	int nameLength = strlen(name);

	if(nameLength > 3 && nameLength < 21)
    {
    	for(int i=0; i<nameLength; i++)
    	{

    		if((name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= 'a' && name[i] <= 'z') || (name[i] == '.'))
    		{
    			if(i == 0 && (name[i] == '.'))
    			{
    				std::cout << "Invalid user/group name -> Name must start with a letter" << std::endl;
    				return -1;
    			}
    		}
    		else
    		{
    			std::cout << "Invalid user/group name -> Name must have only letters and '.' " << std::endl;
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


