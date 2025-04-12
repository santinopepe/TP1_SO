#include "Shm_Structs.h"
#include "Shm_Lib.h"
#include <stdlib.h>

  
const char *colors[] = {
    "\x1B[38;5;196m", // Red
    "\x1B[38;5;28m",  // Dark green
    "\x1B[38;5;27m",  // Blue
    "\x1B[38;5;220m", // Yellow
    "\x1B[38;5;201m", // Magenta
    "\x1B[38;5;51m",  // Cyan
    "\x1B[38;5;88m",  // Brown
    "\x1B[38;5;82m",  // Lime green
    "\x1B[38;5;15m"   // White
};
const char *reset_color = "\x1B[0m";

void print_board(Board * board) {
    printf("Board state:\n");
    for (int i = 0; i < board->height; i++) {
        printf("|");
        for (int j = 0; j < board->width; j++) {
            bool is_head = false;
            for (int k = 0; k < board->num_players; k++) {
                if (board->player_list[k].coord_x == j && board->player_list[k].coord_y == i) {
                    printf(" %s☻%s |", colors[k % 9], reset_color); // Print player head
                    is_head = true;
                    break;
                }
            }
            if (!is_head) {
                if (board->board_pointer[i * board->width + j] <= 0) {
                    printf(" %s❖%s |", colors[(-1) * board->board_pointer[i * board->width + j]], reset_color);
                } else {
                    printf(" %d |", board->board_pointer[i * board->width + j]);
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
        printf("Name: %s%s%s, Points: %u, Illegal Moves: %u, Valid Moves: %u, Position: (%u, %u), Can Move: %s\n",
            colors[i % 9],player->name, reset_color, player->points, player->ilegal_moves, player->valid_moves,
               player->coord_x, player->coord_y, player->is_blocked ? "No" : "Yes");
    }
    printf("\n");
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        printf("Uso: %s <ancho> <alto>\n", argv[0]);
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);


    // Conectar a las memorias compartidas
    Board * board = (Board *) open_shm(SHM_NAME_BOARD, sizeof(Board) + sizeof(int)*width*height, O_RDONLY);
    Sinchronization * sync = (Sinchronization *) open_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_RDWR);


    while (!board->has_ended){

        sem_wait(&(sync->changes));

        
        print_board(board);
        print_players(board);
        
        sem_post(&(sync->view_done));
        
    }

    // Cleanup
    close_shm(board, sizeof(Board) + sizeof(int)*width*height);
    close_shm(sync, sizeof(Sinchronization));

    return 0;
}
