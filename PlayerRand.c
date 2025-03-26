#include "Utilis.h"
#include <stdlib.h>
#include <time.h>



void * create_shm(char * name, int size, int flags){

  int fd = shm_open(name, flags, 0666);
  if (fd == -1) {
      perror("shm_open");
      exit(EXIT_FAILURE);
  }
  int aux;
  if(flags==O_RDONLY){
      aux = PROT_READ;
  } else{
      aux = PROT_READ | PROT_WRITE;
  }
  void * ptr = mmap(0, size, aux, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
  }

  return ptr;
}


int main(int argc, char * argv[]){
    unsigned char move;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

     

    // Conectar a las memorias compartidas
    Board * board = (Board * ) create_shm(SHM_NAME_BOARD, sizeof(Board), O_RDONLY);
    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_RDWR);

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
    
    
    while (!board->player_list[player_number].is_bolcked){
    // Incrementar readers_count
      sem_wait(&sync->variable_mutex); 
      sync->readers_count++;
      if (sync->readers_count == 1) {
          sem_wait(&sync->game_state_mutex);
      }
      sem_post(&sync->variable_mutex);

      // Enviar solicitud de movimiento al máster
      move = rand() % 8;

      // Decrementar readers_count
      sem_wait(&sync->variable_mutex);
      sync->readers_count--;
      if (sync->readers_count == 0) {
          sem_post(&sync->game_state_mutex);
      }
      sem_post(&sync->variable_mutex);

      
      
      // Escribir el movimiento en el pipe
      if (write(STDOUT_FILENO, &move, sizeof(unsigned char)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }      
  
      usleep(200000);
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