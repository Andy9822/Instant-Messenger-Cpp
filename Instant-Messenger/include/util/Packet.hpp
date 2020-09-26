#ifndef PACKET_HPP
#define PACKET_HPP


using namespace std;
struct Packet {

    int status;
    char group[16];
    char message[126];

    Packet() {}

    Packet(char* group, char* message) {
      this->status = 1440;
      strncpy(this->group, group, 15);
      strncpy(this->message, message, 125);
    }

};

#endif
