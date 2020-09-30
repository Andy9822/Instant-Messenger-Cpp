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
		n = read(client_socketfd, (incomingData + receivedBytes), packetSize-receivedBytes);
		if (n < 0) 
		{
			cout << "ERROR reading from socket\n" << endl;
			*connectedClient = false;
			break;
		}
		else if(n == 0) // Client closed connection
		{
			*connectedClient = false;
			cout << "End of connection with socket " << client_socketfd << endl;
			break;
		}
		receivedBytes += n;
	} while ( receivedBytes != packetSize);

	return (Packet*) incomingData;
}