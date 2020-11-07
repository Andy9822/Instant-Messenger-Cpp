CC=g++
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
BUILD_DIR=./build
SERVER_PORT=8888
FE_SERVER_PORT=6969
FE_CLIENTS_PORT=4040

all: build-server build-client

build-client:
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/client.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/util/ConnectionKeeper.cpp $(SRC_DIR)/app/app_client.cpp $(SRC_DIR)/client/*.cpp

build-server:
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/server.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/util/message.cpp $(SRC_DIR)/util/file_system_manager.cpp $(SRC_DIR)/util/Semaphore.cpp $(SRC_DIR)/app/app_server.cpp $(SRC_DIR)/server/server.cpp \
	$(SRC_DIR)/server/server_group_manager.cpp $(SRC_DIR)/server/server_message_manager.cpp $(SRC_DIR)/util/user.cpp $(SRC_DIR)/server/Group.cpp $(SRC_DIR)/util/ConnectionMonitor.cpp $(SRC_DIR)/util/ConnectionKeeper.cpp

build-proxy_fe:
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/proxy_fe.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/util/Semaphore.cpp $(SRC_DIR)/app/app_proxy_fe.cpp \
	$(SRC_DIR)/util/ConnectionMonitor.cpp $(SRC_DIR)/util/ConnectionKeeper.cpp $(SRC_DIR)/proxy_fe/ProxyFE.cpp 

run-client:
	$(BUILD_DIR)/client.app JohnnyUser1 SampleRoom1 127.0.0.1 $(FE_CLIENTS_PORT)

run-server:
	$(BUILD_DIR)/server.app $(SERVER_PORT) 4

run-proxy_fe:
	$(BUILD_DIR)/proxy_fe.app $(FE_SERVER_PORT) $(FE_CLIENTS_PORT)

run-rm:
	$(BUILD_DIR)/client.app JohnnyUser1 SampleRoom1 127.0.0.1 $(FE_SERVER_PORT)

clean:
	rm -rf $(SRC_DIR)/*~ $(INC_DIR)/*~ client server *~
	rm $(BUILD_DIR)/*.app
