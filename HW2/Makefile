CC=g++
SHAPESDIR = GameShapes/

hw2exe: hw2.o CollidableObject.o MovingPlatform.o Platform.o Player.o
	$(CC) hw2.o CollidableObject.o MovingPlatform.o Platform.o Player.o -o hw2exe -lsfml-graphics -lsfml-window -lsfml-system -pthread

hw2.o: hw2.cpp $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)Platform.h $(SHAPESDIR)MovingPlatform.h
	$(CC) -c hw2.cpp -o hw2.o

CollidableObject.o: $(SHAPESDIR)CollidableObject.h $(SHAPESDIR)CollidableObject.cpp
	$(CC) -c $(SHAPESDIR)CollidableObject.cpp

Platform.o: $(SHAPESDIR)Platform.cpp $(SHAPESDIR)Platform.h $(SHAPESDIR)CollidableObject.h
	$(CC) -c $(SHAPESDIR)Platform.cpp

MovingPlatform.o: $(SHAPESDIR)MovingPlatform.cpp $(SHAPESDIR)MovingPlatform.h
	$(CC) -c $(SHAPESDIR)MovingPlatform.cpp

Player.o: $(SHAPESDIR)Player.cpp $(SHAPESDIR)Player.h $(SHAPESDIR)CollidableObject.h
	$(CC) -c $(SHAPESDIR)Player.cpp