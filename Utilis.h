#ifndef TP_CHOMP_CHAMP_UTILIS_H
#define TP_CHOMP_CHAMP_UTILIS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <math.h>
#include <stdbool.h>



#define SHM_NAME_BOARD "/game_state"
#define SHM_NAME_SYNC "/game_sync"

#define MAX_PLAYERS 9

typedef struct {
    char name[16]; // Nombre del jugador
    unsigned int points; // Puntaje
    unsigned int iligal_moves; // Cantidad de solicitudes de movimientos inválidas realizadas
    unsigned int valid_moves; // Cantidad de solicitudes de movimientos válidas realizadas
    unsigned short coord_x, coord_y; // Coordenadas x e y en el tablero
    pid_t pid; // Identificador de proceso
    bool can_move; // Indica si el jugador tiene movimientos válidos disponibles
} Player;

typedef struct {
    unsigned short width; // Ancho del tablero
    unsigned short hight; // Alto del tablero
    unsigned int num_players; // Cantidad de jugadores
    Player player_list[9]; // Lista de jugadores
    bool has_ended; // Indica si el juego se ha terminado
    int board_pointer[]; // Puntero al comienzo del tablero. fila-0, fila-1, ..., fila-n-1
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


void * create_shm(char * name, int size);

void create_sem(sem_t * sem, int value);

void initialize_board(Board * board);

















#endif //TP_CHOMP_CHAMP_UTILIS_H
