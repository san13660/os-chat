// Client side C/C++ program to demonstrate Socket programming 
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

using namespace std;
using namespace chat;

#define PORT 8080
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

int sock = 0;

void *listenServer(void *arg)
{
	while(1)
	{
		char read_buffer[1024] = {0};
		int valread; 
		valread = read( sock , read_buffer, sizeof(read_buffer)); 
		
		if(valread == 0)
		{
			cout << "CONNECTION TO SERVER LOST" << endl;
			exit(1);
			break;
		}
		
		string str(read_buffer);

		ServerMessage serverMessage;
		serverMessage.ParseFromString(str);
		
		if(serverMessage.option() == 1)
		{
			cout << "[" << serverMessage.broadcast().username() << "] " << serverMessage.broadcast().message() << endl;
		}
		else if(serverMessage.option() == 2)
		{
			cout << "[Mensaje directo de " <<  serverMessage.message().username() << "] " << serverMessage.message().message() << endl;
		}
		else if(serverMessage.option() == 5)
		{
			cout << "[USUARIOS CONECTADOS](" << serverMessage.connecteduserresponse().connectedusers_size() << ")" << endl;
			for(int i = 0; i < serverMessage.connecteduserresponse().connectedusers_size(); i++)
			{
				string username_i = serverMessage.connecteduserresponse().connectedusers(i).username();
				string ip_i = serverMessage.connecteduserresponse().connectedusers(i).ip();
				string status_i = serverMessage.connecteduserresponse().connectedusers(i).status();
				int userId_i = serverMessage.connecteduserresponse().connectedusers(i).userid();
				cout << "[" << username_i << "](" << status_i << ")" << endl;
			}
		}
		else
		{
			cout << "[UNKNOWN SERVER MESAGE][" << serverMessage.option() << "]" << endl;
		}
		
	}
	return NULL;
}


int main(int argc, char const *argv[]) 
{ 
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	string opcion;
	string opcionstatus;
	string username;
	string directmessage;
	string tousername;
	string usernameinfo;
	
	cin.clear();
  	cout << "Ingresa tu nombre de usuario: ";
 	getline(cin, username);
		
	
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}

	//----------------START SYNCRONIZE INFO------------------------------------
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
		cm2.set_option(8);
		cm2.set_allocated_acknowledge(myInfoAcknowledge);

		string binary;
		cm2.SerializeToString(&binary);

		char cstr[binary.size() + 1];
		strcpy(cstr, binary.c_str());

		send(sock , cstr, strlen(cstr) , 0 );
	}

	//----------------END SYNCRONIZE INFO------------------------------------
	
	pthread_t thread;
	pthread_create(&thread, NULL, &listenServer, NULL);
	
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
		string input;
		cin.clear();
		getline(cin, input);

		string delimiter = " ";
		string command = input.substr(0, input.find(delimiter));
		
		if(command == "/mensajedirecto")
		{
			string tokenmessage = input.substr(input.find(delimiter) + 1, input.length() - 1);
			string directuser = tokenmessage.substr(0, tokenmessage.find(delimiter));
			string directmessage = tokenmessage.substr(tokenmessage.find(delimiter) + 1, tokenmessage.length() - 1);

			system("clear");
			cout << "\nEnviar mensajes directos" << endl;
			cout << "-----------" << endl << endl;
			cout << "Ingresa el nombre de usuario: ";
			cin.clear();
			getline(cin, tousername);
			cout << "Ingresa el mensaje: ";
			cin.clear();
			getline(cin, directmessage);

			DirectMessageRequest *directMessageRequest(new DirectMessageRequest);
			directMessageRequest->set_message(directmessage);
			directMessageRequest->set_username(tousername);

			ClientMessage cm;
			cm.set_option(5);
			cm.set_allocated_directmessage(directMessageRequest);

			string binary2;
			cm.SerializeToString(&binary2);

			char cstr2[binary2.size() + 1];
			strcpy(cstr2, binary2.c_str());

			send(sock , cstr2, strlen(cstr2) , 0 );				
		}
		else if(command == "/cambiarstatus")
		{
			string tokenstatus = input.substr(input.find(delimiter) + 1, input.length() - 1);
			
			if (tokenstatus == "1" || tokenstatus == "2" || tokenstatus == "3") {
					string new_status;
					if (tokenstatus == "1")
					{
						new_status = "ACTIVO";
					}
					else if (tokenstatus == "2")
					{
						new_status = "OCUPADO";
					}
					else
					{
						new_status = "INACTIVO";
					}
					
					cout << "Estado Cambiado a " << new_status << endl;

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
					cout << "Opcion no valida.\a\n";
				}
		}
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
		else if(command == "/infousuario")
		{
			system("clear");
			cin.clear();
			cout << "Mostrar informacion de un usuario" << endl;
			cout << "-----------" << endl << endl;
			cout << "Ingresa el nombre de usuario: ";
			getline(cin, usernameinfo);

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
		else if(command == "/salir")
		{
			close(sock);
			exit(1);
		}
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
