CC=g++
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src


all: $(BIN_DIR)/server.o $(BIN_DIR)/client.o
	${CC} -o client $(BIN_DIR)/client.o
	${CC} -pthread -o server $(BIN_DIR)/server.o
	
$(BIN_DIR)/client.o: $(SRC_DIR)/client.cpp
	$(CC) -c $(SRC_DIR)/client.cpp -Wall -Wextra
	mv client.o $(BIN_DIR)

$(BIN_DIR)/server.o: $(SRC_DIR)/server.cpp
	$(CC) -c $(SRC_DIR)/server.cpp -Wall -Wextra
	mv server.o $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ client server *~ 