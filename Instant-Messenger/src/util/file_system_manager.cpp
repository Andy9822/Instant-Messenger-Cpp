#include "../../include/util/file_system_manager.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>

using namespace std::chrono;

namespace filesystemmanager{

FileSystemManager::FileSystemManager() {

}

void FileSystemManager::prepareDirectory() {

}

void FileSystemManager::appendGroupMessageToHistory(Message message) {
    std::stringstream groupFile;
    std::stringstream messageContent;

    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << message.getGroup() << FILE_EXTENSION;
    messageContent << message.getUser() << FILE_SEPARATOR;
    messageContent << message.getText() << FILE_SEPARATOR;
    messageContent << message.getTime() << endl;
    
    std::ofstream groupRepository(groupFile.str(), std::ios_base::app | std::ios_base::out);
    
    groupRepository << messageContent.str();
    groupRepository.close();
}

std::vector<std::vector<std::string>> FileSystemManager::readGroupHistoryMessages(string groupName) {
    
    std::string line;
    std::vector<std::vector<std::string> > parsedCsv;
    std::stringstream groupFile;
    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << groupName << FILE_EXTENSION;
    std::ifstream data;
    
    data.open(groupFile.str());

    while(std::getline(data,line))
    {
        cout << line << endl;
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> parsedRow;
        while(std::getline(lineStream,cell,','))
        {
            parsedRow.push_back(cell);
        }

        parsedCsv.push_back(parsedRow);
    }

    data.close();

    return parsedCsv;
}

} // namespace filesystemmanager;