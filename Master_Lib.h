#include "Utilis.h"
#include "Shm_Lib.h"
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>


/* This struct is used to store the initial 
*  coordinates of the players.
*/
typedef struct {
    int x;
    int y;
} Point;


/**
 * @brief Set the parameters of the game on an array which is easier to handle.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @param param_array The array where the parameters will be stored.
 * @param player_array The array where the players will be stored.
 * @param view Pointer to the view.
*/
int set_params(int argc, char *argv[], int param_array[], char *player_array[], char **view);


/**
 * @brief Check if the argument that is being read is a player or flag. Used in set_params.
 * @param player The player to check.
 * @return True if the player is valid, false otherwise.
*/
bool check_is_player(char *player);


/**
 * @brief Print the values of the game.
 * @param param_array The array with the parameters.
 * @param player_array The array with the players.
 * @param num_players The number of players.
 * @param view The view.
*/
void print_game_vals(int param_array[], char * player_array[], char * view,int num_players); 



/**
 * @brief Initialize the synchronization variables.
 * @param sync The synchronization struct.
*/
void initialize_sync(Sinchronization * sync); 


/**
 * @brief Initialize the game.
 * @param board The board.
 * @param param_array The array with the parameters.
 * @param player_array The array with the players.
 * @param num_players The number of players.
 * @param view The view.
 * @param write_fd Matrix with the file descriptors.
*/
pid_t initialize_game(Board * board, int param_array[], char * player_array[], int num_players, char * view, int write_fd[][2]); 

/**
 * @brief Generate the random values on the board. Used in initialize_game.
 * @param board_pointer The pointer to the board.
 * @param width The width of the board.
 * @param height The height of the board.
*/
void generate_board(int * board_pointer, int width, int height, int seed); 



/**
 * @brief Generate a circle where the player will start the game. Used in initialize_game.
 * @param width The width of the board.
 * @param height The height of the board.
 * @param num_points The number of points.
 * @return An array of points.
*/
Point * generate_circle(int width, int height, int num_points, Point * points); 

/**
 * @brief Move the player on the board. Used in main
 * @param board The board.
 * @param player The player.
 * @param move The move.
 * @param width The width of the board.
 * @param height The height of the board.
 * @param player_number The number of the player.
*/

void move_player(Board * board, Player * player, int move, int width, int player_number);


/**
 * @brief Check if the player can move. Used in main.
 * @param board The board.
 * @param player The player.
 * @param width The width of the board.
 * @param height The height of the board.
 * @param player_number The number of the player.
 * 
*/

void check_can_move(Board * board, Player * player, int width, int height);



/**
 * @brief Create a semaphore. Used in initialize_sync.
 * @param sem The semaphore.
 * @param value The value of the semaphore.
*/
void create_sem(sem_t * sem, int value); 


/**
 * @brief Check if the move is valid. Used in main.
 * @param board The board.
 * @param player The player.
 * @param move The move.
 * @param width The width of the board.
 * @param height The height of the board.
 * @return True if the move is valid, false otherwise.
*/

bool is_valid_move(Board *board, Player *player, int move, int width, int height); 
