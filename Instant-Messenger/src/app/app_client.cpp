#include "../../include/client/client.h"
#include "../../include/util/definitions.hpp"
#include <iterator>
#include <vector>
#include <string>
#include <regex>
#include <signal.h>


int validateName(char *name)
{
    string stringToBeValidated(name);
    int nameLength = stringToBeValidated.length();
    regex validator("[A-Za-z][A-Za-z0-9.]+");

    bool isValidSize = ( nameLength >= 4 && nameLength <= USERNAME_MAX_SIZE);
    bool isValidString = regex_match(stringToBeValidated.begin(), stringToBeValidated.end(), validator);

    if (isValidSize && isValidString)
    {
        return 0;
    }
    else
    {
        if (!isValidSize)
        {
            std::cout << "Invalid user or group name -> Name must have between 4 and 20 characters" << std::endl;
        }
        else
        {
            std::cout << "Invalid user or group name -> Name must start with letter and be formed with numbers, letters and . only" << std::endl;
        }
        return -1;
    }
}



int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
	if(argc < 3)
    {
    	std::cout << "You forgot to include usename / group !" << std::endl;
    	return -1;
    }
    
    char *user = argv[1];
    char *group = argv[2];
    
    if(validateName(user) < 0 || validateName(group) < 0)
    	return -1;   

    Client client;
	if(client.ConnectToServer(user, group) < 0)
		return -1;
    
    if(client.clientCommunication() < 0)
    	return -1;   

    return 0;
}

