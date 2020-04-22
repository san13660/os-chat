// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <pthread.h>

#include <iostream>
#include "mensaje.pb.h"

using namespace std;
using namespace chat;

#define PORT 8080
#define MAX_CLIENTS 99

int clients_sockets[MAX_CLIENTS] = {0};
string clients_names[MAX_CLIENTS];
string clients_status[MAX_CLIENTS];

void *listenClient(void *client_index_p_)
{
	int *client_index_p = (int *) client_index_p_;
	int client_index = *client_index_p;
	int socket = clients_sockets[client_index];
	int valread;
	while(1){
		char buffer[1024] = {0}; 
		valread = read( socket , buffer, 1024); 

		if(valread == 0)
		{
			printf("CLIENT %d DISCONECTED\n", socket);
			break;
		}

		string str(buffer);

		ClientMessage clientMessage;
		clientMessage.ParseFromString(str);

		// Se puede accesar a los valores de la siguiente manera:
		//cout << "Option: " << m2.option() << endl;
		//cout << "Message: " << m2.broadcast().message() << endl;

		if(clientMessage.option() == 1)
		{

			clients_names[client_index] = clientMessage.synchronize().username();

			cout << "Client: " << socket << " username: " << clients_names[client_index] << endl;

			MyInfoResponse *miInfoResponse(new MyInfoResponse);
			miInfoResponse->set_userid(socket);
	
			ServerMessage sm;
			sm.set_option(4);
			sm.set_allocated_myinforesponse(miInfoResponse);

			string binary;
			sm.SerializeToString(&binary);

			char cstr[binary.size() + 1];
			strcpy(cstr, binary.c_str());

			send(socket , cstr, strlen(cstr) , 0 ); 	
		}
		else if(clientMessage.option() == 2)
		{
			ConnectedUserResponse *connectedUserResponse(new ConnectedUserResponse);
			if(clientMessage.connectedusers().userid() == 0){
				for(int i=0;i<MAX_CLIENTS;i++){
					if(clients_sockets[i] != 0){
						ConnectedUser *connectedUser = connectedUserResponse->add_connectedusers();
						connectedUser->set_username(clients_names[i]);
						connectedUser->set_status(clients_status[i]);
						connectedUser->set_userid(clients_sockets[i]);
						connectedUser->set_ip("192");
					}
				}
			}
			else
			{
				for(int i=0;i<MAX_CLIENTS;i++){
					if(clients_sockets[i] != 0 && clients_names[i] == clientMessage.connectedusers().username()){
						ConnectedUser *connectedUser = connectedUserResponse->add_connectedusers();
						connectedUser->set_username(clients_names[i]);
						connectedUser->set_status(clients_status[i]);
						connectedUser->set_userid(clients_sockets[i]);
						connectedUser->set_ip("192");
						break;
					}
				}
			}
			ServerMessage sm;
			sm.set_option(5);
			sm.set_allocated_connecteduserresponse(connectedUserResponse);

			string binary;
			sm.SerializeToString(&binary);

			char cstr[binary.size() + 1];
			strcpy(cstr, binary.c_str());

			send(socket , cstr, strlen(cstr) , 0 );
		}
		else if(clientMessage.option() == 3)
		{
			clients_status[client_index] = clientMessage.changestatus().status();
			cout << "Client " << socket << " changed status to: " << clients_status[client_index] << endl;

			ChangeStatusResponse *changeStatusResponse(new ChangeStatusResponse);
			changeStatusResponse->set_userid(socket);
			changeStatusResponse->set_status(clients_status[client_index]);
	
			ServerMessage sm;
			sm.set_option(6);
			sm.set_allocated_changestatusresponse(changeStatusResponse);

			string binary;
			sm.SerializeToString(&binary);

			char cstr[binary.size() + 1];
			strcpy(cstr, binary.c_str());

			send(socket , cstr, strlen(cstr) , 0 ); 	
		}
		else if(clientMessage.option() == 4)
		{
			cout << "Client " << socket << " broadcasted: " << clientMessage.broadcast().message() << endl;
			
			BroadcastMessage *broadcastMessage(new BroadcastMessage);
			broadcastMessage->set_message(clientMessage.broadcast().message());
			broadcastMessage->set_userid(socket);
			broadcastMessage->set_username(clients_names[client_index]);
	
			ServerMessage sm;
			sm.set_option(1);
			sm.set_allocated_broadcast(broadcastMessage);

			string binary;
			sm.SerializeToString(&binary);

			char cstr[binary.size() + 1];
			strcpy(cstr, binary.c_str());	
			
			for(int i=0;i<MAX_CLIENTS;i++){
				if(clients_sockets[i] != 0 && i != client_index){
					send(clients_sockets[i] , cstr , strlen(cstr) , 0);
				}
			}
		}
		else if(clientMessage.option() == 5)
		{
			cout << "Client " << socket << " sent direct message to " << clientMessage.directmessage().username() << endl;
			
			DirectMessage *directMessage(new DirectMessage);
			directMessage->set_message(clientMessage.directmessage().message());
			directMessage->set_userid(socket);
			directMessage->set_username(clients_names[client_index]);
	
			ServerMessage sm;
			sm.set_option(2);
			sm.set_allocated_message(directMessage);

			string binary;
			sm.SerializeToString(&binary);

			char cstr[binary.size() + 1];
			strcpy(cstr, binary.c_str());	

			for(int i=0;i<MAX_CLIENTS;i++){
				if(clients_sockets[i] != 0 && clients_names[i] == clientMessage.directmessage().username()){
					send(clients_sockets[i] , cstr , strlen(cstr) , 0);
					break;
				}
			}
		}
		else
		{
			cout << "[UNKNOWN CLIENT MESAGE][" << clientMessage.option() << "]" << endl;
		}
	}

	clients_sockets[client_index] = 0;
	clients_names[client_index] = "";
	clients_status[client_index] = "";
	close(socket);
	free(client_index_p);
	return NULL;
}

int main(int argc, char const *argv[]) 
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	int server_fd; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}
	
	cout << "SERVER STARTED" << endl;
	
	while(1){

		int new_socket;
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		
		int *index= (int *)malloc(sizeof(int));
		for(int i=0;i<MAX_CLIENTS;i++){
			if(clients_sockets[i] == 0){
				clients_sockets[i] = new_socket;
				clients_names[i] = "Anonimo";
				clients_status[i] = "ACTIVO";
				*index = i;
				break;
			}
		}


		printf("CLIENT %d CONECTED\n", new_socket);
		pthread_t thread;
		pthread_create(&thread, NULL, &listenClient, index);
	}
	
	google::protobuf::ShutdownProtobufLibrary();
	return 0; 
} 

