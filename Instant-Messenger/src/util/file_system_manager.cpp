#include "../../include/util/file_system_manager.hpp"
#include "../../include/util/definitions.hpp"
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

using namespace std::chrono;

namespace filesystemmanager{

FileSystemManager::FileSystemManager() : semaphore(1) {

}

void FileSystemManager::appendGroupMessageToHistory(Message message) {
    if (message.getIsNotification() ) return; // if entry or disconnected message, do not save to the history
    semaphore.wait();
    std::stringstream groupFile;
    std::stringstream messageContent;
    string messageTextTreated = message.getText();

    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << message.getGroup() << FILE_EXTENSION;
    messageContent << message.getUser() << FILE_SEPARATOR;
    messageContent << messageTextTreated << FILE_SEPARATOR;
    messageContent << message.getTime() << endl;

    cropToFileHistoryLimit(message.getGroup(), true);
    std::ofstream groupRepository(groupFile.str().c_str(), std::ios_base::app | std::ios_base::out);

    groupRepository << messageContent.str();
    groupRepository.close();

    semaphore.post();
}

/**
 * This method garantees that we are only reading N messages in the history
 * @param groupName
 * @return
 */
std::vector<Message> FileSystemManager::readGroupHistoryMessages(string groupName) {
    semaphore.wait();
    std::string line;
    std::stringstream groupFile;
    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << groupName << FILE_EXTENSION;
    std::ifstream data;
    std::vector <Message> messages;

    cropToFileHistoryLimit(groupName, false);

    data.open(groupFile.str().c_str());

    while(getline(data,line))
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
/**
 * This method is responsible for preparing the file before it sends the history to the user
 * We need to guarantee that we have up to N messages per group file where N is the limit configured
 */
    void FileSystemManager::cropToFileHistoryLimit(string group, bool isWrite) {
    std::stringstream groupFile, tempGroupFile;
    groupFile << MESSAGES_BASE_PATH << PATH_SEPARATOR << group << FILE_EXTENSION;
    tempGroupFile << groupFile.str() << "_temp";
    ifstream oldFile(groupFile.str().c_str());
    std::string line;
    vector<std::string> lines;
    int isWriteDiscount = 0;
    while (std::getline(oldFile, line)) {
        lines.push_back(line);
    }

    if (isWrite) {
        isWriteDiscount = 1;
    }

    if (lines.size() > getMaxNumberOfMessagesInHistory()) {
        std::ofstream newFile(tempGroupFile.str(), std::ios_base::app | std::ios_base::out);
        for (int i = (lines.size() - (getMaxNumberOfMessagesInHistory() - isWriteDiscount)); i < lines.size(); i++) {
            if (i > 0) {
                newFile << lines[i] << endl;
            }
        }

        remove(groupFile.str().c_str());
        rename(tempGroupFile.str().c_str(), groupFile.str().c_str());
        newFile.close();
    }

    oldFile.close();
}

int FileSystemManager::getMaxNumberOfMessagesInHistory() {
    if ( this->maxNumberOfMessagesInHistory <= 0 ) {
        return DEFAULT_NUMBER_OF_RECORDED_MESSAGES;
    }
    return this->maxNumberOfMessagesInHistory;
}

void FileSystemManager::setMaxNumberOfMessagesInHistory(int value) {
    this->maxNumberOfMessagesInHistory = value;
}

} // namespace filesystemmanager;