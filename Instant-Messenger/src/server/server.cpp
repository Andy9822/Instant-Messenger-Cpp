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
        }

        // Initialize socket file descriptor
        socket_fd = 0;
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
            this->sendPacket(frontEndSocket, connectionRefusedPacket);
            delete connectionRefusedPacket;
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
        pthread_create(tid2, NULL, monitorConnection, (void *) this);
        ConnectionKeeper(this->socket_fd); // starts the thread that keeps sending keep alives
        return 0;
    }
    void * Server::monitorConnection(void *args) {
        Server* _this = (Server *) args;

        cout << "monitorConnection" << endl;
        _this->connectionMonitor->monitor(&(_this->socket_fd));
        _this->closeFrontEndConnection(_this->socket_fd);

        return NULL;
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
                cout << "[DEBUG] " << receivedPacket->message << ", " << receivedPacket->clientDispositiveIdentifier <<
                    " FE socket: " << _this->socket_fd << endl;
                //TODO: send this to the backup servers
                // replicateToBackupServers();
                Packet *pack = new Packet();
                pack->type = ACK_PACKET;
                strcpy(pack->user_id, receivedPacket->user_id);
                _this->sendPacket(_this->socket_fd, pack);
                std::cout << "[DEBUG] mandei ACK para socket: " << _this->socket_fd << std::endl;
                _this->groupManager->processReceivedPacket(receivedPacket);
            } else if (receivedPacket->isKeepAlive()){
                _this->connectionMonitor->refresh(_this->socket_fd);
            } else if (receivedPacket->isJoinMessage()) {
                _this->registerUserToServer(receivedPacket, _this->socket_fd); // considers the front end connection
            } else if (receivedPacket->isDisconnect()) {
                pair<int, int> connectionId = pair<int, int>();
                connectionId.first = receivedPacket->clientDispositiveIdentifier;
                connectionId.second = _this->socket_fd;
                _this->closeClientConnection(connectionId); // considers the front end connection
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
}