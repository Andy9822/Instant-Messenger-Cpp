#include "../../include/proxy_fe/ProxyFE.hpp"


std::map<int, std::pair<pthread_t, time_t>> openClientsSockets;
bool ProxyFE::online_RMserver;
int ProxyFE::serverRM_socket;

ProxyFE::ProxyFE() {
    // Init semaphore for openClientsSockets 
    openClientsSockets_semaphore = new Semaphore(1);

    // Init semaphore for online_RMserver and set variable to false
    online_semaphore = new Semaphore(1);
    online_RMserver = false;

    // Init server reconnect mutex
    pthread_mutex_init(&mutex_server_reconnect, NULL);

    // Server reconnect mutex inits locked as initially it's everything connected and fine
    pthread_mutex_lock(&mutex_server_reconnect);

    // Configure serverRM address properties
    serv_sock_addr.sin_family = AF_INET;
    serv_sock_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_sock_addr.sin_zero), 8);

    // Configure clients address properties
    client_sock_addr.sin_family = AF_INET;
    client_sock_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(client_sock_addr.sin_zero), 8);

    // Initialize socket file descriptor
    server_socket_fd = 0;
    clients_socket_fd = 0;
}

void ProxyFE::setPortServerRM(int port) {
    serv_sock_addr.sin_port = htons(port);
}

void ProxyFE::setPortClients(int port) {
    client_sock_addr.sin_port = htons(port);
}

void ProxyFE::prepareSocketConnection(int* socket_fd, sockaddr_in* serv_addr) 
{
    int opt = 1;

    // Create socket file descriptor
    if ((*socket_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        cout << "ERROR opening socket\n" << endl;
        exit(1);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

    // Attach socket to server's port
    if (::bind(*socket_fd, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
        cout << "ERROR on binding\n" << endl;
        exit(1);
    }

    // Configure socket to listen for tcp connections
    if (listen(*socket_fd, MAXBACKLOG) < 0) // SOMAXCONN is the maximum value of backlog
    {
        cout << "ERROR on listening\n" << endl;
        exit(1);
    }
}

void ProxyFE::prepareServerConnection()
{
    prepareSocketConnection(&server_socket_fd, &serv_sock_addr);
    std::cout << "Prepared server socket_fd: " << server_socket_fd << " ✅" << std::endl;
}

void ProxyFE::prepareClientsConnection()
{
    prepareSocketConnection(&clients_socket_fd, &client_sock_addr);
    std::cout << "Prepared clients socket_fd: " << clients_socket_fd << " ✅" << std::endl;
}

void ProxyFE::prepareConnection()
{
    prepareServerConnection();
    prepareClientsConnection();
}


void ProxyFE::printPortNumber() {
    socklen_t len_srv = sizeof(serv_sock_addr);
    socklen_t len_cli = sizeof(client_sock_addr);

    if (getsockname(server_socket_fd, (struct sockaddr *) &serv_sock_addr, &len_srv) < 0) {
        cout << "Unable to print port Number!" << endl;
        exit(1);
    }
    cout << "Listening ServerRM connection on PORT " << ntohs(serv_sock_addr.sin_port) << endl;

    if (getsockname(clients_socket_fd, (struct sockaddr *) &client_sock_addr, &len_cli) < 0) {
        cout << "Unable to print port Number!" << endl;
        exit(1);
    }
    cout << "Listening clients connections on PORT: " << ntohs(client_sock_addr.sin_port) << endl;

}

int ProxyFE::handleServerConnection() {
    int status;

    // Allocate memory space to store value in heap and be able to use it after this function ends
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    if ((newsockfd = accept(server_socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        cout << "ERROR on accept\n" << endl;
        pthread_mutex_unlock(&mutex_server_reconnect);
        return -1;
    }

    std::cout << "Primary RM server connected with socket: " << newsockfd << std::endl;
    
    // Save server RM socket to openSockets map
    serverRM_socket = newsockfd;
    openClientsSockets_semaphore->wait();
    openClientsSockets[serverRM_socket] = make_pair(NULL, time(0));
    openClientsSockets_semaphore->post();
    
    // Set application as online
    online_semaphore->wait();
    online_RMserver = true;
    online_semaphore->post();

    std::cout << "✅ ServerRM connected - System is Online ✅" << std::endl;

    return 0; 
}

void* ProxyFE::listenServerReconnect(void* args)
{
    ProxyFE* _this = (ProxyFE *) args;
    while (true)
    {
        pthread_mutex_lock(&(_this->mutex_server_reconnect));
        std::cout << "Waiting for ServerRM reconnection..." << std::endl;
        _this->handleServerConnection();
    }
}

void ProxyFE::handleServerReconnect() 
{
    pthread_create(&reconnect_server_tid, NULL, listenServerReconnect, (void*) this);
}


int ProxyFE::handleClientConnection(pthread_t *tid) 
{
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    if ((newsockfd = accept(clients_socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        cout << "ERROR on accept\n" << endl;
        return -1;
    }


    //Create a pair for sending more than 1 parameter to the new thread that we are about to create
    std::pair<int, ProxyFE*>* args = (std::pair<int, ProxyFE*>*) calloc(1, sizeof(std::pair<int, ProxyFE*>));

    args->first = newsockfd;

    // Send reference of this instance to the new thread
    args->second = this;

    pthread_create(tid, NULL, listenClientCommunication, (void *) args);

    // Add client to openSockets map
    openClientsSockets_semaphore->wait();
    openClientsSockets[newsockfd] = make_pair(*tid, time(0));
    openClientsSockets_semaphore->post();

    return 0;
}

void* ProxyFE::listenClientCommunication(void *args) 
{
    // We cast our receveid void* args to a pair*
    std::pair<int, ProxyFE *> *args_pair = (std::pair<int, ProxyFE *>*) args;

    // Get socket from the new connected client
    int client_socketfd = args_pair->first;

    // Get a reference of the object instance in this thread
    ProxyFE *_this = (ProxyFE *) args_pair->second;

    // Free pair created for receiving arguments
    free(args_pair);

    bool is_client_connected = true;
    // TODO remover isso, é so pra fingir que teve OK e cliente ta apto a conectar
    {
        char messageBuffer[USERNAME_MAX_SIZE] = "welcome to FE land";
        char usernameBuffer[MESSAGE_MAX_SIZE] = "FE bro";
        char groupBuffer[GROUP_MAX_SIZE] = "algum group";
        _this->readPacket(client_socketfd, &is_client_connected);
        _this->sendPacket(client_socketfd, new Packet(usernameBuffer, groupBuffer, messageBuffer, time(0)));
    }

    while (is_client_connected) {
        // Listen for an incoming Packet from client
        Packet *receivedPacket = _this->readPacket(client_socketfd, &is_client_connected);
        if (!is_client_connected) {
            // Free allocated memory for reading Packet
            free(receivedPacket);
            break;
        }
        std::cout << "recebi: " << receivedPacket->message << std::endl;
    }

    
    std::cout << "Client socket " << client_socketfd <<  " has disconnected" << std::endl;
    handleSocketDisconnection(client_socketfd, _this);
    
    return NULL;
}

void ProxyFE::handleSocketDisconnection(int socket, ProxyFE* _this)
{
    _this->openClientsSockets_semaphore->wait();
    _this->openClientsSockets.erase(socket);
    _this->openClientsSockets_semaphore->post();
}

void* ProxyFE::monitorKeepAlivesAux(void* args) 
{

    ProxyFE* _this = (ProxyFE *) args;
    int checking_socket;
    while (true)
    {
        sleep(10);

        _this->openClientsSockets_semaphore->wait();
        time_t checking_time = time(0);
        vector<int> expired_sockets;  

        std::map<int, std::pair<pthread_t, time_t>>::iterator it;
        // Iterate over openSockets map and check when was received each socket's last keep alive
        for (it = _this->openClientsSockets.begin(); it != _this->openClientsSockets.end(); it++ )
        {
            checking_socket = it->first;
            time_t last_keep_alive = it->second.second;
            if (checking_time - last_keep_alive >= 10)
            {
                expired_sockets.push_back(checking_socket);
            }
        }
        // If there are expired sockets, remove them from openSockets map and close its sockets_fd
        int expireds = expired_sockets.size();
        if (expireds)
        {
            for (int i = 0; i < expireds; i++)
            {
                checking_socket = expired_sockets[i];

                // Close socket and interrupt any pending transmission
                shutdown(checking_socket, 2);
                close(checking_socket);

                // If it's ServerRM's socket then signal that the application is now offline
                if (checking_socket == _this->serverRM_socket)
                {   
                    _this->online_semaphore->wait();
                    _this->online_RMserver = false;
                    _this->online_semaphore->post();
                    pthread_mutex_unlock(&(_this->mutex_server_reconnect));
                    std::cout << "❌❗❌ ServerRM went down - System is Offline ❌❗❌" << std::endl;
                    _this->openClientsSockets.erase(checking_socket);
                }
            }
        }
        _this->openClientsSockets_semaphore->post();        
    }
    
}

void ProxyFE::monitorKeepAlives() {
    pthread_create(&monitor_tid, NULL, monitorKeepAlivesAux, (void*) this);
}

    