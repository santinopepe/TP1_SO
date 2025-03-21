#include "Utilis.h"
#include <stdlib.h>
int toNum(char * str);



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
void print_board(Board * board) {
    printf("Board state:\n");
    for (int i = 0; i < board->hight; i++) {
        for (int j = 0; j < board->width; j++) {
            for(int k = 0; k < board->num_players; k++){
                if(board->player_list[k].coord_x == j && board->player_list[k].coord_y == i){
                    printf("X ");
                } else{
                    printf("%d ", board->board_pointer[i * board->width + j]);
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_players(Board * board) {
    printf("Players:\n");
    for (int i = 0; i < board->num_players; i++) {
        Player * player = &board->player_list[i];
        printf("Name: %s, Points: %u, Illegal Moves: %u, Valid Moves: %u, Position: (%u, %u), Can Move: %s\n",
               player->name, player->points, player->iligal_moves, player->valid_moves,
               player->coord_x, player->coord_y, player->can_move ? "Yes" : "No");
    }
    printf("\n");
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        printf("Uso: %s <ancho> <alto>\n", argv[0]);
        return 1;
    }

    int width = toNum(argv[1]);
    int height = toNum(argv[2]);

    // Conectar a las memorias compartidas
    Board * board = (Board *) create_shm(SHM_NAME_BOARD, sizeof(Board));
    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization));


        printf("entre1\n");

        int sem_value;

        // Esperar a que el máster indique que hay cambios
        //sem_wait(&sync->changes);

        printf("entre\n");

        // Imprimir el estado del juego
        print_board(board);
        print_players(board);

        // Indicar al máster que la vista terminó de imprimir
        sem_post(&sync->view_done);

    return 0;
}


int toNum(char * str){
    int num = 0;
    for(int i = 0; str[i] != '\0'; i++){
        num = num * 10 + str[i] - '0';
    }
    return num;
}

/*

int main(int argc, char * argv[]){
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    Board board = (Board) create_shm(SHM_NAME_BOARD, sizeof(Board) + width * height * sizeof(int));
    Sinchronization sync = (Sinchronization) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization));













    return 0;
}*/