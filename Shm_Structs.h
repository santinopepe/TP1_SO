#ifndef SHM_STRUCTS_H
#define SHM_STRUCTS_H

#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>

#define SHM_NAME_BOARD "/game_state"
#define SHM_NAME_SYNC "/game_sync"

typedef struct {
    char name[16]; // Player name
    unsigned int points; // Score of the player
    unsigned int ilegal_moves; // Quantity of illegal moves made
    unsigned int valid_moves; // Quantity of valid moves made
    unsigned short coord_x, coord_y; // Coordinates of the player
    pid_t pid; // Process ID of the player
    bool is_blocked; // Indicates if the player is blocked
} Player;

typedef struct {
    unsigned short width; 
    unsigned short height; 
    unsigned int num_players; 
    Player player_list[9]; 
    bool has_ended; 
    int board_pointer[];
} Board;


//Sincronizacion
typedef struct {
    sem_t changes; // Se usa para indicarle a la vista que hay cambios por imprimir
    sem_t view_done; // Se usa para indicarle al master que la vista terminó de imprimir
    sem_t master_mutex; // Mutex para evitar inanición del master al acceder al estado
    sem_t game_state_mutex; // Mutex para el estado del juego
    sem_t variable_mutex; // Mutex para la siguiente variable
    unsigned int readers_count; // Cantidad de jugadores leyendo el estado
} Sinchronization;


#endif // SHM_STRUCTS_H
