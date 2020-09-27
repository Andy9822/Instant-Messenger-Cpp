#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <ios>
#include <fstream>


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
        
};
}  //filesystemmanager directory;