CC = gcc
CFLAGS = -Wall -lm -pedantic 

all: view player playerI master

view: Vista.c
	$(CC) $(CFLAGS) Vista.c  Shm_Lib.c -o view.out

player: PlayerRand.c
	$(CC) $(CFLAGS) PlayerRand.c Shm_Lib.c -o player.out

playerI:
	$(CC) $(CFLAGS) PlayerInteligente.c Shm_Lib.c -o playerI.out

master: 
	$(CC) $(CFLAGS) Master.c Shm_Lib.c Master_Lib.c -o master.out -lm

clean:
	rm -f view.out player.out playerI.out master.out