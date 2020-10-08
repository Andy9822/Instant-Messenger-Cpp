#include "../../include/client/client.h"
#include "../../include/util/Packet.hpp"



Client::Client(char *ip_address, char *port)
{
	sockfd = 0;

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(atoi(port));    
	
	if(inet_pton(AF_INET, ip_address, &serv_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
	} 

	bzero(&(serv_addr.sin_zero), 8);  
}



Packet Client::buildPacket(string input)
{
	char messageBuffer[MESSAGE_MAX_SIZE] = {0};
	char groupBuffer[GROUP_MAX_SIZE] = {0};
	char usernameBuffer[USERNAME_MAX_SIZE] = {0};

	strncpy(usernameBuffer, this->username.c_str(), USERNAME_MAX_SIZE - 1);
	strncpy(groupBuffer, this->group.c_str(), GROUP_MAX_SIZE - 1);
	strncpy(messageBuffer, input.c_str(), MESSAGE_MAX_SIZE - 1); //Send message with maximum of 255 characters
	
	return Packet(usernameBuffer, groupBuffer, messageBuffer, time(0));
}



string Client::readInput()
{
    char input[MESSAGE_MAX_SIZE];
    bzero(input, MESSAGE_MAX_SIZE);

    if(fgets(input, MESSAGE_MAX_SIZE, stdin) == NULL) // ctrl+d
    {
    	Packet *sendingPacket = new Packet();
		*sendingPacket = buildPacket("< Left the group >");
		sendPacket(sockfd, sendingPacket);
		delete sendingPacket;

        exit(0);
    }

	// just a way to remove the \n
	char *pos;
	if ((pos=strchr(input, '\n')) != NULL) {
		*pos = '\0';
	}
	else {
		cout << "String is too long, cutting int to " << MESSAGE_MAX_SIZE - 1 << " chars." << endl;
	}
	fflush(stdin);
	return input;
}



int Client::registerToServer()
{
	bool connectedClient = true;
	Packet *sendingPacket = new Packet();

	*sendingPacket = buildPacket("< Entered the group >");

	// Asking server if username already exists
	sendPacket(sockfd, sendingPacket);
	Packet *receivedPacket = readPacket(sockfd, &connectedClient);

	if(receivedPacket->clientSocket == -1)
	{
		cout << "You are already logged in 2 sessions" << endl;
		sockfd = -1;
		delete sendingPacket;
		close(sockfd);
		return -1;
	}
	return 0;
}



void Client::setUsername(char* username)
{
	this->username = username;
}



void Client::setGroup(char* group) {
	this->group = group;
}



int Client::ConnectToServer(char* username, char* group)
{
	setUsername(username);
	setGroup(group);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "\n Socket creation error \n" << endl;
		return -1;
	}

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
        cout << "ERROR connecting\n" << endl;
        return -1;
	}

	return registerToServer();
}



void Client::showMessage(Packet* receivedPacket)
{
	string time, user, message;
	if (strcmp(receivedPacket->username, this->username.c_str()) == 0)
	{
		user+= "[Você]:";
	}

	else
	{
		user+= "[";
		user+= receivedPacket->username;
		user+= "]:";
	}

	time_t timestamp = receivedPacket->timestamp;
	tm *ltm = localtime(&timestamp);
	time = to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(1 + ltm->tm_sec);

	message+= time + " " + user + " " + receivedPacket->message;
	cout << message << endl;
}



void * Client::receiveFromServer(void* args)
{
	Client* _this = (Client *) args;
	bool connectedToServer = true;

	while(connectedToServer)
    {
		// Listen from TCP connection in case a Packet is received
		Packet* receivedPacket = _this->readPacket(_this->sockfd, &connectedToServer);

		if (!connectedToServer)
		{
			break;
		}

		_this->showMessage(receivedPacket);
	}
}



void * Client::sendToServer(void* args)
{
	Client* _this = (Client *) args;
	Packet *sendingPacket = new Packet();
	while (true)
	{
		// Read input
		string input = _this->readInput();

		// Prepare Packet struct to be sent
		*sendingPacket = _this->buildPacket(input);

		//Send Packet struct via TCP socket
		_this->sendPacket(_this->sockfd, sendingPacket);
	}
}



int Client::clientCommunication()
{
	int n;
	pthread_t receiverTid, senderTid;
	char input[255];
	int packetSize = sizeof(Packet);
	bool connectedToServer = true;

	pthread_create(&receiverTid, NULL, receiveFromServer, (void*) this);
	pthread_create(&senderTid, NULL, sendToServer, (void*) this);
	pthread_join(receiverTid, NULL);

	close(sockfd);

	return 0;
}
