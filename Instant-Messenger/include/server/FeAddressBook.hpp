//
// Created by gabriel on 11/21/20.
//

#ifndef INSTANT_MESSENGER_FEADDRESSBOOK_HPP
#define INSTANT_MESSENGER_FEADDRESSBOOK_HPP


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "../util/Semaphore.hpp"

using namespace std;

class FeAddressBook {

private:
    std::map<string, int> serverAddressToInternalSocket;
    Semaphore* feAddressBookSemaphore;

public:
    FeAddressBook();
    int getInternalSocketId(string feAddress);
    void registryAddressSocket(string feAddress, int socketId);
    void removeServerSocket(string feAddress);
};


#endif //INSTANT_MESSENGER_FEADDRESSBOOK_HPP
