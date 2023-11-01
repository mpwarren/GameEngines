CC=g++

all: server client

server: server.cpp
	g++ server.cpp -lzmq -lpthread -o server

client: client.cpp
	g++ client.cpp -lzmq -o client