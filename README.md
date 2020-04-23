# PROYECTO CHAT
## Sistemas Operativos - Seccion 10
## Christopher Sandoval 13660
## Maria Fernanda Estrada 14198


### Para compilar el cliente
g++ client.cpp mensaje.pb.cc -lprotobuf -lpthread -std=c++11 -o client

### Para compilar el servidor
g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -std=c++11 -o server
