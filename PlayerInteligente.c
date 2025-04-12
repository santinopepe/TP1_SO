#include "Shm_Structs.h"
#include "Shm_Lib.h"
#include <stdlib.h>
#include <time.h>

const int directions[8][2] = {
    {0, -1},  // Up
    {1, -1},  // Up-Right
    {1, 0},   // Right
    {1, 1},    // Down-Right
    {0, 1},   // Down
    {-1, 1},  // Down-Left
    {-1, 0},  // Left
    {-1, -1}, // Up-Left
};


int find_best_path(Board *board, Player *player) {
    // Possible directions: {dx, dy}
   

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

        
        if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) {
            // Calculate de index of the cell in the board
            int cell_index = (new_y * width) + new_x;

            if (cell_index < 0 || cell_index >= width * height) {
                fprintf(stderr, "Error: cell_index fuera de los límites (%d)\n", cell_index);
                continue;  
            }

            int points = board->board_pointer[cell_index];

        
            if (points > max_points) {
                max_points = points;
                best_move = i;
            }
        }
    }


    return best_move;
}


int main(int argc, char * argv[]){
    unsigned char move;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);


    
    Board * board = (Board * ) open_shm(SHM_NAME_BOARD, sizeof(Board), O_RDONLY);
    Sinchronization * sync = (Sinchronization *) open_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_RDWR);

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
            sem_wait(&sync->master_mutex);  // First reader locks the master
        }
        sem_post(&sync->variable_mutex);

        move = find_best_path(board, &board->player_list[player_number]);

        int x = board->player_list[player_number].coord_x;
        int y = board->player_list[player_number].coord_y;
        
        sem_wait(&sync->variable_mutex);
        sync->readers_count--;
        if (sync->readers_count == 0) {
            sem_post(&sync->master_mutex);  // Last reader unlocks the master
        }
        sem_post(&sync->variable_mutex);

        if (write(STDOUT_FILENO, &move, sizeof(unsigned char)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        } 
        
        while (board->board_pointer[(y+directions[move][1]) * width + (x+directions[move][0])] > 0);
    
    }
    
   
    close_shm(board, sizeof(Board) + sizeof(int) * board->width * board->height);
    close_shm(sync, sizeof(Sinchronization));

    return 0;
    
}
