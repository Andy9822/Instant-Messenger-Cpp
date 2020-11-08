#ifndef PACKET_HPP
#define PACKET_HPP

#include <string.h> 
#include "definitions.hpp"
#include "Uuid.hpp"
#include <ctime>

using namespace std;

struct Packet 
{
    int status;
    char username[USERNAME_MAX_SIZE];
    char user_id[MESSAGE_MAX_SIZE];
    char group[GROUP_MAX_SIZE];
    char message[MESSAGE_MAX_SIZE];
    char message_id[MESSAGE_MAX_SIZE];
    int clientSocket;
    time_t timestamp;
    int type;

    Packet() {
        this->type = MESSAGE_PACKET;
    }

    Packet(int type) {
        this->type = type;
       strcpy(this->message_id, Uuid::generate_uuid_v4().c_str());
    }

    Packet(int type, char* userId) {
        this->type = type;
        strcpy(this->message_id, Uuid::generate_uuid_v4().c_str());
        strcpy(this->user_id, userId);
    }

    Packet(char* username, char* group, char* message, time_t timestamp) {
      this->status = 1440;
      strncpy(this->username, username, USERNAME_MAX_SIZE - 1);
      strncpy(this->group, group, GROUP_MAX_SIZE - 1);
      strncpy(this->message, message, MESSAGE_MAX_SIZE - 1);
      this->timestamp = timestamp;
      strcpy(this->message_id, Uuid::generate_uuid_v4().c_str());
    }

    Packet(char* username, char* group, char* message, time_t timestamp, char* userId, int packetType) {
      this->status = 1440;
      strncpy(this->username, username, USERNAME_MAX_SIZE - 1);
      strncpy(this->group, group, GROUP_MAX_SIZE - 1);
      strncpy(this->message, message, MESSAGE_MAX_SIZE - 1);
      this->timestamp = timestamp;
      strcpy(this->message_id, Uuid::generate_uuid_v4().c_str());
      strcpy(this->user_id, userId);
      this->type = packetType;
    }


    bool isKeepAlive() {
        return this->type == KEEP_ALIVE_PACKET;
    }
    bool isMessage() {
        return this->type == MESSAGE_PACKET;
    }
    bool isElection() {
        return this->type == ELECTION_PACKET;
    }
};

#endif
