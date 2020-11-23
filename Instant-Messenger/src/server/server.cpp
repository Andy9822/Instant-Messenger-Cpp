#include "../../include/server/server.hpp"
#include "../../include/util/definitions.hpp"
#include "../../include/util/ConnectionKeeper.hpp"
#include "../../include/util/StringConstants.hpp"


#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>
#include "../../include/util/exceptions.hpp"

#endif

namespace server {
    vector<int> Server::openSockets;
    bool Server::isPrimaryServer = 0;


    Server::Server() {
        // Init semaphore for openSockets
        sockets_connections_semaphore = new Semaphore(1);
        feConnectionInitializationSemaphore = new Semaphore(1);
        feAddressesSemaphore = new Semaphore(1);

        connectionMonitor = new ConnectionMonitor();

        socketFeList = vector<int>(); // TODO: remove
        feAddresses = vector<string>();
        this->feAddressBook = new FeAddressBook();
        groupManager = new ServerGroupManager(this->feAddressBook);

        rm_listening_serv_addr.sin_family = AF_INET;
        rm_listening_serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(rm_listening_serv_addr.sin_zero), 8);

        rm_listening_socket_fd = 0;
    }

    void Server::prepareReplicationManager(int rmNumber) {
        this->setRmNumber(rmNumber);
        this->connectToRmServers();

        if(this->getRmNumber() > 0) {
            this->createRMListenerSocket();
        }
    }

    void Server::closeFrontEndConnections() {
        cout << "Number of front end connections: " << feAddresses.size() << endl;
        this->feAddressesSemaphore->wait();
        for (auto fe_address : this->feAddresses) {
            int socketId = getSocketFromAddress(fe_address);
            close(socketId);
            cout << "[DEBUG] closed " << fe_address;
        }
        this->feAddressesSemaphore->post();
        cout << "Number of front end connections: " << feAddresses.size() << endl;
    }

    void Server::closeServer() {
        cout << "CLOSING All connections from the server " << endl;
        closeFrontEndConnections();
    }

    // TODO adaptar pra quando tiver 2 sockets, ou pelo menos deixa o TODO aqui
    void Server::setPort(int port) {
        /*serv_addr.sin_port = htons(atoi(port)); */
    }

    /**
     * Only used to forward the message to the proper group. The group turns himself to deliver
     * @param socket
     * @param username
     * @param group
     * @return
     */
    int Server::registerUser(pair<string, string> clientIdentifier, char *username, char *group) {
        return groupManager->registerUserToGroup(clientIdentifier, username, group);
    }

    int Server::registerUserToServer(Packet *registrationPacket, string feAddress) {
        // Store new crated socket in the vector of existing connections sockets and increment counter for the user

        this->sockets_connections_semaphore->wait();
        if (incrementNumberOfConnectionsFromUser(registrationPacket->username) == USER_SESSIONS_LIMIT_REACHED ) { // we can keep this
            Packet *connectionRefusedPacket = new Packet(CONNECTION_REFUSED_PACKET);
            strcpy(connectionRefusedPacket->user_id, registrationPacket->user_id);
            std::cout << "[DEBUG|ERROR] limit sessoes alcanadas pelo user" << std::endl;
            if(isPrimaryServer) {
                int socketId = getSocketFromAddress(feAddress);
                this->sendPacket(socketId, connectionRefusedPacket);
            }
            this->sockets_connections_semaphore->post();
            return -1;
        }
        this->sockets_connections_semaphore->post();

        std::pair<string, string> clientFrontEndIdentifier = std::pair<string, string>(); // this is the identifier for the client and we need to store this in the groups
        clientFrontEndIdentifier.first.assign(registrationPacket->user_id);
        clientFrontEndIdentifier.second.assign(feAddress);

        return this->registerUser(clientFrontEndIdentifier, registrationPacket->username,
                                            registrationPacket->group);

    }


    int Server::connectToFE(string feAddress, int fePort)
    {
        int newSocketFE;

        {
            serv_addr.sin_family = AF_INET;    
            serv_addr.sin_port = htons(fePort);
            if(inet_pton(AF_INET, feAddress.c_str(), &serv_addr.sin_addr)<=0)
            { 
                std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
            } 

            bzero(&(serv_addr.sin_zero), 8); 
        }

        if ((newSocketFE = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            cout << "\n Socket creation error \n" << endl;
            return -1;
        }

        if (connect(newSocketFE,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
            cout << "ERROR connecting\n" << endl;
            return -1;
        }

        socketFeList.push_back(newSocketFE);

        string feIpPort = feAddress + ":" + to_string(fePort);

        std::cout << "conectado ao FE (" << feIpPort << ") com socket:" << newSocketFE << std::endl;

        this->feAddressBook->registryAddressSocket(feIpPort, newSocketFE);
        this->feAddresses.push_back(feIpPort);

        return 0;
    }

    int Server::handleFrontEndsConnections() {
        std::pair<string, Server *> *args = (std::pair<string, Server *> *) calloc(1, sizeof(std::pair<string, Server *>));

        vector<ConnectionKeeper*> connectionKeepers = vector<ConnectionKeeper*>();
        args->second = this;

        int i = 0;
        feAddressesSemaphore->wait();
        for ( string feAddress : this->feAddresses ) {
            int socketId = getSocketFromAddress(feAddress);
            this->feConnectionInitializationSemaphore->wait(); // the POST is done inside the new threads created only when the args is no longer necessary
            args->first.assign(feAddress);
            pthread_create(&(this->tid[i++]), NULL, listenFrontEndCommunication, (void *) args);
            std::cout << "handleFrontEndsConnections ao FE com socket:" << feAddress << ", socket:" << socketId << std::endl;
            //this->feConnectionInitializationSemaphore->wait();
            this->feConnectionInitializationSemaphore->post();
            pthread_create(&(this->tid[i++]), NULL, monitorConnection, (void *) args);
            connectionKeepers.push_back(new ConnectionKeeper(socketId)); // starts the thread that keeps sending keep alives
        }
        feAddressesSemaphore->post();

        this->feConnectionInitializationSemaphore->wait();
        free(args);
        this->feConnectionInitializationSemaphore->post();

        return 0;
    }

    void * Server::monitorConnection(void *args) {
        std::pair<string, Server *> *args_pair = (std::pair<string, Server *> *) args;
        string feAddress((string) args_pair->first);
        Server *_this = (Server *) args_pair->second;

        int socketID = _this->getSocketFromAddress(feAddress);

        _this->feConnectionInitializationSemaphore->post();
        _this->connectionMonitor->monitor(&socketID);
        _this->closeFrontEndConnection(feAddress);
        return NULL;
    }

    void *Server::listenFrontEndCommunication(void *args) {

        // TODO: receive a pair from the args. The first will be the socket and the second one will be the server
        std::pair<string, Server *> *args_pair = (std::pair<string, Server *> *) args;
        string feAddress(args_pair->first);
        Server *_this = (Server *) args_pair->second;

        int feSocket = _this->getSocketFromAddress(feAddress);

        _this->feConnectionInitializationSemaphore->post();

        bool connectedFrontEnd = true;
        bool replicationError = false;
        bool connectedRm = true;

        while (connectedFrontEnd) {
            // Listen for an incoming Packet from client
            Packet *receivedPacket = _this->readPacket(feSocket, &connectedFrontEnd);
            if (!connectedFrontEnd) {
                // Free allocated memory for reading Packet
                free(receivedPacket);
                break;
            }

            // verify if should send replication socket or not
            _this->sockets_connections_semaphore->wait();
            replicationError = false;
            if(isPrimaryServer && (receivedPacket->isMessage() || receivedPacket->isJoinMessage() || receivedPacket->isDisconnect())) {
                cout << "Sending " << _this->rm_connect_sockets_fd.size() << " replication packets" << endl;

                for(auto socket : _this->rm_connect_sockets_fd) {
                    cout << "Sending packet of type " << receivedPacket->type << " to socket " << socket.first << endl;
                    strcpy(receivedPacket->frontEndAddress, feAddress.c_str());
                    _this->sendPacket(socket.first, receivedPacket); // send message for each socket on RM socket list
                    Packet *confirmationPacket = _this->readPacket(socket.first, &connectedRm); // wait for socket answer
                    cout << "Reading packet of type " << confirmationPacket->type << " on socket " << socket.first << endl;

                    if (!connectedRm) {
                        // Free allocated memory for reading Packet
                        //free(receivedPacket);
                        auto iterator = _this->rm_connect_sockets_fd.find(socket.first);
                        _this->rm_connect_sockets_fd.erase(iterator);
                        connectedRm = true;
                        replicationError = true;
                    }
                }

                cout << "All packets were replicate successfully" << endl;
            }
            _this->sockets_connections_semaphore->post();

            if(replicationError) {
                cout << "Error on replicating message. What should we do?" << endl;
            }

            if (receivedPacket->isMessage()) {
                cout << "[DEBUG] recebi message " << receivedPacket->message << endl;
                Packet *pack = new Packet();
                pack->type = ACK_PACKET;
                strcpy(pack->user_id, receivedPacket->user_id);
                _this->sendPacket(feSocket, pack);
                std::cout << "[DEBUG] mandei ACK para socket: " << feSocket << std::endl;
                _this->groupManager->processReceivedPacket(receivedPacket);
            } else if (receivedPacket->isKeepAlive()){
                _this->connectionMonitor->refresh(feSocket);
            } else if (receivedPacket->isJoinMessage()) {
                std::cout << "[DEBUG] recebi joinMessage" << std::endl;
                _this->registerUserToServer(receivedPacket, feAddress); // considers the front end connection
            } else if (receivedPacket->isDisconnect()) {
                pair<string, string> connectionId = pair<string, string>();
                connectionId.first.assign(receivedPacket->user_id);
                connectionId.second.assign(feAddress);
                std::cout << "[DEBUG] vou chamar a rotina para o disconnect do fe " <<  connectionId.first << "endereco" << feAddress << std::endl;
                _this->closeClientConnection(connectionId); // considers the front end connection
            }
        }

        _this->connectionMonitor->killSocket(feSocket);
        // Close all properties related to client connection
        _this->closeFrontEndConnection(feAddress);


        cout << "Stopped listening on FE socket " << feSocket << "address: " << feAddress << endl;

        //exit(0);

        return 0;
    }

    /**
     * When the FE dies, let's process it and kill all the connections that we had for the users
     * @param socketId
     */
    void Server::closeFrontEndConnection(string feAddress) {
        bool connectionFound = false;
        feAddressesSemaphore->wait();
        for (string currentAddress : this->feAddresses) {
            if (feAddress.compare(currentAddress) == 0) {
                connectionFound = true;
                break;
            }
        }
        feAddressesSemaphore->post();

        if (!connectionFound) return; // cai fora pq a gente já fechou esse socket

        cout << "[DEBUG] closeFrontEndConnection closing FE " << feAddress << endl;
        pair <string, string> connectionId = pair<string, string>();
        connectionId.first.assign(FE_DISCONNECT);
        connectionId.second.assign(feAddress);

        closeClientConnection(connectionId);

        int socketId = getSocketFromAddress(feAddress);

        if ( (close(socketId) ) == 0) {
            std::cout << "\nClosed socket: " << socketId << ", connection with FE " << feAddress << std::endl;
        } else {
            std::cout << "!!! Fatal error closing socket!!!!" << std::endl;
        }
        purgeFeConnection(feAddress);
    }

    void Server::purgeFeConnection(string feAddress) {
        feAddressesSemaphore->wait();
        int deletion_index = 0;
        auto begin = feAddresses.begin();
        for (auto address : feAddresses) {
            if (address.compare(feAddress) == 0 ) {
                feAddresses.erase(begin + deletion_index); // will delete the deletion_index's item
                feAddressBook->removeServerSocket(feAddress);
            }
            deletion_index += 1;
        }
        feAddressesSemaphore->post();
    }

    void Server::closeClientConnection(pair<string, string> clientConnectionId) {
        sockets_connections_semaphore->wait();
        groupManager->propagateSocketDisconnectionEvent(clientConnectionId, this->connectionsCount);
        sockets_connections_semaphore->post();
    }


    void Server::configureFilesystemManager(int maxNumberOfMessagesInHistory) {
        groupManager->configureFileSystemManager(maxNumberOfMessagesInHistory); // THIS CALL IS OK, WE NEED TO PASS THE INFORMATION
    }

    int Server::incrementNumberOfConnectionsFromUser(string user) {
        map<string, int>::iterator it = this->connectionsCount.find(user);

        if(it != this->connectionsCount.end()) // achou
        {
            if (connectionsCount[user] >= MAX_NUMBER_OF_SIMULTANEOUS_CONNECTIONS ) {
                return USER_SESSIONS_LIMIT_REACHED;
            }
            connectionsCount[user] += 1;
            return 0;
        } else {
            connectionsCount[user] = 1;
            return 0;
        }
    }

    int Server::getSocketFromAddress(const string feAddress) {
        int socketId = feAddressBook->getInternalSocketId(feAddress);

        if (socketId == 0) {
            cout << "[ERROR] the address provided for the FE does not have any socket" << endl;
        }
        return socketId;
    }


    void Server::connectToRmServers() {
        for(int connectMachineNumber = rmNumber; connectMachineNumber < MAX_RM; connectMachineNumber++) {
            int *connectSocket = (int *) calloc(1, sizeof(int *));
            char ip_address[10] = "127.0.0.1";
            struct sockaddr_in rm_connect_socket_addr{};

            rm_connect_socket_addr.sin_family = AF_INET;
            rm_connect_socket_addr.sin_port = htons(RM_BASE_PORT_NUMBER + connectMachineNumber + 1); // port is base port + next machine RM number (rmNumber+1)
            bzero(&(rm_connect_socket_addr.sin_zero), 8);

            if (inet_pton(AF_INET, ip_address, &rm_connect_socket_addr.sin_addr) <= 0) {
                cout << "\n RM Invalid address/ Address not supported \n" << endl;
                continue;
            }

            if ((*connectSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                cout << "\n RM Socket creation error \n" << endl;
                continue;
            }

            if (connect(*connectSocket, (struct sockaddr *) &rm_connect_socket_addr, sizeof(rm_connect_socket_addr)) < 0) {
                cout << "ERROR connecting on RM sockets\n" << endl;
                continue;
            }

            rm_connect_sockets_fd.insert({*connectSocket, rm_connect_socket_addr});
            cout << "Connected to socket " << *connectSocket << " and port " << ntohs(rm_connect_socket_addr.sin_port) << endl;

            std::pair<int *, Server *> *args = (std::pair<int *, Server *> *) calloc(1, sizeof(std::pair<int *, Server *>));

            // Send pointer of the previously allocated address and be able to access it's value in new thread's execution
            args->first = connectSocket;
            args->second = this;

            pthread_t connectedRMThread;
            pthread_create(&connectedRMThread, NULL, handleRMCommunication, (void *) args);
        }
    }

/**
* This method creates listener sockets based on RM number
*/
    void Server::createRMListenerSocket() {
        int opt = 1;
        int rmListeningPort = RM_BASE_PORT_NUMBER + rmNumber;
        rm_listening_serv_addr.sin_port = htons(rmListeningPort);

        // Create socket file descriptor
        if ((rm_listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
            cout << "ERROR opening socket\n" << endl;
            exit(1);
        }

        // Forcefully attaching socket to the port
        if (setsockopt(rm_listening_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        // Attach socket to server's port
        if (::bind(rm_listening_socket_fd, (struct sockaddr *) &rm_listening_serv_addr, sizeof(rm_listening_serv_addr)) < 0) {
            cout << "ERROR on binding\n" << endl;
            exit(1);
        }

        // Configure socket to listen for tcp connections
        if (::listen(rm_listening_socket_fd, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
        {
            cout << "ERROR on listening\n" << endl;
            exit(1);
        }

        cout << "\nSetting listener socket for RM machine of number " << rmNumber << endl;
        cout << "Listening for RM connections on socket: " << rm_listening_socket_fd << " and port " << ntohs(
                rm_listening_serv_addr.sin_port) << endl;

        pthread_t acceptRMConnectionThread;
        pthread_create(&acceptRMConnectionThread, NULL, acceptRMConnection, (void *) this);
    }

    void *Server::acceptRMConnection(void *param) {
        while(1) {
            int *newsockfd = (int *) calloc(1, sizeof(int *));
            socklen_t clilen = sizeof(struct sockaddr_in);

            // reference to this class
            Server *_this;
            _this = (Server *) param;

            if ((*newsockfd = accept(_this->rm_listening_socket_fd, (struct sockaddr *) &_this->rm_listening_serv_addr,
                                     &clilen)) == -1) {
                cout << "ERROR on accept\n" << endl;
                cout << "Could not connect to port " << _this->rm_listening_serv_addr.sin_port << endl;
            }
            else {
                std::cout << "Máquina conectada pelo socket " << *newsockfd << " de porta " << ntohs(_this->rm_listening_serv_addr.sin_port) << std::endl;

                std::pair<int *, Server *> *args = (std::pair<int *, Server *> *) calloc(1, sizeof(std::pair<int *, Server *>));
                _this->rm_connect_sockets_fd.insert({*newsockfd, _this->rm_listening_serv_addr});

                cout << "### Vetor de sockets de Replicacao ###" << endl;
                _this->printRMConnections();
                cout << "######################################" << endl;

                // Send pointer of the previously allocated address and be able to access it's value in new thread's execution
                args->first = newsockfd;
                // Also, send reference of this instance to the new thread
                args->second = _this;

                pthread_t listenRMThread;
                pthread_create(&listenRMThread, NULL, handleRMCommunication, (void *) args);
            }
        }
    }

    void *Server::handleRMCommunication(void *args)
    {
        // We cast our receve id void* args to a pair*
        std::pair<int *, Server *> *args_pair = (std::pair<int *, Server *> *) args;

        // Read socket pointer's value and free the previously allocated memory in the main thread
        int rm_socket_fd = *(int *) args_pair->first;
        free(args_pair->first);

        // Create a reference of the instance in this thread
        Server *_this = (Server*)calloc(1, sizeof(Server*));
        _this = (Server *) args_pair->second;

        // Free pair created for sending arguments
        free(args_pair);

        if(!isPrimaryServer) {
            bool connectedClient = true;
            while (connectedClient) {
                _this->sockets_connections_semaphore->wait();
                cout << "[Communication Thread] - Waiting socket " << rm_socket_fd << " messages" << endl;
                _this->sockets_connections_semaphore->post();
                // Listen for an incoming Packet from client
                Packet *receivedPacket = _this->readPacket(rm_socket_fd, &connectedClient);
                cout << "Received packet from socket " << rm_socket_fd << " of type " << receivedPacket->type << endl;
                if (!connectedClient) {
                    // Free allocated memory for reading Packet
                    free(receivedPacket);
                    break;
                }

                string address(receivedPacket->frontEndAddress);
                //create field on socket to store customer socket id
                if (receivedPacket->isMessage()) {
                    receivedPacket->type = REPLICATION_PACKET;
                    _this->groupManager->processReceivedPacket(receivedPacket);
                } else if (receivedPacket->isJoinMessage()) {
                    std::cout << "[DEBUG] recebi joinMessage" << std::endl;
                    _this->registerUserToServer(receivedPacket, address); // considers the front end connection
                } else if (receivedPacket->isDisconnect()) {
                    pair<string, string> connectionId = pair<string, string>();
                    connectionId.first.assign(receivedPacket->user_id);
                    connectionId.second.assign(address);
                    std::cout << "[DEBUG] vou chamar a rotina para o disconnect do fe " <<  connectionId.first << "endereco" << receivedPacket->frontEndAddress << std::endl;
                    _this->closeClientConnection(connectionId); // considers the front end connection
                }

                cout << "Sending replication packet confirmation to Primary Server from socket " << rm_socket_fd << endl;
                Packet *confirmationPackage = new Packet();
                confirmationPackage->type = REPLICATION_CONFIRMATION_PACKET;
                _this->sendPacket(rm_socket_fd, confirmationPackage);
            }

            auto socket_it = _this->rm_connect_sockets_fd.find(rm_socket_fd);
            _this->rm_connect_sockets_fd.erase(socket_it);

            close(rm_socket_fd);
        }
        return 0;
    }


    void Server::printRMConnections() const {
        sockets_connections_semaphore->wait();
        cout << " rm sockets map size " << rm_connect_sockets_fd.size() << endl;
        for ( const pair<int, struct sockaddr_in> &connectedMachine : rm_connect_sockets_fd)
        {
            cout << "Conectado ao servidor RM de porta " << ntohs(connectedMachine.second.sin_port) << " e socket "
                 << connectedMachine.first << endl;
        }
        sockets_connections_semaphore->post();
    }

    void Server::setRmNumber(int _rmNumber) {
        this->rmNumber = _rmNumber;
    }
    int Server::getRmNumber() {
        return this->rmNumber;
    }
}