#include "../../include/server/server_group_manager.h"
#include "../../include/util/user.hpp"



ServerGroupManager::ServerGroupManager()
{}




int ServerGroupManager::registerUserToServer(int socket, Message *userInfos)
{
	string username = userInfos->user;
	bool userAlreadyExists = false;
	User *newUser;
	list<User*>::iterator user;
	
	for (user = list_users.begin(); user != list_users.end(); ++user)
	{
    	if((*user)->getUsername() == username)
    	{
    		if((*user)->registerSession(socket) < 0)
    			return -1;

    		addUserToGroup(*user, userInfos->group);
    		userAlreadyExists = true;

    		break;
    	}
	}


	if(userAlreadyExists == false)
	{
		newUser = new User(username);
		
		if(newUser->registerSession(socket) < 0)
			return -1;
		
		addUserToGroup(newUser, userInfos->group);
		list_users.push_back(newUser);
	}

	return 0;
}



void ServerGroupManager::addUserToGroup(User *user, string userGroup)
{
	 group.insert(std::pair<string,User*>(userGroup,user));  
}