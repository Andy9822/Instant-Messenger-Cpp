#include "../../include/util/Socket.hpp"

using namespace std;

Socket::Socket()
{

}

Packet* Socket::readPacket(int client_socketfd, bool* connectedClient) 
{
	int packetSize = sizeof(Packet);
	void *incomingData = (void*) malloc(packetSize);
	int receivedBytes = 0;
	int n;

	do {
		n = read(client_socketfd, (Packet*)((uintptr_t) incomingData + receivedBytes), packetSize-receivedBytes);
		if (n < 0) 
		{
			cout << "ERROR reading from socket\n" << endl;
			*connectedClient = false;
			break;
		}
		else if(n == 0)
		{
			*connectedClient = false;
			cout << "End of connection with socket " << client_socketfd << endl;
			break;
		}
		receivedBytes += n;
	} while ( receivedBytes != packetSize);

	return (Packet*) incomingData;
}

// struct sigaction sigIntHandler;

// void Socket::sigHandler(int signal){
// 	if (signal == SIGPIPE)
// 	{
// 		exit(2);
// 	}	
// }

// void Socket::captureSignals()
// {
// 	sigIntHandler.sa_handler = sigHandler;
// 	sigemptyset(&sigIntHandler.sa_mask);
// 	sigIntHandler.sa_flags = 0;
// 	sigaction(SIGPIPE, &sigIntHandler, NULL);
// }

int Socket::sendPacket(int socket_fd, Packet* mypacket)
{
    int n = write(socket_fd, mypacket, sizeof(Packet));
    if (n < 0)
    {
        cout << "ERROR writing to socket: " << socket_fd << endl ;
		shutdown(socket_fd, 2);
    }

    return n >= 0;
}