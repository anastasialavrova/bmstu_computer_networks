all: 
	client server

client: 
	client.cpp general.h
	g++ -pthread client.cpp -o client

server: 
	server.cpp general.h
	g++ -pthread server.cpp -o server