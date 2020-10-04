CC=g++
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
BUILD_DIR=./build


all: build-server build-client

build-client:
	${CC} -std=c++11 -g -o $(BUILD_DIR)/client.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/app/app_client.cpp $(SRC_DIR)/client/*.cpp

build-server:
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/server.app $(SRC_DIR)/util/Socket.cpp $(SRC_DIR)/util/message.cpp $(SRC_DIR)/util/file_system_manager.cpp $(SRC_DIR)/util/Semaphore.cpp $(SRC_DIR)/app/app_server.cpp $(SRC_DIR)/server/server.cpp \
	$(SRC_DIR)/server/server_group_manager.cpp $(SRC_DIR)/server/server_message_manager.cpp $(SRC_DIR)/util/user.cpp

run-client:
	$(BUILD_DIR)/client.app JohnnyUser2 SampleRoom2 127.0.0.1 4040

run-server:
	$(BUILD_DIR)/server.app 4040

#$(BIN_DIR)/app_client.o: $(SRC_DIR)/app_client.cpp $(SRC_DIR)/client.cpp
#	$(CC) -c $(SRC_DIR)/app_client.cpp $(SRC_DIR)/client.cpp -Wall -Wextra
#	mv app_client.o $(BIN_DIR)
#	mv client.o $(BIN_DIR)

#$(BIN_DIR)/app_server.o: $(SRC_DIR)/app_server.cpp $(SRC_DIR)/server.cpp
#	$(CC) -c $(SRC_DIR)/app_server.cpp $(SRC_DIR)/server.cpp -Wall -Wextra
#	mv app_server.o $(BIN_DIR)

clean:
	rm -rf $(SRC_DIR)/*~ $(INC_DIR)/*~ client server *~
	rm $(BUILD_DIR)/*.app
