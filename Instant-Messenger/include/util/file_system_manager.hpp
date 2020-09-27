#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <ios>
#include <fstream>
#include <vector>



#define MESSAGES_BASE_PATH "../../messages"
#define FILE_EXTENSION ".csv"
#define FILE_SEPARATOR ","
#define PATH_SEPARATOR "/"

using namespace std;

using std::string;

namespace filesystemmanager {

using namespace message;

class FileSystemManager {
    public: 
        FileSystemManager();
        void prepareDirectory();
        void appendGroupMessage(Message message);
        std::vector<std::vector<std::string>> readGroupMessages(string groupName);        
};
} // namespace filesystemmanager;