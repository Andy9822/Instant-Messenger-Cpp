CC=g++
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
BUILD_DIR=./build
SERVER_PORT=8888
FE_SERVER_PORT=6969
FE_CLIENTS_PORT=4040
RM_NUMBER = 3
IS_PRIMARY = 1
NUM_MAX_MESSAGES = 4

all: build-server build-client build-proxy_fe

build-client:	Socket.o Semaphore.o Uuid.o ConnectionKeeper.o
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/client.app Socket.o Semaphore.o Uuid.o ConnectionKeeper.o $(SRC_DIR)/app/app_client.cpp $(SRC_DIR)/client/client.cpp

build-server:	Socket.o Semaphore.o Uuid.o ConnectionKeeper.o
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/server.app Socket.o Uuid.o $(SRC_DIR)/util/message.cpp $(SRC_DIR)/util/file_system_manager.cpp Semaphore.o $(SRC_DIR)/app/app_server.cpp $(SRC_DIR)/server/server.cpp \
	$(SRC_DIR)/server/server_group_manager.cpp $(SRC_DIR)/server/server_message_manager.cpp $(SRC_DIR)/util/user.cpp $(SRC_DIR)/server/Group.cpp $(SRC_DIR)/util/ConnectionMonitor.cpp ConnectionKeeper.o $(SRC_DIR)/util/StringConstants.cpp $(SRC_DIR)/server/FeAddressBook.cpp \
	$(SRC_DIR)/server/ServersRing.cpp $(SRC_DIR)/server/Election.cpp

build-proxy_fe:	Socket.o Semaphore.o Uuid.o ConnectionKeeper.o
	${CC} -pthread -std=c++11 -g -o $(BUILD_DIR)/proxy_fe.app Socket.o Uuid.o Semaphore.o $(SRC_DIR)/app/app_proxy_fe.cpp \
	$(SRC_DIR)/util/ConnectionMonitor.cpp ConnectionKeeper.o $(SRC_DIR)/proxy_fe/ProxyFE.cpp

Socket.o:	 $(SRC_DIR)/util/Socket.cpp
	${CC} -c $(SRC_DIR)/util/Socket.cpp

Semaphore.o:	$(SRC_DIR)/util/Semaphore.cpp
	${CC} -c $(SRC_DIR)/util/Semaphore.cpp

ConnectionKeeper.o:	$(SRC_DIR)/util/ConnectionKeeper.cpp
	${CC} -c $(SRC_DIR)/util/ConnectionKeeper.cpp

Uuid.o:	$(SRC_DIR)/util/Uuid.cpp
	${CC} -c $(SRC_DIR)/util/Uuid.cpp


# Targets for automated execution
run-client:
	$(BUILD_DIR)/client.app JohnnyUser1 SampleRoom1

run-server: clean build-server
	$(BUILD_DIR)/server.app $(NUM_MAX_MESSAGES) 3

run-proxy_fe:
	$(BUILD_DIR)/proxy_fe.app $(FE_SERVER_PORT) $(FE_CLIENTS_PORT)

# Targets for automated deploy
deploy-server-principal: 
	make -s clean 
	make -s build-server
	$(BUILD_DIR)/server.app $(NUM_MAX_MESSAGES) 3
	
deploy-server-backup3: 
	make -s clean 
	make -s build-server
	$(BUILD_DIR)/server.app $(NUM_MAX_MESSAGES) 2

deploy-server-backup2: 
	make -s clean 
	make -s build-server
	$(BUILD_DIR)/server.app $(NUM_MAX_MESSAGES) 1

deploy-server-backup1: 
	make -s clean 
	make -s build-server
	$(BUILD_DIR)/server.app $(NUM_MAX_MESSAGES) 0

deploy_fe1: 
	make -s clean 
	make -s build-proxy_fe
	$(BUILD_DIR)/proxy_fe.app 6969 4039

deploy_fe2: 
	make -s clean 
	make -s build-proxy_fe
	$(BUILD_DIR)/proxy_fe.app 6970 4040

deploy_fe3: 
	make -s clean 
	make -s build-proxy_fe
	$(BUILD_DIR)/proxy_fe.app 6971 4041


# Targets for managing docker deploy and logs
deploy: services-down
	docker-compose up -d 
	
deploy_logs: services-down
	docker-compose up
	
services-up:
	docker-compose up -d

services-down:
	docker-compose down

logs_fe:
	docker-compose logs -f fe1 fe2 fe3

logs_rm:
	docker-compose logs -f rm_primary rm_3 rm_2 rm_1

clean:
	- rm -rf $(SRC_DIR)/*~ $(INC_DIR)/*~ client server *~
	- rm -f *.o
	- rm -f $(BUILD_DIR)/*.app
