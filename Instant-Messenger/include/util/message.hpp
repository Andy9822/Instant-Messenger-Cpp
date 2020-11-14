#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <string>
#include <ctime>

using namespace std;
using std::string;

namespace message {

class Message {

    private:
        string text;
        string user;
        string group;
        long int time;
        bool isNotification;
    
    public:
        Message();
        Message(string text, string user, string group, long int time);
        string getText();
        string getUser();
        string getGroup();
        long int getTime();
        void setText(string text);
        void setUser(string user);
        void setGroup(string group);
        void setTime(long int time);
        bool getIsNotification();
        void setIsNotification(bool value);
};
}  //namespace message


#endif