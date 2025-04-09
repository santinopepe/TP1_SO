#include "Utilis.h"
#include "Shm_Lib.h"
#include <stdlib.h>
#include <time.h>

const int directions[8][2] = {
    {0, -1},  // Arriba
    {1, -1},  // Arriba-Derecha
    {1, 0},   // Derecha
    {1, 1},    // Abajo-Derecha
    {0, 1},   // Abajo
    {-1, 1},  // Abajo-Izquierda
    {-1, 0},  // Izquierda
    {-1, -1}, // Arriba-Izquierda
};


int find_best_path(Board *board, Player *player) {
    // Direcciones posibles: {dx, dy}
   

    int max_points, best_move, new_x, new_y;

    int width = board->width;
    int height = board->height;

    int player_x = player->coord_x;
    int player_y = player->coord_y;

    max_points = -1;
    best_move = -1;

    for (unsigned char i = 0; i < 8; i++) {
        new_x = player_x + directions[i][0];
        new_y = player_y + directions[i][1];

        // Verificar si la nueva posición está dentro de los límites del tablero
        if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) {
            // Calcular el índice de la celda en el arreglo unidimensional
            int cell_index = (new_y * width) + new_x;

            if (cell_index < 0 || cell_index >= width * height) {
                fprintf(stderr, "Error: cell_index fuera de los límites (%d)\n", cell_index);
                continue;  // Saltar este movimiento
            }

            int points = board->board_pointer[cell_index];

            // Actualizar el mejor movimiento si se encuentra una celda con más puntos
            if (points > max_points) {
                max_points = points;
                best_move = i;
            }
        }
    }


    // Retornar los puntos totales acumulados
    return best_move;
}


int main(int argc, char * argv[]){
    unsigned char move;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);


    // Conectar a las memorias compartidas
    Board * board = (Board * ) open_shm(SHM_NAME_BOARD, sizeof(Board), O_RDONLY);
    Sinchronization * sync = (Sinchronization *) open_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_RDWR);

    //veo que proceso es el que esta corriendo
    pid_t pid = getpid();
    int player_number = 0;
    while (player_number < board->num_players)    {
        if(board->player_list[player_number].pid == pid){
            break;
        }
        player_number++;
    }

    while (!board->player_list[player_number].is_blocked && !board->has_ended) {

        sem_wait(&sync->game_state_mutex); 
        sem_post(&sync->game_state_mutex); 

        sem_wait(&sync->variable_mutex);
        sync->readers_count++;
        if (sync->readers_count == 1) {
            sem_wait(&sync->master_mutex);  // Primer lector bloquea al master
        }
        sem_post(&sync->variable_mutex);

        move = find_best_path(board, &board->player_list[player_number]);

        int x = board->player_list[player_number].coord_x;
        int y = board->player_list[player_number].coord_y;
        
        sem_wait(&sync->variable_mutex);
        sync->readers_count--;
        if (sync->readers_count == 0) {
            sem_post(&sync->master_mutex);  // Último lector desbloquea al master
        }
        sem_post(&sync->variable_mutex);

        if (write(STDOUT_FILENO, &move, sizeof(unsigned char)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        } 
        
        while (board->board_pointer[(y+directions[move][1]) * width + (x+directions[move][0])] > 0);
       
    }

    // Cerrar la memoria compartida
    close_shm(board, sizeof(Board) + sizeof(int) * board->width * board->height);
    close_shm(sync, sizeof(Sinchronization));

    return 0;
}
