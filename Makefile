CC = gcc
CFLAGS = -Wall

all: view player

view: Vista.c
	$(CC) $(CFLAGS) Vista.c -o view

player: PlayerRand.c
	$(CC) $(CFLAGS) PlayerRand.c -o player

clean:
	rm -f view player
