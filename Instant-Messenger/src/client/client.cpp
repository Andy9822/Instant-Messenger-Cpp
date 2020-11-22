#include "../../include/client/client.h"


Client::Client()
{
	strcpy(this->userId, Uuid::generate_uuid_v4().c_str());
	sockfd = 0;

    readFEAddressesFile();
	setupConnection();
	
	 // Init semaphore
    messageQueueSemaphore = new Semaphore(1);

	// Init consumer mutex
    pthread_mutex_init(&mutex_consumer, NULL);
    pthread_mutex_lock(&mutex_consumer); // Consumer mutex inits locked, due to absence of messages in the queue to be consumed

	//  Init ack mutex
    pthread_mutex_init(&mutex_ack, NULL);
    pthread_mutex_lock(&mutex_ack); // ack mutex inits locked, due to absence of acks 

    // Create thread 24/7 alive for consume queue when available
    pthread_create(&consumer_queue_tid, NULL, consumeMessagesToSendQueue, (void *) this);
}

void Client::setupConnection()
{
	int chosenFE = chooseFE();

	int sizeAddressStr = addresses[chosenFE].size();
	int sizePortStr = ports[chosenFE].size();

	char address[sizeAddressStr];
	char port[sizePortStr];

	strncpy(address, addresses[chosenFE].c_str(), sizeAddressStr-1);
	strncpy(port, ports[chosenFE].c_str(), sizePortStr-1);

	address[sizeAddressStr - 1] = '\0';
	port[sizePortStr - 1] = '\0';

	std::cout << "chosen: " << chosenFE << std::endl;
	std::cout << "port: " << port << std::endl;
	setupSocket(address, port);
}

void Client::setupSocket(char *ip_address, char *port)
{
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(atoi(port));    
	
	if(inet_pton(AF_INET, ip_address, &serv_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
	} 
	bzero(&(serv_addr.sin_zero), 8);  
}

void * Client::consumeMessagesToSendQueue(void * args)
{
    Client* _this = (Client *) args;
    while (true)
    {
        pthread_mutex_lock(&(_this->mutex_consumer));
		
		bool hasMessagesInQueue = true;
        while (hasMessagesInQueue)
        {
			_this->messageQueueSemaphore->wait();

			if (_this->messages_queue.empty())
            {
                hasMessagesInQueue = false;
                _this->messageQueueSemaphore->post();
            }

			else
			{
				Packet message = _this->messages_queue.front();
				_this->messages_queue.pop();
				_this->messageQueueSemaphore->post();

				//Send consumed packet struct via TCP socket
				_this->sendPacket(_this->sockfd, message);  

				// Locks ack mutex to ensure the next message will be consumed only after receiving an ack for the actual sent packed
				pthread_mutex_lock(&(_this->mutex_ack));
			}
		}
    }
}

int Client::chooseFE()
{
	srand((unsigned) time(0));
    int randomNumber;
	randomNumber = rand() % addresses.size();
	return randomNumber;
}

void Client::readFEAddressesFile()
{
    string line;
    ifstream myfile ("src/client/addresses.txt");
    int i = 0;
    if (myfile.is_open())
    {
        while ( getline (myfile,line))
        {
            if (i == 0)
            {
                addresses.push_back(line);
            }
            else
            {
                ports.push_back(line);
            }
            i+=1;
            i%=2;
        }
        myfile.close();
    }
}

Packet Client::buildPacket(string input, int packetType)
{
	char messageBuffer[MESSAGE_MAX_SIZE] = {0};
	char groupBuffer[GROUP_MAX_SIZE] = {0};
	char usernameBuffer[USERNAME_MAX_SIZE] = {0};

	strncpy(usernameBuffer, this->username.c_str(), USERNAME_MAX_SIZE - 1);
	strncpy(groupBuffer, this->group.c_str(), GROUP_MAX_SIZE - 1);
	strncpy(messageBuffer, input.c_str(), MESSAGE_MAX_SIZE - 2); //Send message with maximum of 255 characters
	
	// Adjust last character to end of string in case string was bigger than max size
	messageBuffer[MESSAGE_MAX_SIZE - 1] = '\0';

	return Packet(usernameBuffer, groupBuffer, messageBuffer, time(0), this->userId, packetType);
}



string Client::readInput()
{
    string input;
    std::getline(std::cin, input);

	// Ctrl + D
	if(std::cin.eof()) {
		close(sockfd);
        exit(0);
	}

	return input;
}



int Client::registerToServer()
{
	bool connectedClient = true;
	Packet *sendingPacket = new Packet();

	sendingPacket->clientDispositiveIdentifier = 123123; // TODO: remove, this is just a debug

	*sendingPacket = buildPacket("<Entered the group>", JOIN_PACKET);
	sendPacket(sockfd, sendingPacket);

	// Asking server if user can join with one more session
	Packet *receivedPacket;
	bool waitingAccept = true;
	while (waitingAccept)
	{
		receivedPacket = readPacket(sockfd, &connectedClient);
		if(receivedPacket->type == CONNECTION_REFUSED_PACKET)
		{
			cout << "You are already logged in 2 sessions" << endl;
			sockfd = -1;
			delete sendingPacket;
			close(sockfd);
			return -1;
		}

		if (receivedPacket->type == ACCEPT_PACKET)
		{
			waitingAccept = false;
		}
	}

    std::cout << "\n" << "Bem-vindo ao grupo: " << group << std::endl;

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

	
	ConnectionKeeper(this->sockfd); // starts the thread that keeps sending keep alives


	//TODO refazer isso pra esperar register no main loop e com uma variável de offline ou algo assim
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

		if (receivedPacket->type == MESSAGE_PACKET)
		{
			_this->showMessage(receivedPacket);
		}

		if (receivedPacket->type == ACK_PACKET)
		{
			// Unlocks ack mutex so the consumer can consume the next message
			pthread_mutex_unlock(&(_this->mutex_ack));
		}
	}
	return NULL;
}



void * Client::sendToServer(void* args)
{
	Client* _this = (Client *) args;
	Packet sendingPacket;
	while (true)
	{
		// Read input
		string input = _this->readInput();

		// Prepare Packet struct to be sent
		sendingPacket = _this->buildPacket(input, MESSAGE_PACKET);

		// Append sending packet to queue 
		_this->messageQueueSemaphore->wait();
		_this->messages_queue.push(sendingPacket);
		_this->messageQueueSemaphore->post();

		// Unlocks consumer mutex so it knows there's something in the queue
		pthread_mutex_unlock(&(_this->mutex_consumer));
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

	std::cout << "A conexão com o servidor foi perdida" << std::endl;
	close(sockfd);

	return 0;
}
