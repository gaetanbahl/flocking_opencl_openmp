CC=g++
CFLAGS=-Wall -O
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lX11
EXEC=bin/flocking
SRCDIR=src
BINDIR=bin
INCLUDEDIR=include

all: $(EXEC)

bin/flocking: main.o boiddrawing.o
	$(CC) -o $(BINDIR)/flocking $(BINDIR)/main.o $(BINDIR)/boiddrawing.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c 
	$(CC) -o $(BINDIR)/main.o -c $(SRCDIR)/main.c -I$(INCLUDEDIR)/* $(CFLAGS)

boiddrawing.o: $(SRCDIR)/boiddrawing.cpp
	$(CC) -o $(BINDIR)/boiddrawing.o -c $(SRCDIR)/boiddrawing.cpp -I$(INCLUDEDIR)/* $(CFLAGS)

clean:
	rm -f $(BINDIR)/*.o 

mrproper: clean
	rm -f $(EXEC)
