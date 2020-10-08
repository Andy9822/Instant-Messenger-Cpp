#include "../../include/util/file_system_manager.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>

using namespace std::chrono;

namespace filesystemmanager{

FileSystemManager::FileSystemManager() : semaphore(1) {

}

void FileSystemManager::prepareDirectory() {

}

void FileSystemManager::appendGroupMessageToHistory(Message message) {
    semaphore.wait();
    std::stringstream groupFile;
    std::stringstream messageContent;
    string messageTextTreated = message.getText();

    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << message.getGroup() << FILE_EXTENSION;
    messageContent << message.getUser() << FILE_SEPARATOR;
    messageContent << messageTextTreated << FILE_SEPARATOR;
    messageContent << message.getTime() << endl;
    
    std::ofstream groupRepository(groupFile.str(), std::ios_base::app | std::ios_base::out);

    groupRepository << messageContent.str();
    groupRepository.close();

    semaphore.post();
}

std::vector<Message> FileSystemManager::readGroupHistoryMessages(string groupName) {
    semaphore.wait();
    std::string line;
    std::stringstream groupFile;
    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << groupName << FILE_EXTENSION;
    std::ifstream data;
    std::vector <Message> messages;

    data.open(groupFile.str());

    while(std::getline(data,line))
    {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> parsedRow;
        while(std::getline(lineStream,cell, FILE_SEPARATOR))
        {
            parsedRow.push_back(cell);
        }

        stringstream intTime(parsedRow[2]);

        long int time = 0;
        intTime >> time;
        Message message = Message((string) parsedRow[1], (string) parsedRow[0], groupName, time);

        messages.push_back(message);

    }

    data.close();
    semaphore.post();

    return messages;
}

} // namespace filesystemmanager;