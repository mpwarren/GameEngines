CC=g++
SHAPESDIR = GameShapes/

all: server client

server: Server.o Timeline.o CollidableObject.o MovingPlatform.o Platform.o Player.o 
	$(CC) Server.o Timeline.o CollidableObject.o MovingPlatform.o Platform.o Player.o -o server -lsfml-graphics -lsfml-system -pthread -lzmq

client: Client.o CollidableObject.o MovingPlatform.o Platform.o Player.o
	$(CC) Client.o Timeline.o CollidableObject.o MovingPlatform.o Platform.o Player.o -o client -lsfml-graphics -lsfml-window -lsfml-system -pthread -lzmq

Server.o: Server.cpp  $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)Platform.h $(SHAPESDIR)MovingPlatform.h
	$(CC) -c Server.cpp -lzmq -lpthread

Client.o: Client.cpp  $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)Platform.h $(SHAPESDIR)MovingPlatform.h
	$(CC) -c Client.cpp -lzmq -lpthread

Timeline.o: Timeline.h Timeline.cpp
	$(CC) -c Timeline.cpp

CollidableObject.o: $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)CollidableObject.cpp
	$(CC) -c $(SHAPESDIR)CollidableObject.cpp

Platform.o: $(SHAPESDIR)Platform.cpp $(SHAPESDIR)Platform.h $(SHAPESDIR)CollidableObject.h
	$(CC) -c $(SHAPESDIR)Platform.cpp

MovingPlatform.o: $(SHAPESDIR)MovingPlatform.cpp $(SHAPESDIR)MovingPlatform.h
	$(CC) -c $(SHAPESDIR)MovingPlatform.cpp

Player.o: $(SHAPESDIR)Player.cpp $(SHAPESDIR)Player.h $(SHAPESDIR)CollidableObject.h
	$(CC) -c $(SHAPESDIR)Player.cpp