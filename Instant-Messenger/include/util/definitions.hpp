#ifndef DEFAULT_NUMBER_OF_RECORDED_MESSAGES
#define DEFAULT_NUMBER_OF_RECORDED_MESSAGES 10
#endif

#ifndef RANDOM_PORT_NUMBER
#define RANDOM_PORT_NUMBER 0
#endif

#ifndef USERNAME_MAX_SIZE
#define USERNAME_MAX_SIZE 20
#endif

#ifndef UUID_SIZE
#define UUID_SIZE 37
#endif

#ifndef GROUP_MAX_SIZE
#define GROUP_MAX_SIZE 20
#endif

#ifndef MESSAGE_MAX_SIZE
#define MESSAGE_MAX_SIZE 255
#endif

#ifndef JOIN_QUIT_STATUS_MESSAGE
#define JOIN_QUIT_STATUS_MESSAGE 3
#endif

#ifndef DEBUG_MODE
#define DEBUG_MODE false
#endif


#ifndef DEBUG_MODE
#define DEBUG_MODE false
#endif


#ifndef JOINED_MESSAGE
#define JOINED_MESSAGE "<Joined the group>"
#endif

#ifndef LEFT_GROUP_MESSAGE
#define LEFT_GROUP_MESSAGE "<Left the group>"
#endif

#ifndef KEEP_ALIVE_TIMEOUT
#define KEEP_ALIVE_TIMEOUT 10
#endif

#ifndef KEEP_ALIVE_INTERVAL
#define KEEP_ALIVE_INTERVAL 2
#endif

#ifndef PACKET_TYPES
#define PACKET_TYPES
/////// Types of Packet
enum
{
    MESSAGE_PACKET = 0,
    ACK_PACKET,
    JOIN_PACKET,
    ACCEPT_PACKET,
    KEEP_ALIVE_PACKET,
    CONNECTION_REFUSED_PACKET,
    DISCONNECT_PACKET,
    ELECTION_PACKET,
};
////////
#endif