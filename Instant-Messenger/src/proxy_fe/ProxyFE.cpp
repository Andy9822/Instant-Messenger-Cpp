#include "../../include/proxy_fe/ProxyFE.hpp"
#include "../../include/util/ConnectionKeeper.hpp"
#include "../../include/util/Packet.hpp"

std::map<int, std::pair<pthread_t, time_t>> openClientsSockets;
Semaphore* ProxyFE::online_semaphore;
Semaphore* ProxyFE::users_map_semaphore;
Semaphore* ProxyFE::sockets_map_semaphore;
bool ProxyFE::online_RMserver;
int ProxyFE::serverRM_socket;

////////////////////// Setup sockets and connection methods ////////////////////// 

ProxyFE::ProxyFE() {
    // Configure serverRM address properties
    serv_sock_addr.sin_family = AF_INET;
    serv_sock_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_sock_addr.sin_zero), 8);

    // Configure clients address properties
    client_sock_addr.sin_family = AF_INET;
    client_sock_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(client_sock_addr.sin_zero), 8);

    // Initialize socket file descriptor
    connections_server_socket_fd = 0;
    clients_socket_fd = 0;

    // Init semaphore for processing message semaphore
    processing_message_semaphore = new Semaphore(1);

    // Init semaphore for online_RMserver and set variable to false
    online_semaphore = new Semaphore(1);
    online_RMserver = false;

    // Init semaphore for users map and sockets map
    users_map_semaphore = new Semaphore(1);
    sockets_map_semaphore = new Semaphore(1);

    // Init message consumer mutex
    pthread_mutex_init(&mutex_consumer_message, NULL);
    pthread_mutex_lock(&mutex_consumer_message);

    // Init server reconnect mutex locked as initially it's everything connected and fine
    pthread_mutex_init(&mutex_server_reconnect, NULL);
    pthread_mutex_lock(&mutex_server_reconnect);

    // Init connections keep alives monitor
    this->keepAliveMonitor = new ConnectionMonitor();

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
    prepareSocketConnection(&connections_server_socket_fd, &serv_sock_addr);
    std::cout << "Prepared server socket_fd: " << connections_server_socket_fd << " ✅" << std::endl;
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

    if (getsockname(connections_server_socket_fd, (struct sockaddr *) &serv_sock_addr, &len_srv) < 0) {
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
////////////////////// End of setup methods //////////////////////

////////////////////////// SERVER METHODS //////////////////////////
void ProxyFE::processServerPacket(Packet* receivedPacket, int socket)
{
    switch (receivedPacket->type)
    {
    case MESSAGE_PACKET:
    {
        std::cout << "📮Recebi Message do server📨📩: " << receivedPacket->message << std::endl;
        std::cout << "user_id: " << receivedPacket->user_id << std::endl;
        string userID(receivedPacket->user_id); 
        users_map_semaphore->wait();
        int clientSocket = usersMap[userID];
        users_map_semaphore->post();       
        if(clientSocket != 0)
        {
            sendPacket(clientSocket, receivedPacket); // TODO Socket agora dá shutdown quando dá erro, comprovar todos os pontos que tem sendPacket 
            break;
        }
    }
        break;
    case KEEP_ALIVE_PACKET:
        std::cout << "recebi Keep Alive do server 🖧" << std::endl;
        keepAliveMonitor->refresh(socket);
        break;
    
    case ACCEPT_PACKET:
    {
        std::cout << "🛂Recebi ACCEPT do server🛂🎟🎫: " << receivedPacket->username << " pode dar join na sala" << std::endl;
        std::cout << "user_id: " << receivedPacket->user_id << std::endl;
        string userID(receivedPacket->user_id);
        users_map_semaphore->wait();
        int clientSocket = usersMap[userID];
        users_map_semaphore->post();
        if(clientSocket != 0)
        {
            std::cout << "vou mandar ACCEPT_PACKET pro usuario " << clientSocket << std::endl;
            sendPacket(clientSocket, new Packet(ACCEPT_PACKET)); 
            break;
        }

        std::cout << "O usuário não está mais conectado" << std::endl;
        break;
    }
    
    case CONNECTION_REFUSED_PACKET:
    {
        std::cout << "⛔Recebi PROBIÇÃO do server🚫:  NÃO pode dar join na sala" << std::endl;
        std::cout << "user_id: " << receivedPacket->user_id << std::endl;
        string userID(receivedPacket->user_id);
        users_map_semaphore->wait();
        int clientSocket = usersMap[userID];
        users_map_semaphore->post();
        if(clientSocket != 0)
        {
            std::cout << "vou mandar CONNECTION_REFUSED_PACKET pro usuario " << clientSocket << std::endl;
            sendPacket(clientSocket, new Packet(CONNECTION_REFUSED_PACKET));
            break;
        }

        std::cout << "O usuário não está mais conectado" << std::endl;
        break;
    }
    
    case ACK_PACKET:
    {
        std::cout << "📫Recebi ACK do server📫: " << std::endl;
        std::cout << "user_id: " << receivedPacket->user_id << std::endl;
        string userID(receivedPacket->user_id);
        users_map_semaphore->wait();
        int clientSocket = usersMap[userID];
        users_map_semaphore->post();
        if(clientSocket != 0)
        {
            std::cout << "vou mandar ACK pro usuario " << clientSocket << std::endl;
            sendPacket(clientSocket, new Packet(ACK_PACKET));
            break;
        }

        std::cout << "O usuário não está mais conectado" << std::endl;
        break;
    }

    default:
        std::cout << "Undefined Packet type received" << std::endl;
        break;
    }
}

void* ProxyFE::listenServerCommunication(void *args) 
{
    // We cast our receveid void* args to a pair*
    std::pair<int, ProxyFE *> *args_pair = (std::pair<int, ProxyFE *>*) args;

    // Get a reference of the object instance in this thread
    ProxyFE *_this = (ProxyFE *) args_pair->second;

    // Get socket from the new connected client
    _this->serverRM_socket = args_pair->first;

    // Free pair created for receiving arguments
    free(args_pair);
    
    // Send keep alives to serverRM
    ConnectionKeeper(_this->serverRM_socket); //TODO NAO MEXER NESSA LINHA SENÃO PODE QUEBRAR TUDO

    // Listen for incoming Packets from server until it disconnects
    bool is_server_connected = true;
    while (is_server_connected) {
        Packet* receivedPacket = _this->readPacket(_this->serverRM_socket, &is_server_connected);
        if (!is_server_connected) {
            // Free allocated memory for reading Packet
            free(receivedPacket);
            break;
        }
        _this->processServerPacket(receivedPacket, _this->serverRM_socket);
    }
    
    std::cout << "serverRM socket " << _this->serverRM_socket <<  " has disconnected" << std::endl;
    _this->handleServerDisconnection(_this->serverRM_socket);
    
    return NULL;
}

int ProxyFE::handleServerConnection(pthread_t *tid) {
    int status;

    // Allocate memory space to store value in heap and be able to use it after this function ends
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    if ((newsockfd = accept(connections_server_socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        cout << "ERROR on accept\n" << endl;
        pthread_mutex_unlock(&mutex_server_reconnect);
        return -1;
    }

    std::cout << "Primary RM server connected with socket: " << newsockfd << std::endl;
    
    // Save server RM socket to openSockets map
    serverRM_socket = newsockfd;

    //Create a pair for sending more than 1 parameter to the new thread that we are about to create for listening client Packets
    std::pair<int, ProxyFE*>* args = (std::pair<int, ProxyFE*>*) calloc(1, sizeof(std::pair<int, ProxyFE*>));
    
    args->second = this; // Send reference of this instance to the new thread
    args->first = serverRM_socket;

    pthread_create(tid, NULL, listenServerCommunication, (void *) args);

    //Create another pair for sending more than 1 parameter to the new thread to monitor keep alives from this connection
    std::pair<int, ProxyFE*>* args2 = (std::pair<int, ProxyFE*>*) calloc(1, sizeof(std::pair<int, ProxyFE*>));
    
    args2->second = this; // Send reference of this instance to the new thread
    args2->first = serverRM_socket;

    //TODO ver esses tid totalmente errados
    pthread_create(tid, NULL, monitorConnectionKeepAlive, (void *) args2);
    
    // Set application as online
    online_semaphore->wait();
    online_RMserver = true;
    online_semaphore->post();

    std::cout << "✅ ServerRM connected - System is Online ✅" << std::endl;

    return 0; 
}

void* ProxyFE::listenServerReconnect(void* args)
{
    // We cast our receveid void* args to a pair*
    std::pair<pthread_t*, ProxyFE *> *args_pair = (std::pair<pthread_t*, ProxyFE *>*) args;

    // Get tid 
    pthread_t* tid = args_pair->first;

    // Get a reference of the object instance in this thread
    ProxyFE *_this = (ProxyFE *) args_pair->second;

    // Free pair created for receiving arguments
    free(args_pair);
    
    while (true)
    {
        pthread_mutex_lock(&(_this->mutex_server_reconnect));
        std::cout << "Waiting for ServerRM reconnection..." << std::endl;
        _this->handleServerConnection(tid);
    }

    return NULL;
}

void ProxyFE::handleServerReconnect(pthread_t *tid) 
{
    //Create a pair for sending more than 1 parameter to the new thread that we are about to create for listening client Packets
    std::pair<pthread_t*, ProxyFE*>* args = (std::pair<pthread_t*, ProxyFE*>*) calloc(1, sizeof(std::pair<pthread_t*, ProxyFE*>));
    
    args->first = tid;
    args->second = this; // Send reference of this instance to the new thread
    
    pthread_create(&reconnect_server_tid, NULL, listenServerReconnect, (void*) args);
}
////////////////////////// End of server methods //////////////////////////

////////////////////////// Client methods //////////////////////////
int ProxyFE::handleClientConnection(pthread_t *tid) 
{
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    if ((newsockfd = accept(clients_socket_fd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        cout << "ERROR on accept\n" << endl;
        return -1;
    }


    //Create a pair for sending more than 1 parameter to the new thread that we are about to create for listening client Packets
    std::pair<int, ProxyFE*>* args = (std::pair<int, ProxyFE*>*) calloc(1, sizeof(std::pair<int, ProxyFE*>));
    
    args->second = this; // Send reference of this instance to the new thread
    args->first = newsockfd;

    pthread_create(tid, NULL, listenClientCommunication, (void *) args);

    //Create another pair for sending more than 1 parameter to the new thread to monitor keep alives from this connection
    std::pair<int, ProxyFE*>* args2 = (std::pair<int, ProxyFE*>*) calloc(1, sizeof(std::pair<int, ProxyFE*>));
    
    args2->second = this; // Send reference of this instance to the new thread
    args2->first = newsockfd;

    pthread_create(tid, NULL, monitorConnectionKeepAlive, (void *) args2);

    return 0;
}

void* ProxyFE::monitorConnectionKeepAlive(void *args) 
{
    // We cast our receveid void* args to a pair*
    std::pair<int, ProxyFE *> *args_pair = (std::pair<int, ProxyFE *>*) args;

    // Get socket from the new connected client
    int client_socketfd = args_pair->first;

    // Get a reference of the object instance in this thread
    ProxyFE *_this = (ProxyFE *) args_pair->second;

    // Free pair created for receiving arguments
    free(args_pair);

    //We call this method that will keep running until keep alive timeouts
    _this->keepAliveMonitor->monitor(&client_socketfd);

    // After client socket disconnection we need to process this disconnection in application level
    std::cout << "Timed out socket " << client_socketfd << std::endl;

    shutdown(client_socketfd, 2);

    return NULL;
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

    char userID[UUID_SIZE];
    bool known_user_ID = false;

    // Listen for incoming Packets from client untill it disconnects
    bool is_client_connected = true;
    while (is_client_connected) {
        Packet* receivedPacket = _this->readPacket(client_socketfd, &is_client_connected);
        if (!is_client_connected) {
            break;
        }

        if (!known_user_ID)
        {
            strcpy(userID, receivedPacket->user_id);
            known_user_ID = true;
        }
        
        _this->processClientPacket(receivedPacket, client_socketfd);

    }

    if (known_user_ID)
    {
        _this->disconnectUser(userID);
    }
    std::cout << "Client " << userID << " socket " << client_socketfd <<  " has disconnected" << std::endl;
    _this->handleSocketDisconnection(client_socketfd);
    
    return NULL;
}

void ProxyFE::disconnectUser(char* userID)
{
    Packet* disconnect_packet = new Packet(DISCONNECT_PACKET, userID);

    std::cout << "Vou mandar server DISCONNECT_PACKET do userID: " << userID << std::endl;

    sendToServer(disconnect_packet);
}

void ProxyFE::registerUserSocket(Packet* receivedPacket, int socket)
{
    string userID(receivedPacket->user_id); 

    users_map_semaphore->wait();
    usersMap[userID] = socket;
    users_map_semaphore->post();

    sockets_map_semaphore->wait();
    socketsMap[socket] = userID;
    sockets_map_semaphore->post();

    std::cout << "🆔id: " << userID << " socket:" << usersMap[userID] << std::endl;
    std::cout << "👦users map size:" << usersMap.size() << std::endl;

    std::cout << "💻socket: " << socket << " user:" << socketsMap[socket] << std::endl;
    std::cout << "💾sockets map size:" << socketsMap.size() << std::endl;

    receivedPacket->clientSocket = socket;
    sendToServer(receivedPacket);
}

void ProxyFE::processClientPacket(Packet* receivedPacket, int socket)
{
    switch (receivedPacket->type)
    {
    case MESSAGE_PACKET:
        processIncomingClientMessage(receivedPacket);
        break;
    
    case KEEP_ALIVE_PACKET:
        std::cout << "recebi Keep Alive do cliente 🙇" << std::endl;
        keepAliveMonitor->refresh(socket);
        break;
    
    case JOIN_PACKET:
        registerUserSocket(receivedPacket, socket);
        break;
    
    default:
        std::cout << "Undefined Packet type received" << std::endl;
        break;
    }
}

void ProxyFE::handleSocketDisconnection(int socket)
{
    keepAliveMonitor->killSocket(socket);

    sockets_map_semaphore->wait();
    string userID = socketsMap[socket];
    socketsMap.erase(socket);
    sockets_map_semaphore->post();

    users_map_semaphore->wait();
    usersMap.erase(userID);
    users_map_semaphore->post();

    close(socket);
}

void ProxyFE::handleServerDisconnection(int socket)
{
    handleSocketDisconnection(socket);
    online_semaphore->wait();
    online_RMserver = false;
    online_semaphore->post();
    pthread_mutex_unlock(&mutex_server_reconnect);
    std::cout << "❌❗❌ ServerRM went down - System is Offline ❌❗❌" << std::endl;
}
    

////////////////////////// Fe Forwarding methods //////////////////////////
void ProxyFE::activateMessageConsumer(pthread_t* tid) 
{
    pthread_create(tid, NULL, handleProcessingMessage, (void *) this);
}

void* ProxyFE::handleProcessingMessage(void* args) 
{
    // Get a reference of the object instance in this thread
    ProxyFE *_this = (ProxyFE *) args;
    int success;
    while (true)
    {
        pthread_mutex_lock(&(_this->mutex_consumer_message));
        std::cout << "recebi mssagem cliente e to handleProcessingMessage" << std::endl;
        
        _this->sendToServer(_this->processing_message);
        
        _this->processing_message_semaphore->post();
    }  
}


void ProxyFE::sendToServer(Packet* packet)
{
    bool hasFailed = false;
retry:
    int status = sendPacket(this->serverRM_socket, packet);

    if (status == ERROR_SENDING)
    {
        std::cout << "❌ Erro enviando, sevidorRM está off ❌" << std::endl;
        hasFailed = true;
        sleep(1);
        goto retry;
    }
    if (hasFailed)
    {
        std::cout << "✅ Novo servidor RM conectou e recebeu a mensagem ✅" << std::endl;
    }
}

void ProxyFE::processIncomingClientMessage(Packet* message)
{
    std::cout << "📮Recebi Message do usuario 👦: " << message->message << std::endl;
    std::cout << "message->user_id" << message->user_id << std::endl;
    std::cout << "message->message_id" << message->message_id << std::endl;
    processing_message_semaphore->wait();
    processing_message = message;
    pthread_mutex_unlock(&mutex_consumer_message);
} 