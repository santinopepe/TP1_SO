#include "Shm_Structs.h"
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



int main(int argc, char * argv[]){
    unsigned char move;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

     

    // Conectar a las memorias compartidas
    Board * board = (Board * ) open_shm(SHM_NAME_BOARD, sizeof(Board), O_RDONLY);
    Sinchronization * sync = (Sinchronization *) open_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_RDWR);

    //veo que proceso es el que esta corriendo
    pid_t pid = getpid();
    int player_number =0;
    while (player_number < board->num_players)
    {
      if(board->player_list[player_number].pid == pid){
        break;
      }
      player_number++;
    }


    srand(time(NULL));
    

    
    while (!board->player_list[player_number].is_blocked && !board->has_ended) {

      sem_wait(&sync->game_state_mutex); //Bloqueo el acceso al board pq estoy modificando
      sem_post(&sync->game_state_mutex); //Desbloqueo el acceso al board pq termine de modificar

    // Incrementar readers_count
      sem_wait(&sync->variable_mutex);
      sync->readers_count++;
      if (sync->readers_count == 1) {
          sem_wait(&sync->master_mutex);
      }
      sem_post(&sync->variable_mutex);

      // Enviar solicitud de movimiento al máster
      move = 0; 

      //Decrementar readers_count
      sem_wait(&sync->variable_mutex);
      sync->readers_count--;
      if (sync->readers_count == 0) {
          sem_post(&sync->master_mutex);
      }
      sem_post(&sync->variable_mutex);

      int x = board->player_list[player_number].coord_x;
      int y = board->player_list[player_number].coord_y;
      
      // Escribir el movimiento en el pipe
      if (write(STDOUT_FILENO, &move, sizeof(unsigned char)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }      
  
      while (board->board_pointer[(y+directions[move][1]) * width + (x+directions[move][0])] > 0);
    }     
    
  // Cleanup
  if (munmap(board, sizeof(Board) + sizeof(int)*width*height) == -1) {
      perror("munmap");
  }
  if (munmap(sync, sizeof(Sinchronization)) == -1) {
      perror("munmap");
  }
    return 0;
}