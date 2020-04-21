// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <iostream>
#include "mensaje.pb.h"

using namespace std;
using namespace chat;

#define PORT 8080
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

int sock = 0;
char write_buffer[10];

static int getLine (char *prmpt, char *buff, size_t sz) {
	int ch, extra;

	// Get line with buffer overrun protection.
	if (prmpt != NULL)
	{
		printf ("%s", prmpt);
		fflush (stdout);
    	}
	if (fgets (buff, sz, stdin) == NULL)
	{
		return NO_INPUT;
	}

	// If it was too long, there'll be no newline. In that case, we flush
	// to end of line so that excess doesn't affect the next call.
	if (buff[strlen(buff)-1] != '\n')
	{
		extra = 0;
		while (((ch = getchar()) != '\n') && (ch != EOF))
		{
			extra = 1;
		}
	return (extra == 1) ? TOO_LONG : OK;
	}

	// Otherwise remove newline and give string back to caller.
	buff[strlen(buff)-1] = '\0';
	return OK;
}

void *listenServer(void *arg)
{
	while(1)
	{
		char read_buffer[1024] = {0};
		int valread; 
		valread = read( sock , read_buffer, sizeof(read_buffer)); 
		printf("%s\n",read_buffer );
	}
	return NULL;
}


int main(int argc, char const *argv[]) 
{ 
	GOOGLE_PROTOBUF_VERIFY_VERSION;

    char opcion;
	char opcionstatus;
	string usernameinfo;
	
	string username;
  	cout << "Ingresa tu nombre de usuario: ";
  	cin >> username;
		
	
	struct sockaddr_in serv_addr; 
	char *hello = "Hello from client"; 
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
	miInfo->set_username("username123");
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
	
	while(1)
	{

		int rc;
		rc = getLine ("", write_buffer, sizeof(write_buffer));

		if (rc == NO_INPUT)
		{
			// Extra NL since my system doesn't output that on EOF.
			printf ("\nNo input\n");
			return 1;
		}

		if (rc == TOO_LONG)
		{
			printf ("Input too long [%s]\n", write_buffer);
			return 1;
		}

		if(strcmp(write_buffer, "/m") == 0)
		{
			bool bandera = false;
			do {
				system("clear");
				cin.clear();
				cout << "Menu principal" << endl;
				cout << "-----------" << endl << endl;
				cout << "\t1 .- Regresar al chat principal" << endl;
				cout << "\t2 .- Enviar mensajes directos" << endl;
				cout << "\t3 .- Cambiar de status" << endl;
				cout << "\t4 .- Lista de usuarios conectados" << endl;
				cout << "\t5 .- Mostrar informacion de un usuario" << endl;
				cout << "\t6 .- Ayuda" << endl;
				cout << "\t7 .- Salir" << endl << endl;
				cout << "Elije una opcion: ";

				cin >> opcion;
			
				if(opcion == '1')
				{
					system("clear");
					cout << "Regresar al chat principal.\n";
					bandera = true;
				}
				else if(opcion == '2')
				{
					string tousername;
					string directmessage;

					system("clear");
					cin.clear();
		    		cout << "Enviar mensajes directos" << endl;
		    		cout << "-----------" << endl << endl;
		    		cout << "Ingresa el nombre de usuario: ";
		    		cin >> tousername;
					cin.clear();
					cout << "Ingresa el mensaje: ";
		    		cin >> directmessage;

					DirectMessageRequest *directMessageRequest(new DirectMessageRequest);
					directMessageRequest->set_message(directmessage);
					directMessageRequest->set_username(tousername);

					ClientMessage cm;
					cm.set_option(5);
					cm.set_allocated_directmessage(directMessageRequest);

					string binary2;
					cm.SerializeToString(&binary2);

					char cstr2[binary.size() + 1];
					strcpy(cstr2, binary.c_str());

					send(sock , cstr2, strlen(cstr2) , 0 );				
				}
				else if(opcion == '3')
				{
					system("clear");
					cin.clear();
		    			cout << "Cambiar de status" << endl;
		    			cout << "-----------" << endl << endl;
		    			cout << "\t1 .- Activo" << endl;
		    			cout << "\t2 .- Ocupado" << endl;
		    			cout << "\t3 .- Inactivo" << endl << endl;
		    			cout << "Elije una opcion: ";
		    			cin >> opcionstatus;
					switch(opcionstatus) {
						case '1':
							cout << "Cambiar a estado ACTIVO\n";
							break;
						case '2':
							cout << "Cambiar a estado OCUPADO\n";
							break;
						case '3':
							cout << "Cambiar a estado INACTIVO\n";
							break;
						default:
							cout << "Opcion no valida.\a\n";
							break;
					}
				}
				else if(opcion == '4')
				{
					system("clear");
					cout << "Lista de usuarios conectados.\n";
				}
				else if(opcion == '5')
				{
					system("clear");
					cin.clear();
					cout << "Mostrar informacion de un usuario" << endl;
					cout << "-----------" << endl << endl;
					cout << "Ingresa el nombre de usuario: ";
					cin >> usernameinfo;
				}
				else if(opcion == '6')
				{
					system("clear");
					cout << "Ayuda.\n";
				}
				else if(opcion == '7')
				{
					bandera = true;
					close(sock);
					exit(1);
				}
				else if(opcion > '7')
				{
					system("clear");
					cout << "Opcion no valida.\a\n";
				}
			} while(bandera != true);
		}
		else
		{
			BroadcastRequest *broadcastRequest(new BroadcastRequest);
			string str(write_buffer);
			broadcastRequest->set_message(str);

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
