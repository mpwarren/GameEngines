CC=g++
SHAPESDIR = GameShapes/

section2: section2.o Platform.o MovingPlatform.o Player.o
	$(CC) section2.o Platform.o MovingPlatform.o Player.o -o section2 -lsfml-graphics -lsfml-window -lsfml-system

section2.o: section2.cpp $(SHAPESDIR)Platform.h $(SHAPESDIR)MovingPlatform.h
	$(CC) -c section2.cpp -o section2.o

Platform.o: $(SHAPESDIR)Platform.cpp $(SHAPESDIR)Platform.h
	$(CC) -c $(SHAPESDIR)Platform.cpp

MovingPlatform.o: $(SHAPESDIR)MovingPlatform.cpp $(SHAPESDIR)MovingPlatform.h
	$(CC) -c $(SHAPESDIR)MovingPlatform.cpp

Player.o: $(SHAPESDIR)Player.cpp $(SHAPESDIR)Player.h
	$(CC) -c $(SHAPESDIR)Player.cpp