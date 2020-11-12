#include "../../include/server/ReplicationManager.hpp"
#include "../../include/util/definitions.hpp"
#include "../../include/server/server.hpp"
#include <unistd.h>

using namespace server;

ReplicationManager::ReplicationManager() {
    sockets_connections_semaphore = new Semaphore(1);

    rm_listening_serv_addr.sin_family = AF_INET;
    rm_listening_serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(rm_listening_serv_addr.sin_zero), 8);

    rm_listening_socket_fd = 0;
}

void ReplicationManager::connectToRmServers() {
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

        std::pair<int *, ReplicationManager *> *args = (std::pair<int *, ReplicationManager *> *) calloc(1, sizeof(std::pair<int *, ReplicationManager *>));

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
void ReplicationManager::createRMListenerSocket() {
    int opt = 1;
    int rmListeningPort = RM_BASE_PORT_NUMBER + rmNumber;
    rm_listening_serv_addr.sin_port = htons(rmListeningPort);

    //cout << "Listening on port " << rmListeningPort << endl;

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

void *ReplicationManager::acceptRMConnection(void *param) {
    while(1) {
        int *newsockfd = (int *) calloc(1, sizeof(int *));
        socklen_t clilen = sizeof(struct sockaddr_in);

        // reference to this class
        ReplicationManager *_this;
        _this = (ReplicationManager *) param;

        if ((*newsockfd = accept(_this->rm_listening_socket_fd, (struct sockaddr *) &_this->rm_listening_serv_addr,
                                 &clilen)) == -1) {
            cout << "ERROR on accept\n" << endl;
            cout << "Could not connect to port " << _this->rm_listening_serv_addr.sin_port << endl;
        }
        else {
            std::cout << "Máquina conectada pelo socket " << *newsockfd << " de porta " << ntohs(_this->rm_listening_serv_addr.sin_port) << std::endl;

            std::pair<int *, ReplicationManager *> *args = (std::pair<int *, ReplicationManager *> *) calloc(1, sizeof(std::pair<int *, ReplicationManager *>));
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

void *ReplicationManager::handleRMCommunication(void *args)
{
    // We cast our receve id void* args to a pair*
    std::pair<int *, ReplicationManager *> *args_pair = (std::pair<int *, ReplicationManager *> *) args;

    // Read socket pointer's value and free the previously allocated memory in the main thread
    int rm_socket_fd = *(int *) args_pair->first;
    free(args_pair->first);

    // Create a reference of the instance in this thread
    ReplicationManager *_this = (ReplicationManager*)calloc(1, sizeof(Server*));
    _this = (ReplicationManager *) args_pair->second;

    // Free pair created for sending arguments
    free(args_pair);

    bool connectedClient = true;
    while (connectedClient)
    {
        _this->sockets_connections_semaphore->wait();
        cout << "[Communication Thread] - Waiting socket " << rm_socket_fd << " messages" << endl;
        _this->sockets_connections_semaphore->post();
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

    auto socket_it = _this->rm_connect_sockets_fd.find(rm_socket_fd);
    _this->rm_connect_sockets_fd.erase(socket_it);

    close(rm_socket_fd);
    return 0;
}

void ReplicationManager::printRMConnections() const {
    sockets_connections_semaphore->wait();
    cout << " rm sockets map size " << rm_connect_sockets_fd.size() << endl;
    for ( const pair<int, struct sockaddr_in> &connectedMachine : rm_connect_sockets_fd)
    {
        cout << "Conectado ao servidor RM de porta " << ntohs(connectedMachine.second.sin_port) << " e socket "
             << connectedMachine.first << endl;
    }
    sockets_connections_semaphore->post();
}

void ReplicationManager::setRmNumber(int _rmNumber) {
    this->rmNumber = _rmNumber;
}
int ReplicationManager::getRmNumber() {
    return this->rmNumber;
}