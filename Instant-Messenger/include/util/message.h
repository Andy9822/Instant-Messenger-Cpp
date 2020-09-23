#include <string>
#include <ctime>

using namespace std;
using std::string;

namespace message {

#define BUFFER_SIZE 10

class Message {

    private:
        string text;
        string user;
        string group;
        time_t time;
    
    public:
        string getText();
        string setText(string text);
        string getUser();
        string setUser(string user);
        string getGroup();
        string setGroup(string group);
        string getTime();
        string setTime(time_t time);

};
}  //namespace message