#include "../util/message.hpp"
#include "../util/user.hpp"

#include <string>
#include <iostream>
#include <map> 
#include <list>

using namespace std;

/*using std::string;
using namespace message;*/
using namespace user;


class ServerGroupManager {

	private:
		list<User*> list_users;
		multimap<string, User*> group;
    void addUserToGroup(User *user, string group);

  public:
    ServerGroupManager();
    int registerUserToGroup(int socket, string username, string group);




      //void sendMessage(Message message, User* users);
 
};