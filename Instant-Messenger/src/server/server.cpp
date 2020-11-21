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
        feSocketsSemaphore = new Semaphore(1);
        groupManager = new ServerGroupManager();
        connectionMonitor = new ConnectionMonitor();

        rm_listening_serv_addr.sin_family = AF_INET;
        rm_listening_serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(rm_listening_serv_addr.sin_zero), 8);

        rm_listening_socket_fd = 0;

        // TODO assim como proxyFE, o server vai ter que ter 2 sockets, um pra conectar nos FE e outro nos RM
        {
            // Configure server address properties
            // serv_addr.sin_family = AF_INET;
            // serv_addr.sin_addr.s_addr = INADDR_ANY;
            // bzero(&(serv_addr.sin_zero), 8);
        }
        socketFeList = vector<int>();

    }

    void Server::prepareReplicationManager(int rmNumber) {
        this->setRmNumber(rmNumber);
        this->connectToRmServers();

        if(this->getRmNumber() > 0) {
            this->createRMListenerSocket();
        }
    }

    void Server::closeFrontEndConnections() {
        cout << "Number of front end connections: " << socketFeList.size() << endl;
        this->feSocketsSemaphore->wait();
        std::for_each(socketFeList.begin(), socketFeList.end(), close);
        this->feSocketsSemaphore->post();
        cout << "Number of front end connections: " << socketFeList.size() << endl;
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
    int Server::registerUser(pair<char *, int> clientIdentifier, char *username, char *group) {
        return groupManager->registerUserToGroup(clientIdentifier, username, group);
    }

    int Server::registerUserToServer(Packet *registrationPacket, int frontEndSocket) {
        // Store new crated socket in the vector of existing connections sockets and increment counter for the user
        this->sockets_connections_semaphore->wait();
        if (incrementNumberOfConnectionsFromUser(registrationPacket->username) == USER_SESSIONS_LIMIT_REACHED ) { // we can keep this
            Packet *connectionRefusedPacket = new Packet(CONNECTION_REFUSED_PACKET);
            strcpy(connectionRefusedPacket->user_id, registrationPacket->user_id);
            std::cout << "[DEBUG|ERROR] limit sessoes alcanadas pelo user" << std::endl;

            //todo: do we need to know if on RM server the connection was refused?
            if(isPrimaryServer) {
                this->sendPacket(frontEndSocket, connectionRefusedPacket);
            }

            this->sockets_connections_semaphore->post();
            return -1;
        }
        this->sockets_connections_semaphore->post();

        std::pair<char *, int> clientFrontEndIdentifier = std::pair<char *, int>(); // this is the identifier for the client and we need to store this in the groups
        clientFrontEndIdentifier.first = (char*)malloc(UUID_SIZE*sizeof(char));
        strcpy(clientFrontEndIdentifier.first, registrationPacket->user_id);
        clientFrontEndIdentifier.second = frontEndSocket;

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
        std::cout << "conectado ao FE com socket:" << newSocketFE << std::endl;

        return 0;
    }

    int Server::handleFrontEndsConnections() { //TODO same as in other places, tids mess
        std::pair<int, Server *> *args = (std::pair<int, Server *> *) calloc(1, sizeof(std::pair<int, Server *>));
        vector<ConnectionKeeper*> connectionKeepers = vector<ConnectionKeeper*>();
        args->second = this;

        int i = 0;
        feSocketsSemaphore->wait();
        for ( int feSocket : this->socketFeList ) {
            this->feConnectionInitializationSemaphore->wait(); // the POST is done inside the new threads created only when the args is no longer necessary
            args->first = feSocket;
            pthread_create(&(this->tid[i++]), NULL, listenFrontEndCommunication, (void *) args);
            std::cout << "handleFrontEndsConnections ao FE com socket:" << feSocket << std::endl;
            this->feConnectionInitializationSemaphore->wait();
            pthread_create(&(this->tid[i++]), NULL, monitorConnection, (void *) args);
            connectionKeepers.push_back(new ConnectionKeeper(feSocket)); // starts the thread that keeps sending keep alives
        }
        feSocketsSemaphore->post();

        this->feConnectionInitializationSemaphore->wait();
        free(args);
        this->feConnectionInitializationSemaphore->post();

        return 0;
    }

    void * Server::monitorConnection(void *args) {
        std::pair<int, Server *> *args_pair = (std::pair<int, Server *> *) args;
        int fe_socketfd = (int) args_pair->first;
        Server *_this = (Server *) args_pair->second;

        _this->feConnectionInitializationSemaphore->post();

        _this->connectionMonitor->monitor(&fe_socketfd);
        _this->closeFrontEndConnection(fe_socketfd);
        return NULL;
    }

    void *Server::listenFrontEndCommunication(void *args) {

        // TODO: receive a pair from the args. The first will be the socket and the second one will be the server
        std::pair<int, Server *> *args_pair = (std::pair<int, Server *> *) args;
        int fe_socketfd = (int) args_pair->first;
        Server *_this = (Server *) args_pair->second;

        _this->feConnectionInitializationSemaphore->post();

        bool connectedFrontEnd = true;
        bool replicationError = false;

        while (connectedFrontEnd) {
            // Listen for an incoming Packet from client
            Packet *receivedPacket = _this->readPacket(fe_socketfd, &connectedFrontEnd);
            if (!connectedFrontEnd) {
                // Free allocated memory for reading Packet
                free(receivedPacket);
                break;
            }

            // verify if should send replication socket or not
            _this->sockets_connections_semaphore->wait();
            if(isPrimaryServer && (receivedPacket->isMessage() || receivedPacket->isJoinMessage() || receivedPacket->isDisconnect())) {
                cout << "Sending " << _this->rm_connect_sockets_fd.size() << " replication packets" << endl;

                for(auto socket : _this->rm_connect_sockets_fd) {
                    cout << "Sending packet of type " << receivedPacket->type << " to socket " << socket.first << endl;
                    _this->sendPacket(socket.first, receivedPacket); // send message for each socket on RM socket list
                    Packet *confirmationPacket = _this->readPacket(socket.first, &connectedFrontEnd); // wait for socket answer
                    cout << "Reading packet of type " << confirmationPacket->type << " on socket " << socket.first << endl;

                    if (!connectedFrontEnd) {
                        // Free allocated memory for reading Packet
                        free(receivedPacket);
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
                cout << "[DEBUG] recebi message" << receivedPacket->message << endl;
                Packet *pack = new Packet();
                pack->type = ACK_PACKET;
                strcpy(pack->user_id, receivedPacket->user_id);
                _this->sendPacket(fe_socketfd, pack);
                std::cout << "[DEBUG] mandei ACK para socket: " << fe_socketfd << std::endl;
                _this->groupManager->processReceivedPacket(receivedPacket);
            } else if (receivedPacket->isKeepAlive()){
                _this->connectionMonitor->refresh(fe_socketfd);
            } else if (receivedPacket->isJoinMessage()) {
                std::cout << "[DEBUG] recebi joinMessage" << std::endl;
                _this->registerUserToServer(receivedPacket, fe_socketfd); // considers the front end connection
            } else if (receivedPacket->isDisconnect()) {
                pair<char *, int> connectionId = pair<char *, int>();
                connectionId.first = (char*)malloc(UUID_SIZE*sizeof(char));
                strcpy(connectionId.first, receivedPacket->user_id);
                connectionId.second = fe_socketfd;

                std::cout << "[DEBUG] vou chamar a rotina para o disconnect do client " << receivedPacket->user_id << std::endl;
                _this->closeClientConnection(connectionId); // considers the front end connection
            }
        }

        _this->connectionMonitor->killSocket(fe_socketfd);
        // Close all properties related to client connection
        _this->closeFrontEndConnection(fe_socketfd);


        cout << "Stopped listening on FE socket " << fe_socketfd << endl;

        //exit(0);

        return 0;
    }

    /**
     * When the FE dies, let's process it and kill all the connections that we had for the users
     * @param socketId
     */
    void Server::closeFrontEndConnection(int socketId) {

        bool connectionFound = false;
        feSocketsSemaphore->wait();
        for (int feConnection : this->socketFeList) {
            if (feConnection == socketId) {
                connectionFound = true;
                break;
            }
        }
        feSocketsSemaphore->post();

        if (!connectionFound) return; // cai fora pq a gente já fechou esse socket

        cout << "[DEBUG] closeFrontEndConnection closing FE " << socketId << endl;
        pair <char *, int> connectionId = pair<char*, int>();
        connectionId.first = (char*)malloc(UUID_SIZE*sizeof(char));
        strcpy(connectionId.first, FE_DISCONNECT);
        connectionId.second = socketId;
        closeClientConnection(connectionId);
        if ((close(socketId)) == 0) {
            std::cout << "\nClosed socket: " << socketId << std::endl;
        } else {
            std::cout << "!!! Fatal error closing socket!!!!" << std::endl;
        }
        eraseSocketFromFeSocketList(socketId);
    }

    void Server::eraseSocketFromFeSocketList(int socketId) {
        feSocketsSemaphore->wait();
        int deletion_index = 0;
        auto begin = socketFeList.begin();
        for (auto socket : socketFeList) {
            if ( socket == socketId ) {
                socketFeList.erase(begin + deletion_index); // will delete the deletion_index's item
            }
            deletion_index += 1;
        }
        feSocketsSemaphore->post();
    }

    void Server::closeClientConnection(pair<char *, int> clientConnectionId) {
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
            }

            if ((*connectSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                cout << "\n RM Socket creation error \n" << endl;
            }

            if (connect(*connectSocket, (struct sockaddr *) &rm_connect_socket_addr, sizeof(rm_connect_socket_addr)) < 0) {
                cout << "ERROR connecting on RM sockets\n" << endl;
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

                //create field on socket to store customer socket id
                if (receivedPacket->isMessage()) {
                    receivedPacket->type = REPLICATION_PACKET;
                    _this->groupManager->processReceivedPacket(receivedPacket);
                } else if (receivedPacket->isJoinMessage()) {
                    cout << " Registering user to group " << endl;
                    receivedPacket->type = REPLICATION_PACKET;
                    _this->registerUserToServer(receivedPacket, receivedPacket->frontEndSocket); // considers the front end connection
                } /*else if (receivedPacket->isDisconnect()) {
                    receivedPacket->type = REPLICATION_PACKET;
                    pair<int, int> connectionId = pair<int, int>();
                    connectionId.first = receivedPacket->clientDispositiveIdentifier;
                    connectionId.second = receivedPacket->clientSocket;
                    _this->closeClientConnection(connectionId); // considers the front end connection
                }*/

                cout << "Sending replication packet confirmation to Primary Server from socket " << rm_socket_fd
                     << endl;
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

    void Server::sendMockDataToRMServers() {
        sockets_connections_semaphore->wait();
        bool connectedFrontEnd = true;

        cout << "Sending " << rm_connect_sockets_fd.size() << " replication packets" << endl;
        for ( const pair<int, struct sockaddr_in> &connectedMachine : rm_connect_sockets_fd)
        {
            Packet *joinPacket = new Packet("Joao", "TESTE2", "Mensagem do servidor primario de registro de usuario", NULL);
            joinPacket->type = JOIN_PACKET;
            cout << "Sending JOIN packet of type " << joinPacket->type << " to socket " << connectedMachine.first << " on port " << ntohs(connectedMachine.second.sin_port) << endl ;
            sendPacket(connectedMachine.first, joinPacket);
            Packet *joinConfirmationPacket = readPacket(connectedMachine.first, &connectedFrontEnd); // wait for socket answer
            cout << "Reading JOIN packet of type " << joinConfirmationPacket->type << " on socket " << connectedMachine.first << " on port " << ntohs(connectedMachine.second.sin_port) << endl;

            Packet *messagePacket = new Packet("Joao", "TESTE2", "replicacao VOANDO GARAI", NULL);
            joinPacket->type = MESSAGE_PACKET;
            cout << "Sending MESSAGE packet of type " << messagePacket->type << " to socket " << connectedMachine.first << " on port " << ntohs(connectedMachine.second.sin_port) << endl ;
            sendPacket(connectedMachine.first, messagePacket);
            Packet *messageConfirmationPacket = readPacket(connectedMachine.first, &connectedFrontEnd); // wait for socket answer
            cout << "Reading MESSAGE packet of type " << messageConfirmationPacket->type << " on socket " << connectedMachine.first << " on port " << ntohs(connectedMachine.second.sin_port) << endl;
        }
        cout << "All packets were replicate successfully" << endl;
        sockets_connections_semaphore->post();
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