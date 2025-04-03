CC = gcc
CFLAGS = -Wall -lm -pedantic 

all: view player playerI master

view: Vista.c
	$(CC) $(CFLAGS) Vista.c -o view.o

player: PlayerRand.c
	$(CC) $(CFLAGS) PlayerRand.c -o player.o

playerI:
	$(CC) $(CFLAGS) PlayerInteligente.c -o playerI.o

master: 
	$(CC) $(CFLAGS) Master.c -o master.o -lm

clean:
	rm -f view.o player.o playerI.o master.o