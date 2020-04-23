// Universidad del Valle de Guatemala
// Sistemas Operativos - Seccion 10
// 22/04/2020
// Proyecto CHAT (lado del cliente)
// Christopher Sandoval 13660
// Maria Fernanda Estrada 14198


// Librerias a utilizar
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sstream>
#include <iostream>
#include "mensaje.pb.h"

// Variables
using namespace std;
using namespace chat;

int sock = 0;
string ipServer = "";
int portServer = 0;

int userIDs[99] = {0};
string usernames[99] = {""};

// El cliente se queda escuchando lo que provenga del servidor
void *listenServer(void *arg)
{
	while(1) // Quedarse esperando informacion hasta que se desconecte
	{
		char read_buffer[1024] = {0};
		int valread; 
		valread = read( sock , read_buffer, sizeof(read_buffer)); 
		
		// Salir del programa si hay error de conexion
		if(valread == 0)
		{
			cout << "SE PERDIO LA CONEXION AL SERVIDOR" << endl;
			exit(1);
			break;
		}
		
		// Posibilidades de mensajes del server
		string str(read_buffer);

		ServerMessage serverMessage;
		serverMessage.ParseFromString(str);
		
		// Broadcast del chat general
		if(serverMessage.option() == 1)
		{
			cout << "[" << serverMessage.broadcast().username() << "] " << serverMessage.broadcast().message() << endl;
		}
		// Mensaje directo de otro usuario
		else if(serverMessage.option() == 2)
		{
			cout << "[MENSAJE DIRECTO DE " <<  serverMessage.message().username() << "] " << serverMessage.message().message() << endl;
		}
		// Confirmacion al cambiar status
		else if(serverMessage.option() == 3)
		{
			cout << "[ERROR] " << serverMessage.error().errormessage() << endl;
		}
		// Confirmacion al cambiar status
		else if(serverMessage.option() == 4)
		{
			cout << "[CONFIRMACION DE SYNC]" << endl;
		}
		// Lista de usuarios conectados, solicitada por el cliente
		else if(serverMessage.option() == 5)
		{
			cout << "[USUARIOS CONECTADOS](" << serverMessage.connecteduserresponse().connectedusers_size() << ")" << endl;
			for(int i = 0; i < serverMessage.connecteduserresponse().connectedusers_size(); i++)
			{
				fill_n(userIDs, userIDs, 0);
				userIDs[i] = serverMessage.connecteduserresponse().connectedusers(i).userid();
				usernames[i] = serverMessage.connecteduserresponse().connectedusers(i).username();

				string username_i = serverMessage.connecteduserresponse().connectedusers(i).username();
				string ip_i = serverMessage.connecteduserresponse().connectedusers(i).ip();
				string status_i = serverMessage.connecteduserresponse().connectedusers(i).status();
				int userId_i = serverMessage.connecteduserresponse().connectedusers(i).userid();
				cout << "[" << username_i << "](" << status_i << ")" << endl;
			}
		}
		// Confirmacion al cambiar status
		else if(serverMessage.option() == 6)
		{
			cout << "[CONFIRMACION CAMBIO DE STATUS]" << endl;
		}
		// Confirmacion al cambiar status
		else if(serverMessage.option() == 7)
		{
			cout << "[CONFIRMACION DE ENVIO BROADCAST]" << endl;
		}
		// Confirmacion al cambiar status
		else if(serverMessage.option() == 8)
		{
			cout << "[CONFIRMACION DE ENVIO DIRECTO]" << endl;
		}
		else
		{
			cout << "[SABER][" << serverMessage.option() << "]" << endl;
		}
		
	}
	return NULL;
}

// main
int main(int argc, char* argv[]) 
{ 
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	// Variables de input
	string opcion;
	string opcionstatus;
	string username;
	string directmessage;
	string tousername;
	string usernameinfo;
	
	// Siempre antes de ingresar y establecer conexion, se pide el usuario al cliente
	username = argv[1];
	stringstream ssPort(argv[3]);
	ssPort >> portServer; 
	ipServer = argv[2];

	cout << "IP: " << ipServer << endl;
	
	// Establecer conexion con el server, se le asigna un socket
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0};
	
	// Si no se puede asignar socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(portServer); 
	 
	if(inet_pton(AF_INET, ipServer.c_str(), &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}

	// ---------------- Inicia sincronizacion con el server (3 WAY HANDSHAKE) -------------------------
	MyInfoSynchronize * miInfo(new MyInfoSynchronize);
	miInfo->set_username(username);
	miInfo->set_ip("127.0.0.1");
	
	ClientMessage m;
	m.set_option(1);
	m.set_allocated_synchronize(miInfo);

	string binary;
	m.SerializeToString(&binary);

	char cstr[binary.size() + 1];
	strcpy(cstr, binary.c_str());

	send(sock , cstr, strlen(cstr) , 0 );
	
	char read_buffer[1024] = {0};
	int valread; 
	valread = read( sock , read_buffer, sizeof(read_buffer));

	string str(read_buffer);

	ServerMessage serverMessage;
	serverMessage.ParseFromString(str);

	if(serverMessage.option() == 4)
	{
		cout << "USER ID: " << serverMessage.myinforesponse().userid() << endl;
		
		MyInfoAcknowledge * myInfoAcknowledge(new MyInfoAcknowledge);
		myInfoAcknowledge->set_userid(serverMessage.myinforesponse().userid());
	
		ClientMessage cm2;
		cm2.set_option(6);
		cm2.set_allocated_acknowledge(myInfoAcknowledge);

		string binary;
		cm2.SerializeToString(&binary);

		char cstr[binary.size() + 1];
		strcpy(cstr, binary.c_str());

		send(sock , cstr, strlen(cstr) , 0 );
	}

	// ---------------- Finaliza sincronizacion y comienza el programa del chat --------------------
	
	pthread_t thread;
	pthread_create(&thread, NULL, &listenServer, NULL);
	
	// Menu de opciones e intrucciones de los comandos
	cout << "\nMenu de opciones" << endl << endl;
	cout << "-----------" << endl;
	cout << "En cualquier momento, puedes ingresar los siguientes comandos segun lo que necesites" << endl << endl;
	cout << "\tEnviar mensajes directos: /mensajedirecto <username> <mensaje>" << endl;
	cout << "\tCambiar de status: /cambiarstatus <status>" << endl;
	cout << "\tLista de usuarios conectados: /listado" << endl;
	cout << "\tMostrar informacion de un usuario: /infousuario <username>" << endl;
	cout << "\tAyuda: /ayuda" << endl;
	cout << "\tSalir: /salir" << endl << endl << endl << endl << endl;

	while(1)
	{
		// Input de chat
		string input;
		cin.clear();
		getline(cin, input);
		
		// Separar y determinar si se comenzo con un comando especial
		string delimiter = " ";
		string command = input.substr(0, input.find(delimiter));
		
		// Comando de mensaje directo
		if(command == "/mensajedirecto")
		{
			string tokenmessage = input.substr(input.find(delimiter) + 1, input.length() - 1);
			string directuser = tokenmessage.substr(0, tokenmessage.find(delimiter));
			string directmessage = tokenmessage.substr(tokenmessage.find(delimiter) + 1, tokenmessage.length() - 1);

			DirectMessageRequest *directMessageRequest(new DirectMessageRequest);
			directMessageRequest->set_message(directmessage);
			directMessageRequest->set_username(directuser);

			ClientMessage cm;
			cm.set_option(5);
			cm.set_allocated_directmessage(directMessageRequest);

			string binary2;
			cm.SerializeToString(&binary2);

			char cstr2[binary2.size() + 1];
			strcpy(cstr2, binary2.c_str());

			send(sock , cstr2, strlen(cstr2) , 0 );				
		}
		
		// Comando de cambiar status
		else if(command == "/cambiarstatus")
		{
			string new_status = input.substr(input.find(delimiter) + 1, input.length() - 1);
			if (new_status == "ACTIVO" || new_status == "OCUPADO" || new_status == "INACTIVO") {
				cout << "ESTADO CAMBIADO A " << new_status << endl;

				ChangeStatusRequest *changeStatusRequest(new ChangeStatusRequest);
				changeStatusRequest->set_status(new_status);

				ClientMessage cm;
				cm.set_option(3);
				cm.set_allocated_changestatus(changeStatusRequest);

				string binary2;
				cm.SerializeToString(&binary2);

				char cstr2[binary2.size() + 1];
				strcpy(cstr2, binary2.c_str());

				send(sock , cstr2, strlen(cstr2) , 0 );	
			}
			else 
			{
				cout << "OPCION NO VALIDA.\a\n";
			}
		}
		
		// Comando de listado general de usuarios conectados
		else if(command == "/listado")
		{
			connectedUserRequest *cUserRequest(new connectedUserRequest);
			cUserRequest->set_userid(0);

			ClientMessage cm;
			cm.set_option(2);
			cm.set_allocated_connectedusers(cUserRequest);

			string binary2;
			cm.SerializeToString(&binary2);

			char cstr2[binary2.size() + 1];
			strcpy(cstr2, binary2.c_str());

			send(sock , cstr2, strlen(cstr2) , 0 );	
		}
		
		// Comando de solicitud de informacion de usuario especifico
		else if(command == "/infousuario")
		{
			string usernameinfo = input.substr(input.find(delimiter) + 1, input.length() - 1);
			
			connectedUserRequest *cUserRequest(new connectedUserRequest);
			cUserRequest->set_userid(1);
			cUserRequest->set_username(usernameinfo);

			ClientMessage cm;
			cm.set_option(2);
			cm.set_allocated_connectedusers(cUserRequest);

			string binary2;
			cm.SerializeToString(&binary2);

			char cstr2[binary2.size() + 1];
			strcpy(cstr2, binary2.c_str());

			send(sock , cstr2, strlen(cstr2) , 0 );	
		}
		
		// Comando de ayuda (volver a mostrar los comandos)
		else if(command == "/ayuda")
		{
			cout << "\nMenu de opciones" << endl << endl;
			cout << "-----------" << endl;
			cout << "En cualquier momento, puedes ingresar los siguientes comandos segun lo que necesites" << endl << endl;
			cout << "\tEnviar mensajes directos: /mensajedirecto <username> <mensaje>" << endl;
			cout << "\tCambiar de status: /cambiarstatus <status>" << endl;
			cout << "\tLista de usuarios conectados: /listado" << endl;
			cout << "\tMostrar informacion de un usuario: /infousuario <username>" << endl;
			cout << "\tAyuda: /ayuda" << endl;
			cout << "\tSalir: /salir" << endl << endl << endl << endl << endl;
		}
		
		// Salir del chat, se debe cerrar el socket de este lado tambien
		else if(command == "/salir")
		{
			close(sock);
			exit(1);
		}
		
		// Si no es ningun comando, broadcastear por default los mensajes a todos
		else
		{
			BroadcastRequest *broadcastRequest(new BroadcastRequest);

			broadcastRequest->set_message(input);

			ClientMessage m;
			m.set_option(4);
			m.set_allocated_broadcast(broadcastRequest);

			string binary;
			m.SerializeToString(&binary);

			char cstr[binary.size() + 1];
			strcpy(cstr, binary.c_str());

			send(sock , cstr, strlen(cstr) , 0 );
		}
	}

	google::protobuf::ShutdownProtobufLibrary();
	return 0; 
}
