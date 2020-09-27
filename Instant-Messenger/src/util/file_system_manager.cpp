#include "../../include/util/file_system_manager.hpp"
#include <sstream>
#include <iostream>
#include <chrono>

using namespace std::chrono;

namespace filesystemmanager{


FileSystemManager::FileSystemManager() {

}

void FileSystemManager::prepareDirectory() {

}

void FileSystemManager::appendGroupMessage(Message message) {
    std::stringstream groupFile;
    std::stringstream messageContent;

    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << message.getGroup() << FILE_EXTENSION;
    messageContent << message.getUser() << FILE_SEPARATOR;
    messageContent << message.getText() << FILE_SEPARATOR;
    messageContent << message.getTime() << endl;
    
    std::ofstream groupRepository(groupFile.str(), std::ios_base::app | std::ios_base::out);
    
    groupRepository << messageContent.str();
}

} // namespace filesystemmanager;