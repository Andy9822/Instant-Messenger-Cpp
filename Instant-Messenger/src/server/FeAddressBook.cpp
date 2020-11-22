#include "../../include/server/FeAddressBook.hpp"


FeAddressBook::FeAddressBook() {
    this->serverAddressToInternalSocket = map<string, int>();
    this->feAddressBookSemaphore = new Semaphore(1);
}

int FeAddressBook::getInternalSocketId(string feAddress) {
    int socket;
    feAddressBookSemaphore->wait();
    socket = this->serverAddressToInternalSocket[feAddress];
    feAddressBookSemaphore->post();
    return socket;
}

void FeAddressBook::registryAddressSocket(string feAddress, int socketId) {
    feAddressBookSemaphore->wait();
    this->serverAddressToInternalSocket[feAddress] = socketId;
    feAddressBookSemaphore->post();
}

void FeAddressBook::removeServerSocket(string feAddress) {
    feAddressBookSemaphore->wait();
    this->serverAddressToInternalSocket.erase(feAddress);
    feAddressBookSemaphore->post();
}