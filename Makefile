CC=g++
CFLAGS=-Wall -O3 -std=c++11 -fopenmp
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lX11 -lconfig++ -fopenmp
EXEC=bin/flocking
SRCDIR=src
BINDIR=bin
INCLUDEDIR=include

all: $(EXEC)

bin/flocking: main.o boid.o
	$(CC) -o $(BINDIR)/flocking $(BINDIR)/main.o $(BINDIR)/boid.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c 
	$(CC) -o $(BINDIR)/main.o -c $(SRCDIR)/main.c -I$(INCLUDEDIR)/* $(CFLAGS)

boid.o: $(SRCDIR)/boid.cpp
	$(CC) -o $(BINDIR)/boid.o -c $(SRCDIR)/boid.cpp -I$(INCLUDEDIR)/* $(CFLAGS)

clean:
	rm -f $(BINDIR)/*.o 

mrproper: clean
	rm -f $(EXEC)
