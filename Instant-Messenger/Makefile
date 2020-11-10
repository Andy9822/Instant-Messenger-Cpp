CC=g++
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
BUILD_DIR=./build
SERVER_PORT=4040

all: build-server build-client

build-client:
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/client.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/util/Uuid.cpp $(SRC_DIR)/app/app_client.cpp $(SRC_DIR)/client/*.cpp

build-server:
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/server.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/util/Uuid.cpp $(SRC_DIR)/util/message.cpp $(SRC_DIR)/util/file_system_manager.cpp $(SRC_DIR)/util/Semaphore.cpp $(SRC_DIR)/app/app_server.cpp $(SRC_DIR)/server/server.cpp \
	$(SRC_DIR)/server/server_group_manager.cpp $(SRC_DIR)/server/server_message_manager.cpp $(SRC_DIR)/util/user.cpp $(SRC_DIR)/server/Group.cpp $(SRC_DIR)/server/ConnectionMonitor.cpp $(SRC_DIR)/client/ConnectionKeeper.cpp

run-client:
	$(BUILD_DIR)/client.app JohnnyUser1 SampleRoom1 127.0.0.1 $(SERVER_PORT)

run-server:
	$(BUILD_DIR)/server.app $(SERVER_PORT) 4 1

clean:
	rm -rf $(SRC_DIR)/*~ $(INC_DIR)/*~ client server *~
	rm $(BUILD_DIR)/*.app
