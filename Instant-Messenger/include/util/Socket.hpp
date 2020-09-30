#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "./Packet.hpp"

class Socket
{

	public:
		Socket();
		Packet* readPacket(int client_socketfd, bool* connectedClient);
};