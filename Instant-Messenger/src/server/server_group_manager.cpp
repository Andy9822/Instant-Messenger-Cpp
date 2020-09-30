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
	std::cout << "vou registar username: " << username << std::endl;
	for (user = list_users.begin(); user != list_users.end(); user++) //TODO ffix this shit
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
	std::cout << "sai do for" << username << std::endl;

	if(userAlreadyExists == false)
	{
		std::cout << "vou newar" << std::endl;
		newUser = new User(username);
		
		std::cout << "vou registerSessionei" << std::endl;
		if(newUser->registerSession(socket) < 0)
			return -1;
		std::cout << "registerSessionei" << std::endl;
		addUserToGroup(newUser, userInfos->group);
		list_users.push_back(newUser);
	}

	return 0;
}



void ServerGroupManager::addUserToGroup(User *user, string userGroup)
{
	 group.insert(std::pair<string,User*>(userGroup,user));  
}