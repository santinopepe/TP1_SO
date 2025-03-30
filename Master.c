#include "Utilis.h"
#include <sys/select.h>
#include <time.h>

#define WIDTH_ERROR -1
#define HEIGHT_ERROR -1
#define PI 3.14159265358979323846

#define CAN_MOVE(x,y,width,height,new_pos)  (((x) >= 0 && (x) < (width)) && ((y) >= 0 && (y) < (height)) && ((new_pos) > 0))

#define DIRECTIONS 8 // Cantidad de direcciones posibles

const int movement[8][2] = {
    {0, -1},  // Arriba
    {1, -1},  // Arriba-Derecha
    {1, 0},   // Derecha
    {1, 1},    // Abajo-Derecha
    {0, 1},   // Abajo
    {-1, 1},  // Abajo-Izquierda
    {-1, 0},  // Izquierda
    {-1, -1}, // Arriba-Izquierda
};


void * create_shm(char * name, int size, int flags){

    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (-1 == ftruncate(fd, size)){
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    void * ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return ptr;
}



  
bool is_valid_move(Board *board, Player *player, int move, int width, int height){ 
    int x = player->coord_x;
    int y = player->coord_y;
  
    
    x += movement[move][0];
    y += movement[move][1];
  

    if(!CAN_MOVE(x,y,width,height,board->board_pointer[y * width + x])){
        return false;
    }

    return true;
}
  

void create_sem(sem_t * sem, int value){
    if (-1 == sem_init(sem, 1, value)){ 
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void initialize_board(Board * board){
    board->width = 10;
    board->height = 10;
    board->num_players = 0;
    board->has_ended = false;
}

bool check_is_player(char * player){
    if (strcmp(player, "-w") == 0 || strcmp(player, "-h") == 0 || strcmp(player, "-d") == 0 || strcmp(player, "-t") == 0 || strcmp(player, "-s") == 0 || strcmp(player, "-v") == 0){
        return false;
    }
    return true;
}

int get_param(int argc, char * argv[], int param_array[], char * player_array[], char * view){
    int num_players = 0;
    for (int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-w") == 0){
            param_array[0] = atoi(argv[i+1]);
            if(param_array[0] < 10){
                fprintf(stderr,"Ancho invalido");
                exit(WIDTH_ERROR);
            }
        } else if (strcmp(argv[i], "-h") == 0){
            param_array[1] = atoi(argv[i+1]);
            if(param_array[1] < 10 ){
                fprintf(stderr,"Alto invalido \n");
                exit(HEIGHT_ERROR);
            }
        } else if (strcmp(argv[i], "-d") == 0){
            param_array[2] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-t") == 0){
            param_array[3] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-s") == 0){
            param_array[4] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-v") == 0){
            (view) = argv[i+1];
        } else if(strcmp(argv[i], "-p")==0){
            i++;
           for (; num_players < MAX_PLAYERS || i < argc; num_players++, i++){
                if (argv[i] == NULL || !check_is_player(argv[i])){ 
                    break;
                }
                player_array[num_players] = argv[i];
           } 
        }     
    }
  
    return num_players;
}

typedef struct {
    int x;
    int y;
} Point;

Point* generate_circle(int n, int m, int num_points) {
    // Centro del círculo
    int center_x = n / 2;
    int center_y = m / 2;

    // Radio del círculo
    int radius = (n < m ? n : m) / 2 - 1;

    // Reservar memoria para los puntos
    Point* points = (Point*) malloc(num_points * sizeof(Point));
    if (points == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Generar puntos equidistantes en el círculo
    for (int i = 0; i < num_points; i++) {
        double angle = 2 * PI * i / num_points;
        points[i].x = center_x + (int)(radius * cos(angle));
        points[i].y = center_y + (int)(radius * sin(angle));
    }

    return points;
}

void initialize_game(int param_array[], char * player_array[], char * view, Board * board, int num_players, int write_fd){
    board->width = param_array[0];
    board->height = param_array[1];
    board->num_players = num_players;
    board->has_ended = false;
    srand(time(NULL));
    // Memory for board_pointer is already allocated with the Board structure
    for (int i = 0; i < board->width * board->height; i++) {
        board->board_pointer[i] = rand() % 9 + 1; // Initialize board cells to 0
    }
    char width[10];
    char height[10];
    sprintf(width,"%d", param_array[0]); 
    sprintf(height,"%d", param_array[1]);

    char * argv[3] = {width, height, NULL};

    Point * points = generate_circle(board->width, board->height, num_players);

    int player_it = 0;
    while(player_it < num_players || player_array[player_it] !=NULL){
        
        strcpy(board->player_list[player_it].name, player_array[player_it]);
        board->player_list[player_it].points = 0;
        board->player_list[player_it].ilegal_moves = 0;
        board->player_list[player_it].valid_moves = 0;
        board->player_list[player_it].coord_x = points[player_it].x;
        board->player_list[player_it].coord_y = points[player_it].y;
        board->player_list[player_it].is_blocked = true;

        board->player_list[player_it].pid = fork(); //CHECKEAR SI ESTO ESTA BIEN
        if (board->player_list[player_it].pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (board->player_list[player_it].pid == 0){
            if (write_fd < 0) {
                perror("write_fd no es válido");
                exit(EXIT_FAILURE);
            }
            dup2(write_fd, STDOUT_FILENO);
            close(write_fd);
            
            execve(player_array[player_it], argv ,NULL) ;
            perror("execve");
            exit(EXIT_FAILURE);
        }

        player_it++;
    }

    free(points);

    if (view != NULL){
        pid_t pid = fork();
        if (pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            execve(view, argv, NULL);
            perror("execve");
            exit(EXIT_FAILURE);
        }
    }

}

void initialize_sync(Sinchronization * sync, int num_players){
    create_sem(&sync->changes, 0);
    create_sem(&sync->view_done, 0);
    create_sem(&sync->master_mutex, 1);
    create_sem(&sync->game_state_mutex, 1);
    create_sem(&sync->variable_mutex, 1);
    sync->readers_count = num_players;
}

void check_can_move(Board * board, Player * player, int width, int height){
    int x = player->coord_x;
    int y = player->coord_y;
    for (int i = 0; i < 8; i++){
        if (CAN_MOVE(x + movement[i][0], y + movement[i][1], width, height, board->board_pointer[(y + movement[i][1]) * width + (x + movement[i][0])])){
            return;
        }
    }
    player->is_blocked = true;
    
}

void print_game_vals(int param_array[], char * player_array[], int num_players){
    printf("width: %d\n", param_array[0]); 
    printf("height: %d\n", param_array[1]);
    printf("delay: %d\n",param_array[2]);
    printf("timeout: %d\n",param_array[3]); 
    printf("seed: %d\n", param_array[4]);
    printf("num_players: %d\n", num_players); 
    for (int i = 0; i < num_players; i++){
        printf("Player %d: %s\n", i+1, player_array[i]);
    }
}

void move_player(Board * board, Player * player, int move, int width, int player_number){  
    player->coord_x += movement[move][0];
    player->coord_y += movement[move][1];
    player->points += board->board_pointer[player->coord_y * width + player->coord_x];
    board->board_pointer[player->coord_y * width + player->coord_x] = (-1)*player_number;
    player->valid_moves++;
}


int main(int argc, char * argv[]) {
    struct timeval timeout;

    int param_array[5] ={10, 10, 200, 10, time(NULL)}; 
    char * view = NULL;
    char * player_array[9]={NULL};
    int pipe_fd[MAX_PLAYERS][2]; //Manu pq esto es una matriz ?? dsp esto nos esta dando error en initialize_game cual el file descriptor que hay q mandar


    int num_players = get_param(argc, argv, param_array, player_array, view);
    
    Board * board = (Board *) create_shm(SHM_NAME_BOARD, sizeof(Board) + sizeof(int)*param_array[0]*param_array[1], O_CREAT);
    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_CREAT);
    
    initialize_sync(sync, num_players);
    initialize_board(board);



    


    //Creamos pipes de los players 
    for (int i = 0; i < num_players; i++){
        if (pipe(pipe_fd[i]) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }

    }


    initialize_game(param_array, player_array, view, board, num_players, pipe_fd[0][0]);
    print_game_vals(param_array,player_array,board->num_players);

    fd_set read_fds;
    unsigned char move;
    int blocked_players = 0;
   // int current_player = 0; //Jugador que tiene prioridad en el turno
    int max_fd = 0;
   int ready; 
    while(!board->has_ended){

        FD_ZERO(&read_fds);

        for (int i = 0; i < num_players; i++){
            FD_SET(pipe_fd[i][0], &read_fds);
            if (pipe_fd[i][0] > max_fd){
                max_fd = pipe_fd[i][0];
            }
        }

        timeout.tv_sec = param_array[3];
        timeout.tv_usec = 0;

        ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        for(int i = 0; i < num_players; i++){
            if (FD_ISSET(pipe_fd[i][0], &read_fds)){
                read(pipe_fd[i][0], &move, sizeof(unsigned char));

                sem_wait(&sync->master_mutex); //Bloqueo el acceso al board pq estoy modificando 
                sem_wait(&sync->game_state_mutex); //Bloqueo el acceso al board pq estoy modificando

                if (!is_valid_move(board, &board->player_list[i], move, board->width, board->height)){ 
                    board->player_list[i].ilegal_moves++;
                    check_can_move(board, &board->player_list[i], board->width, board->height);      
                    if(board->player_list[i].is_blocked){
                        blocked_players++;
                    }          
                } else{
                    move_player(board, &board->player_list[i], move, board->width, i);

                }

                if (blocked_players == num_players){ 
                    board->has_ended = true;
                    break;
                }

                sem_post(&sync->game_state_mutex); //Aviso que termine de leer el board.
                sem_post(&sync->master_mutex); //Aviso que termine de leer el board.

            }
        }


        sem_post(&sync->changes); //Aviso que hubienro cambios en el board. 

        sem_wait(&sync->view_done); //Espero a que la vista termine de imprimir el board. 
        
        /*
        sem_wait(&sync->game_state_mutex); //Espero a que el master termine de leer el board.

        for (int i = 0; i < num_players; i++) { //Este for por ahi este mal pq el player 0 siempre tiene prioridad
            close(pipe_fd[i][1]); // Cerrar el descriptor de escritura en el proceso principal
            read(pipe_fd[i][0], &move, sizeof(unsigned char));

            if (!is_valid_move(board, &board->player_list[i], move, board->width, board->height)){ 
                board->player_list[i].ilegal_moves++;
                check_can_move(board, &board->player_list[i], board->width, board->height);                
            } else{
                move_player(board, &board->player_list[i], move, board->width, i);
                if(board->player_list[i].is_blocked){
                    blocked_players++;
                }
            }

            if (blocked_players == num_players){ 
                board->has_ended = true;
                break;
            }
        }

        sem_post(&sync->game_state_mutex); //Aviso que termine de leer el board.

        sem_post(&sync->changes); //Aviso que hubienro cambios en el board. 

        sem_wait(&sync->view_done); //Espero a que la vista termine de imprimir el board. 
        */

    }


    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        close(pipe_fd[i][0]); // Cerrar el descriptor de lectura después de leer
        close(pipe_fd[i][1]); // Cerrar el descriptor de lectura después de leer

        waitpid(board->player_list[i].pid, NULL, 0); //CHEQUEAR SI ESTO VA ACA O ARRIBA DE LOS CLOSE
    }


    
     
    return 0;
}


/*
//Falta : 
+crear pipes (CREO QUE ESTA HECHO PERO NO SE)
+chequear si la movida que quiere hacer el player es valida, 
+validar los parametros que le entran cuando llamas al ejecutable
+todo el uso de semaforos

*/