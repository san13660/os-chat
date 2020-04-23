// Universidad del Valle de Guatemala
// Sistemas Operativos - Seccion 10
// 22/04/2020
// Proyecto CHAT (lado del sevrer)
// Christopher Sandoval 13660
// Maria Fernanda Estrada 14198

// Librerias a utilizar
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <pthread.h>
#include <iostream>
#include "mensaje.pb.h"

// Constantes
using namespace std;
using namespace chat;

#define PORT 8080
#define MAX_CLIENTS 99

// Informacion de los clientes
int clients_sockets[MAX_CLIENTS] = {0};
string clients_names[MAX_CLIENTS];
string clients_status[MAX_CLIENTS];
string clients_ips[MAX_CLIENTS];

// Funcion para mantenerse escuchando cualquier solicitud del parte del cliente
void *listenClient(void *client_index_p_)
{
	// Informacion del cliente como socket e index para identificarlo
	int *client_index_p = (int *) client_index_p_;
	int client_index = *client_index_p;
	int socket = clients_sockets[client_index];
	int valread;
	
	while(1){

		char buffer[1024] = {0}; 
		valread = read( socket , buffer, 1024); 

		// Verificar que el cliente siga conectado
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

		// La primera opcion es para sincronizar con el server
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
		
		// La segunda opcion es para enviar la lista de todos los users conectados
		else if(clientMessage.option() == 2)
		{
			ConnectedUserResponse *connectedUserResponse(new ConnectedUserResponse);
			bool found = false;
			// Lista completa
			if(clientMessage.connectedusers().userid() == 0){
				found = true;
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
			// Informacion especifica de un user
			else
			{
				for(int i=0;i<MAX_CLIENTS;i++){
					if(clients_sockets[i] != 0 && clients_names[i] == clientMessage.connectedusers().username()){
						ConnectedUser *connectedUser = connectedUserResponse->add_connectedusers();
						connectedUser->set_username(clients_names[i]);
						connectedUser->set_status(clients_status[i]);
						connectedUser->set_userid(clients_sockets[i]);
						connectedUser->set_ip("192");
						found = true;
						break;
					}
				}
			}

			if(found)
			{
				// ENVIO DE CONFIRMACION
				ServerMessage sm;
				sm.set_option(5);
				sm.set_allocated_connecteduserresponse(connectedUserResponse);

				string binary;
				sm.SerializeToString(&binary);

				char cstr[binary.size() + 1];
				strcpy(cstr, binary.c_str());

				send(socket , cstr, strlen(cstr) , 0 );
			}
			else
			{
				// ENVIO DE ERROR
				ErrorResponse *errorResponse(new ErrorResponse);
				errorResponse->set_errormessage("USUARIO NO ENCONTRADO");
	
				ServerMessage sm;
				sm.set_option(3);
				sm.set_allocated_error(errorResponse);

				string binary;
				sm.SerializeToString(&binary);

				char cstr[binary.size() + 1];
				strcpy(cstr, binary.c_str());
				send(socket , cstr, strlen(cstr) , 0 );
			}
		}
		
		// Opcion para cambiar el estado de un cliente
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
		
		// Opcion de broadcast
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
			
			// ENVIO DE CONFIRMACION
			BroadcastResponse *broadcastResponse(new BroadcastResponse);
			broadcastResponse->set_messagestatus("MENSAJE BROADCAST ENVIADO");
	
			ServerMessage sm2;
			sm2.set_option(7);
			sm2.set_allocated_broadcastresponse(broadcastResponse);

			string binary2;
			sm2.SerializeToString(&binary2);

			char cstr2[binary.size() + 1];
			strcpy(cstr2, binary2.c_str());
			send(socket , cstr2, strlen(cstr2) , 0 ); 
			
		}
		
		// Opcion de mensaje directo
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

			bool found = false;

			for(int i=0;i<MAX_CLIENTS;i++){
				if(clients_sockets[i] != 0 && clients_names[i] == clientMessage.directmessage().username()){
					send(clients_sockets[i] , cstr , strlen(cstr) , 0);
					found = true;
					break;
				}
			}
			

			if(found)
			{
				// ENVIO DE CONFIRMACION
				DirectMessageResponse *directMessageResponse(new DirectMessageResponse);
				directMessageResponse->set_messagestatus("MENSAJE DIRECTO ENVIADO");
	
				ServerMessage sm2;
				sm2.set_option(8);
				sm2.set_allocated_directmessageresponse(directMessageResponse);

				string binary2;
				sm2.SerializeToString(&binary2);

				char cstr2[binary2.size() + 1];
				strcpy(cstr2, binary2.c_str());
				send(socket , cstr2, strlen(cstr2) , 0 );
			}
			else
			{
				// ENVIO DE ERROR
				ErrorResponse *errorResponse(new ErrorResponse);
				errorResponse->set_errormessage("USUARIO NO ENCONTRADO");
	
				ServerMessage sm2;
				sm2.set_option(3);
				sm2.set_allocated_error(errorResponse);

				string binary2;
				sm2.SerializeToString(&binary2);

				char cstr2[binary.size() + 1];
				strcpy(cstr2, binary2.c_str());
				send(socket , cstr2, strlen(cstr2) , 0 );
			}
		}
		
		// Opcion de mensaje directo
		else if(clientMessage.option() == 6)
		{
			// FIN DE HANDSHAKE
		}
		// Si no es otra opcion
		else
		{
			cout << "[MENSAJE DESCONOCIDO][" << clientMessage.option() << "]" << endl;
		}
	}

	// Si se sale del while, debe cerrar todas las listas, vaciar y cerrar sockets
	clients_sockets[client_index] = 0;
	clients_names[client_index] = "";
	clients_status[client_index] = "";
	close(socket);
	free(client_index_p);
	return NULL;
}

// main
int main(int argc, char const *argv[]) 
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	// Variables
	stringstream ssPort(argv[1]);
	ssPort >> portnum;
	int server_fd; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	
	// Crear socket para server
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( portnum ); 
	
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
	
	// Despues de lo anterior, se establece que inicio el server
	cout << "SERVER STARTED" << endl;
	
	while(1){
		// Esperar a que alguien solicite un socket nuevo al server
		int new_socket;
		
		// No crea el socket
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		
		// Crear socket nuevo, y agregar a las listas segun el index vacio siguiente
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

		// Conexion exitosa del cliente y se le crea su propio thread
		printf("CLIENT %d CONECTED\n", new_socket);
		pthread_t thread;
		pthread_create(&thread, NULL, &listenClient, index);
	}
	
	google::protobuf::ShutdownProtobufLibrary();
	return 0; 
} 

