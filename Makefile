CC = gcc
CFLAGS = -Wall

all: view player playerI

view: Vista.c
	$(CC) $(CFLAGS) Vista.c -o view

player: PlayerRand.c
	$(CC) $(CFLAGS) PlayerRand.c -o player

playerI:
	$(CC) $(CFLAGS) PlayerInteligente.c -o playerI

clean:
	rm -f view player playerI
