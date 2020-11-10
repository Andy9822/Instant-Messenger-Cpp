#include "../../include/server/ServersRing.hpp"



namespace servers_ring 
{

	ServersRing::ServersRing()
	{
	}



	int ServersRing::getNewPortFromFile()
	{
		string port;
		int portInt;
	  	
	  	if (myfile.is_open())
	  	{
		  	getline(myfile, port);

		  	if(myfile)
		  		portInt = stoi(port);
		  	else
		  	{
		  		// read file from the beginning
		  		myfile.close();
				myfile.open("src/server/config.txt");
				getline(myfile, port);
				portInt = stoi(port);
			}
	  	}

	  	return portInt;
	}



	void ServersRing::connectServersRing(Server serverApp)
	{	
		pthread_t accepting;
		int serverPort;
		char *ip_adress = (char*)"127.0.0.1";

		disconnected = false;

	    myfile.open("src/server/config.txt");

		serverPort = getNewPortFromFile();

		serv_addr.sin_port = htons(serverPort);
		serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(serv_addr.sin_zero), 8);

        prepareClientConnection(ip_adress);
        prepareServerConnection();

        ServersRing *argsAccepting = (ServersRing *)calloc(1, sizeof(ServersRing *));
        argsAccepting = this;

        // make server(server) connection
        pthread_create(&accepting, NULL, AcceptServerConnection, (void *) argsAccepting);  
	}



	void ServersRing::prepareClientConnection(char *ip_adress)
	{
		client_addr.sin_family = AF_INET;        
		
		if(inet_pton(AF_INET, ip_adress, &client_addr.sin_addr)<=0) 
		{ 
			std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
			exit(1);
		} 

		bzero(&(client_addr.sin_zero), 8);
	}



	void ServersRing::checkIfConnectionFailed()
	{
		ServersRing *_this = (ServersRing*)calloc(1, sizeof(ServersRing*));
		_this = this;

		// keep checking if TCP connection is ok
		bool connectedToServer = true;
		while(connectedToServer)
    	{
			// Listen from TCP connection in case a Packet is received
			Packet* receivedPacket = _this->readPacket(_this->socket_client, &connectedToServer);

			if (!connectedToServer)
			{
				break;
			}
		}

		// if server gets disconnected we try to connect it with another server port from the list
		pthread_t connecting;
		disconnected = true;
		pthread_create(&connecting, NULL, connectToServer, (void *) _this);
	}



	// client(server) connecting to server(server)
	void* ServersRing::connectToServer(void * param)
	{
		int port;
		ServersRing *_this = (ServersRing*)param;

		if ((_this->socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			cout << "\n Socket creation error \n" << endl;
			exit(1);
		}

		port = _this->getNewPortFromFile();
		_this->client_addr.sin_port = htons(port);

		// server cannot connect with itself, so we read the next port on the list
		if(port == ntohs(_this->serv_addr.sin_port))
		{
			port = _this->getNewPortFromFile();
			_this->client_addr.sin_port = htons(port);
		}

		while (connect(_this->socket_client,(struct sockaddr *) &_this->client_addr,sizeof(_this->client_addr)) < 0)
		{
			// we only check for new ports if there were a disconnection
		    if(_this->disconnected == true)
		    {
			    port = _this->getNewPortFromFile();
				_this->client_addr.sin_port = htons(port);
				_this->disconnected = false;
			}

			// server cannot connect with itself, so we read the next port on the list
			if(port == ntohs(_this->serv_addr.sin_port))
			{
				port = _this->getNewPortFromFile();
			    _this->client_addr.sin_port = htons(port);
			}
		}
		cout << "ServerRing CONNECTED WITH PORT: " << ntohs(_this->client_addr.sin_port) << endl;

		_this->checkIfConnectionFailed();	
	}



	// preparing server(server) for connection
	void ServersRing::prepareServerConnection()
	{
		// Create socket file descriptor
        if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) <= 0) 
        {
            cout << "ERROR opening socket\n" << endl;
            exit(1);
        }

        // Attach socket to server's port
        while (::bind(socket_server, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        {
        	int port = getNewPortFromFile();
        	serv_addr.sin_port = htons(port);
        }

        // Configure socket to listen for tcp connections
        if (listen(socket_server, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
        {
            cout << "ERROR on listening\n" << endl;
            exit(1);
        }

        cout << "server_socket_fd preparedConnection: " << socket_server << endl;
        cout << "ServerRing running on PORT: " << ntohs(serv_addr.sin_port) << endl;
	}



	void *ServersRing::AcceptServerConnection(void * param) 
	{
		int *newsockfd = (int*)calloc(1, sizeof(int*));
		pthread_t connecting, listenClientcomm;
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(struct sockaddr_in);

        // reference to this class
        ServersRing *_this = (ServersRing*)calloc(1, sizeof(ServersRing*));
        _this = (ServersRing *) param;

        //_this->prepareServerConnection();

        // make client(server) connection
        if(_this->disconnected == false)
        	pthread_create(&connecting, NULL, connectToServer, (void *) _this);
        else
        	_this->disconnected = false;

        if ((*newsockfd = accept(_this->socket_server, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
            cout << "ERROR on accept\n" << endl;
            exit(1);
        }

        std::cout << "CONECTOUUUUUUUU!!!!!!!" << std::endl;

        //Create another pair for the monitoring method
        std::pair<int *, ServersRing *> *args = (std::pair<int *, ServersRing *> *) calloc(1, sizeof(std::pair<int *, ServersRing *>));

        // Send pointer of the previously allocated address and be able to access it's value in new thread's execution
        args->first = newsockfd;
        // Also, send reference of this instance to the new thread
        args->second = _this;

        pthread_create(&listenClientcomm, NULL, listenClientCommunication, (void *) args);
	}



	void *ServersRing::listenClientCommunication(void *args) 
	{
		pthread_t accepting;

	    // We cast our receveid void* args to a pair*
	    std::pair<int *, ServersRing *> *args_pair = (std::pair<int *, ServersRing *> *) args;

	    // Read socket pointer's value and free the previously allocated memory in the main thread
	    int client_socketfd = *(int *) args_pair->first;
	    free(args_pair->first);

	    // Create a reference of the instance in this thread
	    ServersRing *_this = (ServersRing*)calloc(1, sizeof(ServersRing*));
	    _this = (ServersRing *) args_pair->second;

	    // Free pair created for sending arguments
	    free(args_pair);

	    bool connectedClient = true;
	    while (connectedClient) 
	    {
	        // Listen for an incoming Packet from client
	        Packet *receivedPacket = _this->readPacket(client_socketfd, &connectedClient);
	        if (!connectedClient) 
	        {
	            // Free allocated memory for reading Packet
	            free(receivedPacket);
	            break;
	        }
	    }

	    close(client_socketfd);

	    _this->disconnected = true;

        // if server gets disconnected we try to connect it with another server port from the list
        pthread_create(&accepting, NULL, AcceptServerConnection, (void *) _this);

	    return 0;
	}








}