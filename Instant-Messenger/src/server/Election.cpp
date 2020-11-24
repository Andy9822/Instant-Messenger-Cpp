#include "../../include/server/Election.hpp"



namespace election 
{
	Election::Election()
	{
		isParticipating = false;
	}



	void Election::startElection(int server_ID, int clientSocket)
	{
		sendMessageToNextServer(server_ID, clientSocket);
	}



	void Election::sendMessageToNextServer(int server_ID, int sockfd, string status)
	{
		Packet *sendingPacket = new Packet();

		// Prepare Packet struct to be sent
		*sendingPacket = buildPacket(server_ID, status);

		//Send Packet struct via TCP socket
		sendPacket(sockfd, sendingPacket);
	}


	// status == "ELECTED" || status == "ELECTION"
	Packet Election::buildPacket(int server_ID, string status)
	{
		char messageBuffer[MESSAGE_MAX_SIZE] = {0};

		string message = status + " " + to_string(server_ID);

		strncpy(messageBuffer, message.c_str(), MESSAGE_MAX_SIZE - 2); //Send message with maximum of 255 characters
		
		// Adjust last character to end of string in case string was bigger than max size
		messageBuffer[MESSAGE_MAX_SIZE - 1] = '\0';

		return Packet((char*)"", (char*)"", messageBuffer, time(0));
	}




	int Election::processElectionInfo(string Receivedmessage, int server_ID, int sockfd)
	{
		istringstream message(Receivedmessage);
    	string status, receivedID;
    	int client_ID;

    	message >> status;
    	message >> receivedID;

    	client_ID = stoi(receivedID);

    	if(!isParticipating)
    	{
    		if(client_ID > server_ID)
    			sendMessageToNextServer(client_ID, sockfd);
    		else
    			sendMessageToNextServer(server_ID, sockfd);

    		isParticipating = true;
    	}
    	else
    	{
    		if(status == "ELECTION")
    		{
				cout << "❌PRIMARY SERVER DOWN ❌" << endl;
				cout << "💍STARTING NEW ELECTION 🗳" << endl;
    			if(client_ID != server_ID)
    				sendMessageToNextServer(client_ID, sockfd);
    			else
    				sendMessageToNextServer(client_ID, sockfd, "ELECTED");
    		}
    		else
    		{
    			if(client_ID != server_ID)
    				sendMessageToNextServer(client_ID, sockfd, "ELECTED");

    			isParticipating = false;

    			// returns the new primary id
    			return client_ID;
    		}
    	}

    	return 0;
	}




}