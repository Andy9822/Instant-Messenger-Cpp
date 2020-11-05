#include "../../include/proxy_fe/ProxyFE.hpp"


std::map<int, std::pair<int, time_t>> openClientsSockets;
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

    // Configure server address properties
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    // Initialize socket file descriptor
    socket_fd = 0;
}

void ProxyFE::setPort(int port) {
    serv_addr.sin_port = htons(port);
}

void ProxyFE::prepareConnection() {
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


void ProxyFE::printPortNumber() {
    socklen_t len = sizeof(serv_addr);

    if (getsockname(socket_fd, (struct sockaddr *) &serv_addr, &len) < 0) {
        cout << "Unable to print port Number!" << endl;
        exit(1);
    }

    cout << "Server running on PORT: " << ntohs(serv_addr.sin_port) << endl;
}

int ProxyFE::handleServerConnection() {
    int status;

    // Allocate memory space to store value in heap and be able to use it after this function ends
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    if ((newsockfd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        cout << "ERROR on accept\n" << endl;
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

    return 0; 
}

void* ProxyFE::listenServerReconnect(void* args)
{
    ProxyFE* _this = (ProxyFE *) args;
    while (true)
    {
        std::cout << "Waiting for reconnect" << std::endl;
        pthread_mutex_lock(&(_this->mutex_server_reconnect));
        _this->handleServerConnection();
    }
}

void ProxyFE::handleServerReconnect() 
{
    pthread_create(&reconnect_server_tid, NULL, listenServerReconnect, (void*) this);
}

int ProxyFE::handleClientConnection(pthread_t *tid) 
{
    return 0;
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

        std::map<int, std::pair<int, time_t>>::iterator it;
        // Iterate over openSockets map and check when was received each socket's last keep alive
        for (it = _this->openClientsSockets.begin(); it != _this->openClientsSockets.end(); it++ )
        {
            checking_socket = it->first;
            time_t last_keep_alive = it->second.second;
            if (checking_time - last_keep_alive > 10)
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
                // If it's a client socket, just close it
                if (checking_socket != _this->serverRM_socket)
                {
                    std::cout << "Cliente Caiu" << std::endl;
                    
                    // Acho que isso nem vai precisar, dando close o Socket.cpp já identifica erro
                    // pthread_cancel(_this->openClientsSockets[checking_socket].first);
                }
                // If it's server RM socket then signal that the application is now offline
                else
                {   
                    _this->online_semaphore->wait();
                    _this->online_RMserver = false;
                    _this->online_semaphore->post();
                    pthread_mutex_unlock(&(_this->mutex_server_reconnect));
                    std::cout << "Servidor Primario Caiu" << std::endl;
                }
                close(checking_socket);
                _this->openClientsSockets.erase(checking_socket);
            }
        }
        _this->openClientsSockets_semaphore->post();        
        string status = _this->online_RMserver ? "true" : "false";
        std::cout << "Application is online: " << status << std::endl;
        std::cout << "Open sockets: " << _this->openClientsSockets.size() << std::endl;
    }
    
}

void ProxyFE::monitorKeepAlives() {
    pthread_create(&monitor_tid, NULL, monitorKeepAlivesAux, (void*) this);
}

    