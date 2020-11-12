#include "../../include/server/server.hpp"
#include "../../include/util/definitions.hpp"
#include "../../include/client/ConnectionKeeper.hpp"

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>
#include "../../include/util/exceptions.hpp"

#endif

namespace server {
    vector<int> Server::openSockets;

    Server::Server() {
        // Init semaphore for openSockets
        sockets_connections_semaphore = new Semaphore(1);
        groupManager = new ServerGroupManager();
        connectionMonitor = new ConnectionMonitor();

        // TODO assim como proxyFE, o server vai ter que ter 2 sockets, um pra conectar nos FE e outro nos RM
        {
            // Configure server address properties
            // serv_addr.sin_family = AF_INET;
            // serv_addr.sin_addr.s_addr = INADDR_ANY;
            // bzero(&(serv_addr.sin_zero), 8);

            rm_listening_serv_addr.sin_family = AF_INET;
            rm_listening_serv_addr.sin_addr.s_addr = INADDR_ANY;
            bzero(&(rm_listening_serv_addr.sin_zero), 8);
        }

        // Initialize socket file descriptor
        socket_fd = 0;
        rm_listening_socket_fd = 0;
    }

    void Server::setRmNumber(int rmNumber) {
        this->rmNumber = rmNumber;
    }
    int Server::getRmNumber(){
        return this->rmNumber;
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
            //pthread_create(&connectedRMThread, NULL, handleConnectedRMCommunication, (void *) args);
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


    void Server::closeClientConnection(int socket_fd) {
        std::cout << "Closing socket: " << socket_fd << std::endl;
        openSockets.erase(std::remove(openSockets.begin(), openSockets.end(), socket_fd), openSockets.end());
        close(socket_fd);
    }


    void Server::closeFrontEndConnections() {
        cout << "Number of front end connections: " << openSockets.size() << endl;
        // std::for_each(openSockets.begin(), openSockets.end(), close);
        std::for_each(openSockets.begin(), openSockets.end(), close);
        cout << "Number of front end connections: " << openSockets.size() << endl;
    }


    void Server::closeSocket() {
        std::cout << "server_socket_fd: " << socket_fd << std::endl;
        close(socket_fd);
        cout << "Closing server socket..." << endl;
    }


    void Server::closeServer() {
        closeFrontEndConnections();
        closeSocket();
    }


    // TODO adaptar pra quando tiver 2 sockets, ou pelo menos deixa o TODO aqui
    void Server::setPort(int port) {
        /*serv_addr.sin_port = htons(atoi(port)); */
    }

    // Same as above. BTW it doesn't mean to remove it
    void Server::prepareConnection() {
        /*int opt = 1;

        // Create socket file descriptor
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
            cout << "ERROR opening socket\n" << endl;
            exit(1);
        }

        // Forcefully attaching socket to the port
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
        { 
            perror("setsockopt"); 
            exit(EXIT_FAILURE); 
        } 

        // Attach socket to server's port
        if (::bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            cout << "ERROR on binding\n" << endl;
            exit(1);
        }

        // Configure socket to listen for tcp connections
        if (listen(socket_fd, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
        {
            cout << "ERROR on listening\n" << endl;
            exit(1);
        }

        std::cout << "server_socket_fd preparedConnection: " << socket_fd << std::endl;*/
    }


    // TODO acho que isso so faz sentido pra um socket que recebe conexões, em modo server.
    void Server::printPortNumber() {
        socklen_t len = sizeof(serv_addr);

        if (getsockname(socket_fd, (struct sockaddr *) &serv_addr, &len) < 0) {
            cout << "Unable to print port Number!" << endl;
            exit(1);
        }

        cout << "Server running on PORT: " << ntohs(serv_addr.sin_port) << endl;
    }


    /**
     * Only used to forward the message to the proper group. The group turns himself to deliver
     * @param socket
     * @param username
     * @param group
     * @return
     */
    int Server::registerUser(pair<int, int> clientIdentifier, char *username, char *group, char* userID) {
        return groupManager->registerUserToGroup(clientIdentifier, username, group, userID);
    }

    int Server::registerUserToServer(Packet *registrationPacket, int frontEndSocket) {
        // Store new crated socket in the vector of existing connections sockets and increment counter for the user
        this->sockets_connections_semaphore->wait();
        if (incrementNumberOfConnectionsFromUser(registrationPacket->username) == USER_SESSIONS_LIMIT_REACHED ) { // we can keep this
            Packet *connectionRefusedPacket = new Packet(CONNECTION_REFUSED_PACKET);
            strcpy(connectionRefusedPacket->user_id, registrationPacket->user_id);
            std::cout << "[DEBUG|ERROR] limit sessoes alcanadas pelo user" << std::endl;
            this->sendPacket(frontEndSocket, connectionRefusedPacket);
            this->sockets_connections_semaphore->post();
            return -1;
        }
        this->sockets_connections_semaphore->post();

        std::pair<int, int> *clientFrontEndIdentifier = (std::pair<int, int> *) calloc(1, sizeof(std::pair<int, int>)); // this is the identifier for the client and we need to store this in the groups

        clientFrontEndIdentifier->first = registrationPacket->clientDispositiveIdentifier; //TODO aqui mandar user_id e mudar tipos das funções
        clientFrontEndIdentifier->second = frontEndSocket;

        int registered = this->registerUser(*clientFrontEndIdentifier, registrationPacket->username, registrationPacket->group, registrationPacket->user_id);

        return 0;
    }

    // TODO think in IP as parameter etc...
    int Server::ConnectToFE()
    {
        // TODO essa é a parte do socket pros FE
        {
            //TODO ip via params ou arquivos de texto
            char ip_address[10] = "127.0.0.1";
            char port[5] = "6969";
            serv_addr.sin_family = AF_INET;    
            serv_addr.sin_port = htons(atoi(port));     
            if(inet_pton(AF_INET, ip_address, &serv_addr.sin_addr)<=0) 
            { 
                std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
            } 

            bzero(&(serv_addr.sin_zero), 8); 
        }

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            cout << "\n Socket creation error \n" << endl;
            return -1;
        }

        if (connect(socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
            cout << "ERROR connecting\n" << endl;
            return -1;
        }

        std::cout << "conectado ao FE com socket:" << this->socket_fd << std::endl;

        return 0;
    }

    int Server::handleFrontEndConnection(pthread_t *tid, pthread_t *tid2) { //TODO same as in other places, tids mess
        pthread_create(tid, NULL, listenFrontEndCommunication, (void *) this);
        //pthread_create(tid2, NULL, monitorConnection, (void *) this);
        //ConnectionKeeper(this->socket_fd); // starts the thread that keeps sending keep alives
        return 0;
    }
    void * Server::monitorConnection(void *args) {
        Server* _this = (Server *) args;

        cout << "monitorConnection" << endl;
        _this->connectionMonitor->monitor(&(_this->socket_fd));
        _this->closeFrontEndConnection(_this->socket_fd);

        return NULL;
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
                exit(1);
            }

            std::cout << "Máquina conectada pelo socket " << _this->rm_listening_socket_fd << " de porta " << ntohs(_this->rm_listening_serv_addr.sin_port) << std::endl;

            std::pair<int *, Server *> *args = (std::pair<int *, Server *> *) calloc(1, sizeof(std::pair<int *, Server *>));
            _this->rm_connect_sockets_fd.insert({_this->rm_listening_socket_fd, _this->rm_listening_serv_addr});

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

        bool connectedClient = true;
        while (connectedClient)
        {
            cout << "[Communication Thread] - Waiting socket " << rm_socket_fd << " messages" << endl;
            // Listen for an incoming Packet from client
            // todo: we will receive messages here and we need to process them accordingly
            Packet *receivedPacket = _this->readPacket(rm_socket_fd, &connectedClient);
            //cout << "Received another connection!" << endl;
            if (!connectedClient)
            {
                // Free allocated memory for reading Packet
                free(receivedPacket);
                break;
            }
        }

        //todo: remove sockets from list after closing     '

        close(rm_socket_fd);
        return 0;
    }

    void *Server::listenFrontEndCommunication(void *args) {
        Server* _this = (Server *) args;
        bool connectedFrontEnd = true;
        while (connectedFrontEnd) {
            // Listen for an incoming Packet from client
            Packet *receivedPacket = _this->readPacket(_this->socket_fd, &connectedFrontEnd);
            if (!connectedFrontEnd) {
                // Free allocated memory for reading Packet
                free(receivedPacket);
                break;
            }

            if (receivedPacket->isMessage()) {
                cout << "[DEBUG] recebi message" << receivedPacket->message << endl;
                Packet *pack = new Packet();
                pack->type = ACK_PACKET;
                strcpy(pack->user_id, receivedPacket->user_id);
                _this->sendPacket(_this->socket_fd, pack);
                std::cout << "[DEBUG] mandei ACK para socket: " << _this->socket_fd << std::endl;
                _this->groupManager->processReceivedPacket(receivedPacket);
            } else if (receivedPacket->isKeepAlive()){
                _this->connectionMonitor->refresh(_this->socket_fd);
            } else if (receivedPacket->isJoinMessage()) {
                std::cout << "[DEBUG] recebi joinMessage" << std::endl;
                _this->registerUserToServer(receivedPacket, _this->socket_fd); // considers the front end connection
            } else if (receivedPacket->isDisconnect()) {
                pair<int, int> connectionId = pair<int, int>();
                connectionId.first = receivedPacket->clientDispositiveIdentifier;
                connectionId.second = _this->socket_fd;
                _this->closeClientConnection(connectionId); // considers the front end connection
            } else if (receivedPacket->isReplicationMessage()) {
                cout << "[DEBUG] recebi replicacao" << receivedPacket->message << endl;
                _this->groupManager->processReceivedPacket(receivedPacket);
            }

            // verify if should send replication socket or not
            if(_this->getIsPrimaryServer() && (receivedPacket->isMessage() || receivedPacket->isJoinMessage() || receivedPacket->isDisconnect())) {
                for(auto socket : _this->rm_connect_sockets_fd) {
                    receivedPacket->type = REPLICATION_MESSAGE;
                    _this->sendPacket(socket.first, receivedPacket); // send message for each socket on RM socket list
                }
            }
        }

        _this->connectionMonitor->killSocket(_this->socket_fd);
        // Close all properties related to client connection
        _this->closeFrontEndConnection(_this->socket_fd);

        return 0;
    }

    /**
     * When the FE dies, let's process it and kill all the connections that we had for the users
     * @param socketId
     */
    void Server::closeFrontEndConnection(int socketId) {
        pair <int, int> connectionId = pair<int, int>();
        connectionId.first = FE_DISCONNECT;
        connectionId.second = socketId;
        closeClientConnection(connectionId);
//        close(socketId); TODO: fechar a conexão do FE em si
    }

    void Server::closeClientConnection(pair<int, int> clientConnectionId) {
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

    void Server::printRMConnections() const {
        cout << " rm sockets map size " << rm_connect_sockets_fd.size() << endl;
        for ( const pair<int, struct sockaddr_in> &connectedMachine : rm_connect_sockets_fd)
        {
            cout << "Conectado ao servidor RM de porta " << ntohs(connectedMachine.second.sin_port) << " e socket "
                 << connectedMachine.first << endl;
        }
    }

    bool Server::getIsPrimaryServer()
    {
        return this->isPrimaryServer;
    }
    void Server::setIsPrimaryServer(bool value)
    {
        this->isPrimaryServer = value;
    }
}