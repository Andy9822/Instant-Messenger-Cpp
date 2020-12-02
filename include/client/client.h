#include <vector>
#include <queue>
#include <fstream>
#include <ctime>
#include <arpa/inet.h>
#include "../util/Socket.hpp"
#include "../util/Packet.hpp"
#include "../util/Semaphore.hpp"
#include "../util/ConnectionKeeper.hpp"


class Client : public Socket
{
	private:
		string username;
		string group;
		int sockfd;
		vector<string> addresses;
		vector<string> ports;
		struct sockaddr_in serv_addr;
		pthread_t consumer_queue_tid;
		string readInput();
		Packet buildPacket(string input, int packetType);
		void showMessage(Packet* receivedPacket);
	public:
	

		pthread_mutex_t mutex_consumer;
		pthread_mutex_t mutex_ack;
        Semaphore* messageQueueSemaphore;
		std::queue<Packet> messages_queue;
		static void * consumeMessagesToSendQueue(void * args);

		char userId[UUID_SIZE];
		int chooseFE();
		void readFEAddressesFile();
		void setupConnection();
		void setupSocket(char *ip_address, char *port);
		Client();
        static void * receiveFromServer(void* args);
        static void * sendToServer(void* args);
		void setUsername(char* username);
		void setGroup(char* group);
		int ConnectToServer(char* username, char* group);
		int registerToServer();
		int clientCommunication();	
};