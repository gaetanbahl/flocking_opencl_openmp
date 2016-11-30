CC=g++
CFLAGS=-Wall -O
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system
EXEC=bin/flocking
SRCDIR=src
BINDIR=bin

all: $(EXEC)

bin/flocking: main.o
	$(CC) -o $(BINDIR)/flocking $(BINDIR)/main.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) -o $(BINDIR)/main.o -c $(SRCDIR)/main.c $(CFLAGS)

clean:
	rm -f $(BINDIR)/*.o 

mrproper: clean
	rm -f $(EXEC)
