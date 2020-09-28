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

    public:
        ServerGroupManager();
        int registerUserToServer(int socket, string username);
 		int addUserToGroup();

        //void sendMessage(Message message, User* users);
 
};
