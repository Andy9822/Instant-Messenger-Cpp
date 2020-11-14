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
    char user_id[UUID_SIZE];
    char group[GROUP_MAX_SIZE];
    char message[MESSAGE_MAX_SIZE];
    char message_id[UUID_SIZE];
    int clientSocket;
    int frontEndSocket;
    int clientDispositiveIdentifier; // TODO aqui na real é o user_id que é um char[UUID_SIZE]
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
        this->clientDispositiveIdentifier = 17389;
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
      this->clientDispositiveIdentifier = 17389;
      this->type = packetType;
    }

    Packet(char* username, char* group, char* message, int clientDispositiveIdentifier, time_t timestamp) {	
      this->status = 1440;	
      strncpy(this->username, username, USERNAME_MAX_SIZE - 1);	
      strncpy(this->group, group, GROUP_MAX_SIZE - 1);	
      strncpy(this->message, message, MESSAGE_MAX_SIZE - 1);	
      this->clientDispositiveIdentifier = clientDispositiveIdentifier;	
      this->timestamp = timestamp;	
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
    bool isDisconnect() {
        return this->type == DISCONNECT_PACKET;
    }
    bool isAckPacket() {
        return this->type == ACK_PACKET;
    }
    bool isJoinMessage() {
        return this->type == JOIN_PACKET;
    }
};

#endif
