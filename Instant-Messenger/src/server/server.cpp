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
        feAddressesSemaphore = new Semaphore(1);

        connectionMonitor = new ConnectionMonitor();

        socketFeList = vector<int>(); // TODO: remove
        feAddresses = vector<string>();
        this->feAddressBook = FeAddressBook();
        groupManager = new ServerGroupManager(this->feAddressBook);

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
        int socketId = getSocketFromAddress(feAddress);
        this->sockets_connections_semaphore->wait();
        if (incrementNumberOfConnectionsFromUser(registrationPacket->username) == USER_SESSIONS_LIMIT_REACHED ) { // we can keep this
            Packet *connectionRefusedPacket = new Packet(CONNECTION_REFUSED_PACKET);
            strcpy(connectionRefusedPacket->user_id, registrationPacket->user_id);
            std::cout << "[DEBUG|ERROR] limit sessoes alcanadas pelo user" << std::endl;
            this->sendPacket(socketId, connectionRefusedPacket);
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

        this->feAddressBook.registryAddressSocket(feIpPort, newSocketFE);
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
            this->feConnectionInitializationSemaphore->wait();
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
        while (connectedFrontEnd) {
            // Listen for an incoming Packet from client
            Packet *receivedPacket = _this->readPacket(feSocket, &connectedFrontEnd);
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
                feAddressBook.removeServerSocket(feAddress);
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
        int socketId = feAddressBook.getInternalSocketId(feAddress);

        if (socketId == 0) {
            cout << "[ERROR] the address provided for the FE does not have any socket" << endl;
        }
        return socketId;
    }

}