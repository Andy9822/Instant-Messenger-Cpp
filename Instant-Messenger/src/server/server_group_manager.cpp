#include "../../include/server/server_group_manager.hpp"
#include "../../include/util/user.hpp"

using namespace std;

ServerGroupManager::ServerGroupManager() : semaphore(1)
{}




int ServerGroupManager::registerUserToGroup(int socket, string username, string group)
{
	this->semaphore.wait();
	bool userAlreadyExists = false;
	User *newUser;
	list<User*>::iterator user;
	int result = 0;
	
	for (user = list_users.begin(); user != list_users.end(); ++user)
	{
    	if((*user)->getUsername() == username)
    	{
    		if((*user)->registerSession(socket) < 0)
			{
				result = -1;
				break;
			}
    			
    		addUserToGroup(*user, group);
    		userAlreadyExists = true;

    		break;
    	}
	}

	this->semaphore.post();

	if(!userAlreadyExists)
	{
		this->semaphore.wait();
		newUser = new User(username);
		
		if(newUser->registerSession(socket) < 0)
			result = -1;
		
		addUserToGroup(newUser, group);
		list_users.push_back(newUser);
		this->semaphore.post();
	}	
	
	return result;
}

void ServerGroupManager::addUserToGroup(User *user, string userGroup)
{
	cout << "DEBUG: addUserToGroup" << endl;
	std::vector<User*> groupUsers = getUsersByGroup(userGroup);
	for (auto userItr = groupUsers.begin(); userItr != groupUsers.end(); userItr++) {
		if ((*userItr)->getUsername() == user->getUsername()){
			return;	
		}
	}
	group.insert(std::pair<string,User*>(userGroup,user));  
}

std::vector<User*> ServerGroupManager::getUsersByGroup(string groupName) {
	std::vector<User*> users;
	cout << "DEBUG: getUsersByGroup" << endl;
	for (auto itr = group.begin(); itr != group.end(); itr++) {
		if (itr -> first == groupName) {
			users.push_back(itr->second);
			cout << itr -> first << "  " << itr -> second->getUsername() << endl;	// TODO: REMOVE
		}        		
	}
	return users;
}
