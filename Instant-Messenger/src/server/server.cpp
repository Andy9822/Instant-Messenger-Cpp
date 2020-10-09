#include "../../include/server/server.hpp"
#include "../../include/util/definitions.hpp"

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>

#endif

namespace server {
    vector<int> Server::openSockets;
    sem_t Server::semaphore;

    Server::Server() {
        groupManager = new ServerGroupManager();

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
        // Create socket file descriptor
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            cout << "ERROR opening socket\n" << endl;
            exit(1);
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

        // Init binary semaphore for openSockets
        init_semaphore();

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

        // checking if there are already 2 users with the same name registered
        registered = _this->registerUser(client_socketfd, username, group);
        
        // if there are already two users with the same name, we close the connection
        if (registered < 0) {
            pack->clientSocket = -1;
            _this->sendPacket(client_socketfd, pack);
            delete pack;
            return -1;

        }
        // if there was already one entry with the same username before, we don't print <entered the group> a second time 
        else if(registered == 1)
        {
	        pack->clientSocket = JOIN_QUIT_STATUS_MESSAGE;
	        _this->sendPacket(client_socketfd, pack);
	    }

        delete pack;

        return 0;
    }


    int Server::handleClientConnection(pthread_t *tid) {
        int status;

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

        // Store new crated socket in the vector of existing connections sockets
        wait_semaphore();
        openSockets.push_back(*newsockfd);
        post_semaphore();

        //Create a pair for sending more than 1 parameter to the new thread that we are about to create
        std::pair<int *, Server *> *args = (std::pair<int *, Server *> *) calloc(1, sizeof(std::pair<int *, Server *>));

        // Send pointer of the previously allocated address and be able to access it's value in new thread's execution
        args->first = newsockfd;

        // Also, send reference of this instance to the new thread
        args->second = this;

        pthread_create(tid, NULL, clientCommunication, (void *) args);

        groupManager->printListOfUsers(); //debug purposes
        groupManager->printListOfGroups(); //debug purposes

        sleep(5);

        return 0;
    }


    void *Server::clientCommunication(void *args) {
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

            _this->groupManager->processReceivedPacket(receivedPacket);
        }

        // Close all properties related to client connection
        _this->closeClientCommunication(client_socketfd);

        return 0;
    }


    void Server::closeClientCommunication(int client_socket) {
        std::cout << "\n\nFreeing allocated memory and closing client connection thread" << std::endl;
        wait_semaphore();

        openSockets.erase(std::remove(openSockets.begin(), openSockets.end(), client_socket), openSockets.end());
        groupManager->disconnectUser(client_socket);

        post_semaphore();

        if ((close(client_socket)) == 0) {
            std::cout << "\nClosed socket: " << client_socket << std::endl;
        } else {
            std::cout << "!!! Fatal error closing socket!!!!" << std::endl;
        }
    }


    // TODO may be move functions below to a binary semaphore class and just extend that class
    void Server::init_semaphore() {
        sem_init(&semaphore, 0, 1);
    }

    void Server::wait_semaphore() {
        sem_wait(&semaphore);
    }

    void Server::post_semaphore() {
        sem_post(&semaphore);
    }

    void Server::configureFilesystemManager(int maxNumberOfMessagesInHistory) {
        groupManager->configureFileSystemManager(maxNumberOfMessagesInHistory);
    }
}