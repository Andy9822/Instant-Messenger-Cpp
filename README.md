# Instant-Messenger-Cpp 💬
The purpose of this project is to implement a messenger application between users in a low-level implementation using TCP API from UNIX sockets.<br />
The system is completely distributed therefore it has fault tolerance and data replication.

## Messenger Application 👨‍👩‍👦‍👦
The system consists in a simple messenger application with users and groups.<br />
A client specifies its username and the room that wants to connect. If the same username is previously connected in 2 other devices, in the same room or different it does'nt matter, his new connection wil be denied. <br />
The rooms have a message history so if a user connects to an existent room he will be able to see the last 10 sent messages.

## Fault Tolerance 🔌
All servers are connected with each other and listen for keep alives. If they detect that the primary server, or the leader, is down they'll start a new election using the ring alorithm and elect the next server that will assume the lead.<br />
During this time, the users remain connected to the room and can send messages but they will be put in a buffer and delivered once the application gets online again.<br />
This process is completely transparent and imperceptible for clients.

## Data Replication 🗳
As all servers are connected, when the primary server receives a message it replicates it to all the backup servers (passive replication). Thanks to it, the exactly same data is shared and synced between all the servers.<br />
If the primary server falls, the new elected server will have the same data and will carry on from the same point.

## Run the project 🎡

The entire project was Dockerized to easily start the application and deletage to docker-compose all the responsability to orchestrate the containers.
To run and test the project run the following commands:

### Start the system:<br />
   run `make  deploy` <br />
This will start all the containers in the correct order and turn the application online

### Start system with docker-compose logs :<br />
   run `make  deploy-logs` <br />
This will start all the containers and also show the docker-composer logs of all of them

### Execute client:<br />
   run `make build-client`<br />
   run `./build/client.app <username> <room>`<br />
This will run the client and connect to the specified room with the username you have passed.

## Author 🧙‍♂️
- Andy Ruiz Garramones - [Andy9822](https://github.com/Andy9822)
- Gabriel Stepien - [gabriel-inf](https://github.com/gabriel-inf)
- Cassiano Jaeger Stradolini - [gabriel-inf](https://github.com/cassianojaeger)
- GuilhermeChaves - [gabriel-inf](https://github.com/GuilhermeChaves)