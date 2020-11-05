#include "../../include/server/server.hpp"
#include "../../include/util/definitions.hpp"

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

        // Configure server address properties
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(serv_addr.sin_zero), 8);

        // Initialize socket file descriptor
        socket_fd = 0;
    }


    void Server::closeClientConnection(int socket_fd) {
        std::cout << "Closing socket: " << socket_fd << std::endl;
        openSockets.erase(std::remove(openSockets.begin(), openSockets.end(), socket_fd), openSockets.end());
        close(socket_fd);
    }


    void Server::closeConnections() {
        cout << "Number of client connections: " << openSockets.size() << endl;
        // std::for_each(openSockets.begin(), openSockets.end(), close);
        std::for_each(openSockets.begin(), openSockets.end(), closeClientConnection);
        cout << "Number of client connections: " << openSockets.size() << endl;
    }


    void Server::closeSocket() {
        std::cout << "server_socket_fd: " << socket_fd << std::endl;
        close(socket_fd);
        cout << "Closing server socket..." << endl;
    }


    void Server::closeServer() {
        closeConnections();
        closeSocket();
    }


    void Server::setPort(int port) {
        serv_addr.sin_port = htons(port);
    }

    void Server::prepareConnection() {
        int opt = 1;

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

        std::cout << "server_socket_fd preparedConnection: " << socket_fd << std::endl;
    }


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
    int Server::registerUser(int socket, char *username, char *group) {
        return groupManager->registerUserToGroup(socket, username, group);
    }


    int Server::registerUserToServer(void *args) {
        char username[USERNAME_MAX_SIZE] = {0};
        char group[GROUP_MAX_SIZE] = {0};
        int registered;

        std::pair<int *, Server *> *args_pair = (std::pair<int *, Server *> *) args;
        int client_socketfd = *(int *) args_pair->first;
        Server *_this = (Server *) args_pair->second;

        bool connectedClient = true;
        Packet *pack = new Packet();

        // getting client info
        Packet *receivedPacket = _this->readPacket(client_socketfd, &connectedClient);

        strncpy(username, receivedPacket->username, USERNAME_MAX_SIZE - 1);
        strncpy(group, receivedPacket->group, GROUP_MAX_SIZE - 1);

        // Store new crated socket in the vector of existing connections sockets and increment counter for the user
        _this->sockets_connections_semaphore->wait();
        if (incrementNumberOfConnectionsFromUser(username) == USER_SESSIONS_LIMIT_REACHED ) {
            pack->clientSocket = -1;
            _this->sendPacket(client_socketfd, pack);
            delete pack;
            return -1;
        }
        openSockets.push_back(client_socketfd);
        _this->sockets_connections_semaphore->post();

        registered = _this->registerUser(client_socketfd, username, group);

        // if there was already one entry with the same username before, we don't print <entered the group> a second time 
        if(registered == 1)
        {
	        pack->clientSocket = JOIN_QUIT_STATUS_MESSAGE;
	        _this->sendPacket(client_socketfd, pack);
	    }

        delete pack;
        return 0;
    }


    int Server::handleClientConnection(pthread_t *tid) {
        int status;
        pthread_t monitoringThread;

        // Allocate memory space to store value in heap and be able to use it after this function ends
        int *newsockfd = (int *) calloc(1, sizeof(int));
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(struct sockaddr_in);

        if ((*newsockfd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
            cout << "ERROR on accept\n" << endl;
            return -1;
        }

        // Registering user to server
        std::pair<int *, Server *> *user = (std::pair<int *, Server *> *) calloc(1, sizeof(std::pair<int *, Server *>));
        user->first = newsockfd;
        user->second = this;

        status = this->registerUserToServer(user);

        free(user);

        // if there are already two users with the same name -> close and ignore connection
        if (status < 0) {
            close(*newsockfd);
            return 0;
        }

        std::cout << "client connected with socket: " << *newsockfd << std::endl;

        //Create a pair for sending more than 1 parameter to the new thread that we are about to create
        std::pair<int *, Server *> *args = (std::pair<int *, Server *> *) calloc(1, sizeof(std::pair<int *, Server *>));

        // Send pointer of the previously allocated address and be able to access it's value in new thread's execution
        args->first = newsockfd;

        // Also, send reference of this instance to the new thread
        args->second = this;
        pthread_create(&monitoringThread, NULL, monitorConnection, (void *) args);
        pthread_create(tid, NULL, listenClientCommunication, (void *) args);

        return 0;
    }
    void * Server::monitorConnection(void *args) {
        std::pair<int *, Server *> *args_pair = (std::pair<int *, Server *> *) args;
        int client_socketfd = *(int *) args_pair->first;
        Server *_this = (Server *) args_pair->second;

        cout << "monitorConnection" << endl;
        _this->connectionMonitor->monitor(&client_socketfd);
        _this->closeListenClientCommunication(client_socketfd);
    }

    void *Server::listenClientCommunication(void *args) {
        // We cast our receveid void* args to a pair*
        std::pair<int *, Server *> *args_pair = (std::pair<int *, Server *> *) args;

        // Read socket pointer's value and free the previously allocated memory in the main thread
        int client_socketfd = *(int *) args_pair->first;
        free(args_pair->first);

        // Create a reference of the instance in this thread
        Server *_this = (Server *) args_pair->second;


        // Free pair created for sending arguments
        free(args_pair);

        bool connectedClient = true;
        while (connectedClient) {
            // Listen for an incoming Packet from client
            Packet *receivedPacket = _this->readPacket(client_socketfd, &connectedClient);
            if (!connectedClient) {
                // Free allocated memory for reading Packet
                free(receivedPacket);
                break;
            }

            if (!receivedPacket->isKeepAlive) {
                _this->groupManager->processReceivedPacket(receivedPacket);
            } else {
                _this->connectionMonitor->refresh(client_socketfd);
            }

        }

        _this->connectionMonitor->killSocket(client_socketfd);
        // Close all properties related to client connection
        _this->closeListenClientCommunication(client_socketfd);

        return 0;
    }

    void Server::closeListenClientCommunication(int client_socket) {

        sockets_connections_semaphore->wait();
        if(std::find(openSockets.begin(), openSockets.end(), client_socket) == openSockets.end()) {
            std::cout << "The connection was already closed: " << client_socket << std::endl;
            sockets_connections_semaphore->post();
            return;
        }
        std::cout << "\n\nFreeing allocated memory and closing client connection thread" << std::endl;
        openSockets.erase(std::remove(openSockets.begin(), openSockets.end(), client_socket), openSockets.end());
        groupManager->propagateSocketDisconnectionEvent(client_socket, this->connectionsCount);

        sockets_connections_semaphore->post();

        if ((close(client_socket)) == 0) {
            std::cout << "\nClosed socket: " << client_socket << std::endl;
        } else {
            std::cout << "!!! Fatal error closing socket!!!!" << std::endl;
        }
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