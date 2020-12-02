#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "./Packet.hpp"

class Socket
{
	private:
	void sigHandler(int signal);
	void captureSignals();

	public:
		Socket();
		Packet* readPacket(int client_socketfd, bool* connectedClient);
        int sendPacket(int socket_fd, Packet* mypacket);
        int sendPacket(int socket_fd, Packet mypacket);
};

#endif