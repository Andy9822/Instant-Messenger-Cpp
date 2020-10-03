#ifndef PACKET_HPP
#define PACKET_HPP

#include <string.h> 
#include "definitions.hpp"
#include <ctime>

using namespace std;

struct Packet 
{
    int status;
    char username[USERNAME_MAX_SIZE];
    char group[GROUP_MAX_SIZE];
    char message[MESSAGE_MAX_SIZE];
    int clientSocket;
    time_t timestamp;

    Packet() {}

    Packet(char* username, char* group, char* message, time_t timestamp) {
      this->status = 1440;
      strncpy(this->username, username, USERNAME_MAX_SIZE - 1);
      strncpy(this->group, group, GROUP_MAX_SIZE - 1);
      strncpy(this->message, message, MESSAGE_MAX_SIZE - 1);
      this->timestamp = timestamp;
    }
};

#endif
