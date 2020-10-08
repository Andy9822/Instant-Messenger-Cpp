#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <ios>
#include <fstream>
#include <vector>
#include "message.hpp"
#include "Semaphore.hpp"


#define MESSAGES_BASE_PATH "messages"
#define FILE_EXTENSION ".csv"
#define FILE_SEPARATOR (char) 31
#define PATH_SEPARATOR "/"

using namespace std;
using std::string;
using namespace message;

namespace filesystemmanager {

class FileSystemManager {

    private: 
        Semaphore semaphore;

    public: 
        FileSystemManager();
        void prepareDirectory();
        void appendGroupMessageToHistory(Message message);
        std::vector< Message > readGroupHistoryMessages(string groupName);
};
} // namespace filesystemmanager;