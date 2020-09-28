#include "../../include/server/server_group_manager.h"
#include "../../include/util/user.hpp"



ServerGroupManager::ServerGroupManager()
{}




int ServerGroupManager::registerUserToServer(int socket, string username)
{
	bool userAlreadyExists = false;
	User *newUser;
	list<User*>::iterator user;
	
	for (user = list_users.begin(); user != list_users.end(); ++user)
	{
    	if((*user)->getUsername() == username)
    	{
    		if((*user)->registerSession(socket) < 0)
    			return -1;

    		userAlreadyExists = true;

    		break;
    	}
	}


	if(userAlreadyExists == false)
	{
		newUser = new User(username);
		
		if(newUser->registerSession(socket) < 0)
			return -1;
		
		list_users.push_back(newUser);
	}

	return 0;
}



int ServerGroupManager::addUserToGroup()
{
	
	return 0;
}