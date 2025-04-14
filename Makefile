CC = gcc
CFLAGS = -Wall -lm -pedantic 

all: view playerI master

view: Vista.c
	$(CC) $(CFLAGS) Vista.c  Shm_Lib.c -o view.out

playerI:
	$(CC) $(CFLAGS) PlayerInteligente.c Shm_Lib.c -o playerI.out

master: 
	$(CC) $(CFLAGS) Master.c Shm_Lib.c Master_Lib.c -o master.out -lm

clean:
	rm -f view.out playerI.out master.out