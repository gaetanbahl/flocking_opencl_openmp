CC=g++
CFLAGS=-Wall -O3 -std=c++11 -fopenmp
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lX11 -lconfig++ -lOpenCL -fopenmp
EXEC=bin/flocking
SRCDIR=src
BINDIR=bin
INCLUDEDIR=include
OCLCOMMON=include/common/*

all: $(EXEC)

bin/flocking: main.o boid.o intel
	$(CC) -o $(BINDIR)/flocking $(BINDIR)/*.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c 
	$(CC) -o $(BINDIR)/main.o -c $(SRCDIR)/main.c -I$(INCLUDEDIR)/* $(CFLAGS)

boid.o: $(SRCDIR)/boid.cpp
	$(CC) -o $(BINDIR)/boid.o -c $(SRCDIR)/boid.cpp -I$(INCLUDEDIR)/* $(CFLAGS)

intel: $(OCLCOMMON)
	$(CC) -c $(OCLCOMMON) $(CFLAGS)
	mv *.o $(BINDIR)

clean: mrproper
	rm -f $(BINDIR)/*.o 

mrproper: clean
	rm -f $(EXEC)
