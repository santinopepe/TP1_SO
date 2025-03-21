#include "Utilis.h"
#include <stdlib.h>
#include <time.h>



void * create_shm(char * name, int size){

    int fd = shm_open(name, O_RDONLY, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }


    void * ptr = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

int main(int argc, char * argv[]){

    int move;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    // Conectar a las memorias compartidas
    Board * board = (Board * ) create_shm(SHM_NAME_BOARD, sizeof(Board) + width * height * sizeof(int));
    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization));

    srand(time(NULL));

    while (!board->has_ended) {
        sem_wait(&sync->game_state_mutex);

        // Enviar solicitud de movimiento al máster
        move = rand() % 7 + 1;;

        sem_post(&sync->game_state_mutex);
    }

    return move;
}