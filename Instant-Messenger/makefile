CC=g++
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src


all: 
	${CC} -o client $(SRC_DIR)/app_client.cpp $(SRC_DIR)/client.cpp
	${CC} -pthread -o server $(SRC_DIR)/app_server.cpp $(SRC_DIR)/server.cpp
	
#$(BIN_DIR)/app_client.o: $(SRC_DIR)/app_client.cpp $(SRC_DIR)/client.cpp
#	$(CC) -c $(SRC_DIR)/app_client.cpp $(SRC_DIR)/client.cpp -Wall -Wextra
#	mv app_client.o $(BIN_DIR)
#	mv client.o $(BIN_DIR)

#$(BIN_DIR)/app_server.o: $(SRC_DIR)/app_server.cpp $(SRC_DIR)/server.cpp
#	$(CC) -c $(SRC_DIR)/app_server.cpp $(SRC_DIR)/server.cpp -Wall -Wextra
#	mv app_server.o $(BIN_DIR)

clean:
	rm -rf $(SRC_DIR)/*~ $(INC_DIR)/*~ client server *~ 