#include "Utilis.h"
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>


/*---------------------------------------------------------------------Defines----------------------------------------------------------------------------------------------------------*/

#define PI 3.14159265358979323846

#define CAN_MOVE(x,y,width,height,new_pos)  (((x) >= 0 && (x) < (width)) && ((y) >= 0 && (y) < (height)) && ((new_pos) > 0))

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
/*---------------------------------------------------------------------Struct----------------------------------------------------------------------------------------------------------*/

/* This struct is used to store the initial 
*  coordinates of the players.
*/
typedef struct {
    int x;
    int y;
} Point;


/*---------------------------------------------------------------------Prototypes----------------------------------------------------------------------------------------------------------*/





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
 * @brief Create a shared memory segment.
 * @param name The name of the shared memory segment.
 * @param size The size of the shared memory segment.
 * @param flags The flags to open the shared memory segment.
 * @return A pointer to the shared memory segment.
*/
void * create_shm(char * name, int size, int flags); 

/**
 * @brief Initialize the synchronization variables.
 * @param sync The synchronization struct.
*/
void initialize_sync(Sinchronization * sync); 

/**
 * @brief Create a semaphore. Used in initialize_sync.
 * @param sem The semaphore.
 * @param value The value of the semaphore.
*/
void create_sem(sem_t * sem, int value); 


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
Point * generate_circle(int width, int height, int num_points); 

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
 * @brief Check if the move is valid. Used in main.
 * @param board The board.
 * @param player The player.
 * @param move The move.
 * @param width The width of the board.
 * @param height The height of the board.
 * @return True if the move is valid, false otherwise.
*/

bool is_valid_move(Board *board, Player *player, int move, int width, int height); 


/*---------------------------------------------------------------------Main----------------------------------------------------------------------------------------------------------*/


int main(int argc, char *argv[]) {


    int param_array[5] = {10, 10, 200, 10, time(NULL)};
    char *player_array[9] = {NULL};
    char *view = NULL;
    int pipe_fd[MAX_PLAYERS][2]; 

    int num_players = set_params(argc, argv, param_array, player_array, &view);

    print_game_vals(param_array, player_array, view, num_players);

    Board * board = (Board *) create_shm(SHM_NAME_BOARD, sizeof(Board) + sizeof(int)*param_array[0]*param_array[1], O_CREAT); 

    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization), O_CREAT);
    
    initialize_sync(sync);
    int max_fd=0;
    for (int i = 0; i < num_players; i++){

        if (pipe(pipe_fd[i]) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        
    }

    pid_t view_pid = initialize_game(board, param_array, player_array, num_players, view, pipe_fd);

    fd_set read_fds;
    unsigned char move;
    int blocked_players = 0;
    int ready; 
    struct timeval timeout;

    for(int i = 0; i < num_players; i++){
        if (pipe_fd[i][0] > max_fd){
            max_fd = pipe_fd[i][0];
        }
    }

    while(!board->has_ended){
        FD_ZERO(&read_fds);
        for (int i = 0; i < num_players; i++){
            FD_SET(pipe_fd[i][0], &read_fds);
            
        }

        timeout.tv_sec = param_array[3];
        timeout.tv_usec = 0;
        ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == -1){
            perror("select");
            exit(EXIT_FAILURE);
        } else if(ready == 0){
            perror("Timeout");
            exit(EXIT_FAILURE);
        }

        
         for(int i = 0; i < num_players; i++){
            if(board->player_list[i].is_blocked){
                continue;
            }
            if(FD_ISSET(pipe_fd[i][0], &read_fds)){
                read(pipe_fd[i][0], &move, sizeof(unsigned char));
                sem_wait(&sync->game_state_mutex);
                sem_wait(&sync->master_mutex); //Bloqueo el acceso al board pq estoy modificando
                sem_post(&sync->game_state_mutex); //Desbloqueo el acceso al board pq termine de modificar

                if (!is_valid_move(board, &board->player_list[i], move, board->width, board->height)){ 
                    
                    board->player_list[i].ilegal_moves++;
                            
                } else{
                    move_player(board, &board->player_list[i], move, board->width, i);
                    
                }
                sem_post(&sync->master_mutex);
            }
            
            check_can_move(board, &board->player_list[i], board->width, board->height);      
            if(board->player_list[i].is_blocked){
                blocked_players++;
            }  
            
            if(view!=NULL){
                sem_post(&sync->changes); //Aviso que hubienro cambios en el board. 
                sem_wait(&sync->view_done); //Espero a que la vista termine de imprimir el board. 
                usleep(param_array[2]*1000); //Delay para que la vista pueda imprimir el board.
            }  
            if (blocked_players == num_players){ 
                board->has_ended = true;
                break;
            }          
        }

    }



    int status;
    if(view != NULL){
        waitpid(view_pid, &status , WNOHANG);
        if(WIFEXITED(status)){
            printf("View exited (%d)\n", WEXITSTATUS(status));
        }
    }
    for (int i = 0; i < num_players; i++)
    {

        pid_t pid = waitpid(board->player_list[i].pid, &status, 0);
        if (pid > 0) {
            if (WIFEXITED(status)) {
                printf("Player %d exited (%d) with a score of %d/%d/%d \n", i, WEXITSTATUS(status), board->player_list[i].points, board->player_list[i].valid_moves, board->player_list[i].ilegal_moves);
            }
        }
    }


    sem_destroy(&sync->changes);
    sem_destroy(&sync->view_done);
    sem_destroy(&sync->master_mutex);
    sem_destroy(&sync->game_state_mutex);
    sem_destroy(&sync->variable_mutex);

    munmap(board, sizeof(Board) + sizeof(int) * param_array[0] * param_array[1]);
    shm_unlink(SHM_NAME_BOARD);

    munmap(sync, sizeof(Sinchronization));
    shm_unlink(SHM_NAME_SYNC);

    return 0;

}





/*---------------------------------------------------------------------Functions----------------------------------------------------------------------------------------------------------*/


int set_params(int argc, char *argv[], int param_array[], char *player_array[], char **view) {
    int num_players = 0;
    for (int i = 0; i < argc && argv[i] != NULL; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            param_array[0] = atoi(argv[i + 1]);
            if (param_array[0] < 10) {
                fprintf(stderr, "Ancho invalido\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-h") == 0) {
            param_array[1] = atoi(argv[i + 1]);
            if (param_array[1] < 10) {
                fprintf(stderr, "Alto invalido\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            param_array[2] = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-t") == 0) {
            param_array[3] = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-s") == 0) {
            param_array[4] = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-v") == 0) {
            *view = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;
            for (; num_players < MAX_PLAYERS && i < argc; num_players++, i++) {
                if (argv[i] == NULL || !check_is_player(argv[i])) {
                    i--; //If the player is not valid, we go back one position to read it again, due to the fact that it is a flag or NULL, and we need to read it again. Or if we reached the end of the array.
                    break;
                }
                player_array[num_players] = argv[i];
            }
        }
    }

    return num_players;
}

bool check_is_player(char *player) {
    if (strcmp(player, "-w") == 0 || strcmp(player, "-h") == 0 || strcmp(player, "-d") == 0 || strcmp(player, "-t") == 0 || strcmp(player, "-s") == 0 || strcmp(player, "-v") == 0) {
        return false;
    }
    return true;
}

void print_game_vals(int param_array[], char * player_array[], char * view ,int num_players){
    printf("width: %d\n", param_array[0]); 
    printf("height: %d\n", param_array[1]);
    printf("delay: %d\n",param_array[2]);
    printf("timeout: %d\n",param_array[3]); 
    printf("seed: %d\n", param_array[4]);
    printf("view: %s\n", view);
    printf("num_players: %d\n", num_players); 
    for (int i = 0; i < num_players; i++){
        printf("Player %d: %s\n", i, player_array[i]);
    }
}

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

void initialize_sync(Sinchronization * sync){
    create_sem(&sync->changes, 1);
    create_sem(&sync->view_done, 0);
    create_sem(&sync->master_mutex, 1);
    create_sem(&sync->game_state_mutex, 1);
    create_sem(&sync->variable_mutex, 1);
    sync->readers_count = 0;  
}

void create_sem(sem_t * sem, int value){
    if (-1 == sem_init(sem, 1, value)){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

pid_t initialize_game(Board * board, int param_array[], char * player_array[], int num_players, char * view, int write_fd[][2]){
    board->width = param_array[0];
    board->height = param_array[1];
    board->num_players = num_players;
    board->has_ended = false;

    generate_board(board->board_pointer, board->width, board->height, param_array[4]);

    Point * points = generate_circle(board->width, board->height, num_players);

   

    char width[10];
    char height[10];


    sprintf(width,"%d", param_array[0]); 
    sprintf(height,"%d", param_array[1]);

    char * argv[4] = {NULL, width, height,NULL}; 

    for (int i = 0; i < num_players; i++){

        board->player_list[i].coord_x = points[i].x;
        board->player_list[i].coord_y = points[i].y; 
        strcpy(board->player_list[i].name, player_array[i]);
        board->player_list[i].points = 0;
        board->player_list[i].ilegal_moves = 0;
        board->player_list[i].valid_moves = 0;
        board->player_list[i].is_blocked = false;
        
        board->board_pointer[points[i].y * board->width + points[i].x] = (-1)*(i); 

        pid_t pid = fork();
        if (board->player_list[i].pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            board->player_list[i].pid = pid;
            close(write_fd[i][1]); // Cerrar el extremo de escritura del pipe en el padre
        }else {
            close(write_fd[i][0]); // Cerrar el extremo de lectura del pipe en el hijo
            dup2(write_fd[i][1], STDOUT_FILENO); // Redirigir el extremo de escritura al estándar de salida
            close(write_fd[i][1]);

            argv[0] = player_array[i];

            if (execve(player_array[i], argv ,NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);

            }
        }


    }
    
    free(points);

    
    if (view != NULL){
        pid_t pid = fork();
        if (pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            argv[0] = view;
            if (execve(view, argv, NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
        }
        return pid;
    }
    return -1;
}

void generate_board(int * board_pointer, int width, int height, int seed){
    srand(seed);
    for (int i = 0; i < width * height; i++){
        board_pointer[i] = rand() % 9 + 1;
    }
}

//PODRIAMOS MEJORAR ESTA FUNCION PARA QUE SEAN QUE LOS PUNTOS ESTEN MAS ESPACIADOS
Point * generate_circle(int width, int height, int num_points) {
    
    int center_x = width / 2;
    int center_y = height / 2;

    
    int radius = (width < height ? width : height) / 2 - 1;

    
    Point * points = (Point*) malloc(num_points * sizeof(Point));
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

void move_player(Board * board, Player * player, int move, int width, int player_number){  
    player->coord_x += movement[move][0];
    player->coord_y += movement[move][1];
    player->points += board->board_pointer[player->coord_y * width + player->coord_x];
    board->board_pointer[player->coord_y * width + player->coord_x] = (-1)*player_number;
    player->valid_moves++;
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