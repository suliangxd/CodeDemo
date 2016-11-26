all:server client

server: server.cpp utility.h
  g++ server.cpp utility.h -o server

client: client.cpp utility.h
  g++ client.h utility.h -o client
 
clean:
	rm -rf server client
