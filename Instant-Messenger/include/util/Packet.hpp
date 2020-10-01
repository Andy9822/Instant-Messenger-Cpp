#ifndef PACKET_HPP
#define PACKET_HPP

#include <string.h> 

using namespace std;

struct Packet 
{
    int status;
    char username[20];
    char group[20];
    char message[256];
    int clientSocket;

    Packet() {}

    Packet(char* username, char* group, char* message) {
      this->status = 1440;
      strncpy(this->username, username, 20);
      strncpy(this->group, group, 20);
      strncpy(this->message, message, 256);
    }
};

#endif
