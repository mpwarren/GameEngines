CC=g++
SHAPESDIR = GameShapes/

all: server client

server: MainServer.o CollidableObject.o MovingPlatform.o Platform.o Player.o
	$(CC) MainServer.o CollidableObject.o MovingPlatform.o Platform.o Player.o -o server -lsfml-graphics -lsfml-system -pthread -lzmq

client: GameClient.o CollidableObject.o MovingPlatform.o Platform.o Player.o
	$(CC) GameClient.o CollidableObject.o MovingPlatform.o Platform.o Player.o -o client -lsfml-graphics -lsfml-window -lsfml-system -pthread -lzmq

MainServer.o: MainServer.cpp  $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)Platform.h $(SHAPESDIR)MovingPlatform.h
	$(CC) -c MainServer.cpp -lzmq -lpthread

GameClient.o: GameClient.cpp  $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)Platform.h $(SHAPESDIR)MovingPlatform.h
	$(CC) -c GameClient.cpp -lzmq -lpthread

CollidableObject.o: $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)CollidableObject.cpp
	$(CC) -c $(SHAPESDIR)CollidableObject.cpp

Platform.o: $(SHAPESDIR)Platform.cpp $(SHAPESDIR)Platform.h $(SHAPESDIR)CollidableObject.h
	$(CC) -c $(SHAPESDIR)Platform.cpp

MovingPlatform.o: $(SHAPESDIR)MovingPlatform.cpp $(SHAPESDIR)MovingPlatform.h
	$(CC) -c $(SHAPESDIR)MovingPlatform.cpp

Player.o: $(SHAPESDIR)Player.cpp $(SHAPESDIR)Player.h $(SHAPESDIR)CollidableObject.h
	$(CC) -c $(SHAPESDIR)Player.cpp