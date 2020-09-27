#ifndef PACKET_HPP
#define PACKET_HPP


using namespace std;

struct Packet 
{
    int status;
    char group[20];
    char message[256];
    int clientSocket;

    Packet() {}

    Packet(char* group, char* message) {
      this->status = 1440;
      strncpy(this->group, group, 20);
      strncpy(this->message, message, 256);
    }
};

#endif
