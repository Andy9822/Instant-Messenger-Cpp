#ifndef MESSAGE_HPP
#define MESSAGE_HPP

//#include <string>
#include <ctime>
#include <string.h>

using namespace std;
//using std::string;


struct Message
{
    char text[256];
    char user[20];
    char group[20];
    long int time;

    Message() {}

    Message(char* text, char* user, char* group, long int time) 
    {
        strncpy(this->text, text, 256);
        strncpy(this->user, user, 20);
        strncpy(this->group, group, 20);
        this->time = time;
    }   
};















/*namespace message {

class Message {

    private:
        string text;
        string user;
        string group;
        long int time;
    
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
};
}  //namespace message
*/
#endif