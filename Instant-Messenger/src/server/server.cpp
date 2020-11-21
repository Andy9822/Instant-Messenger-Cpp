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

    Server::Server() {
        // Init semaphore for openSockets
        sockets_connections_semaphore = new Semaphore(1);
        feConnectionInitializationSemaphore = new Semaphore(1);
        feSocketsSemaphore = new Semaphore(1);

        connectionMonitor = new ConnectionMonitor();

        socketFeList = vector<int>();
        this->feAddressBook = FeAddressBook();
        groupManager = new ServerGroupManager(this->feAddressBook);

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
            this->sendPacket(frontEndSocket, connectionRefusedPacket);
            this->sockets_connections_semaphore->post();
            return -1;
        }
        this->sockets_connections_semaphore->post();

        std::pair<string, string> clientFrontEndIdentifier = std::pair<string, string>(); // this is the identifier for the client and we need to store this in the groups
        clientFrontEndIdentifier.first.assign(registrationPacket->user_id);
        clientFrontEndIdentifier.second.assign(registrationPacket->feAddress);

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

        this->feAddressBook.registryAddressSocket(feIpPort, newSocketFE);

        return 0;
    }

    int Server::handleFrontEndsConnections() { //TODO same as in other places, tids mess
        std::pair<string, Server *> *args = (std::pair<string, Server *> *) calloc(1, sizeof(std::pair<string, Server *>));
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
        while (connectedFrontEnd) {
            // Listen for an incoming Packet from client
            Packet *receivedPacket = _this->readPacket(fe_socketfd, &connectedFrontEnd);
            if (!connectedFrontEnd) {
                // Free allocated memory for reading Packet
                free(receivedPacket);
                break;
            }

//            cout << "[DEBUG] FE listenFrontEndCommunication " << fe_socketfd <<  endl;

            if (receivedPacket->isMessage()) {
                cout << "[DEBUG] recebi message" << receivedPacket->message << endl;
                //TODO: send this to the backup servers
                // replicateToBackupServers();
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
}