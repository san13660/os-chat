# PROYECTO CHAT
### Sistemas Operativos - Seccion 10
### Christopher Sandoval 13660
### Maria Fernanda Estrada 14198

### ------------------------------------------------------------------------------------------

#### Para compilar el cliente
g++ client.cpp mensaje.pb.cc -lprotobuf -lpthread -std=c++11 -o client
#### Para ejecutar el cliente
<nombredelcliente> <nombredeusuario> <IPdelservidor> <puertodelservidor>
Ejemplo: ./client chris123 127.0.0.1 8080

### ------------------------------------------------------------------------------------------
#### Para compilar el servidor
g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -std=c++11 -o server
### Para ejecutar el servidor
<nombredelservidor> <puertodelservidor>
Ejemplo: ./server 8080
 
### ------------------------------------------------------------------------------------------
Despues de ingresar como cliente, se desplegara un menu con los comandos disponibles. A partir de aqui ya se puede comenzar a escribir en el chat.
