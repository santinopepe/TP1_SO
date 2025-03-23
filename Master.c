#include "Utilis.h"


#define PI 3.14159265358979323846



void * create_shm(char * name, int size){

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


bool can_move(int x, int y, int width, int height, Board *board){
    return x >= 0 && x < width && y >= 0 && y < height && board->board_pointer[y * width + x] != -1;
  }
  
  int is_valid_move(Board *board, Player *player, int move, int width, int height){ 
    int x = player->coord_x;
    int y = player->coord_y;
  
    int movement[8][2] ={
      {0, 1},
      {1, 0},
      {0, -1},
      {-1, 0},
      {1, 1},
      {1, -1},
      {-1, 1},
      {-1, -1}
    };
  
    x += movement[move][0];
    y += movement[move][1];
  
    if(!can_move(x, y, width, height, board)){
      int i =0;
      while (i < 8) //si no es posibe con el numero random encontrar un mov valido lo hace a la fuerza
      {
        if(player->coord_x + movement[i][0] >= 0 && player->coord_x + movement[i][0] < width && player->coord_y + movement[i][1] >= 0 && player->coord_y + movement[i][1] < height && board->board_pointer[(player->coord_y + movement[i][1]) * width + player->coord_x + movement[i][0]] != -1){
          x = player->coord_x + movement[i][0];
          y = player->coord_y + movement[i][1];
          break;
        }
        i++;
      }
      if (i == 8)
      {
        player->is_bolcked = true; //NO SE SI ES QUE ESTO SE MODIFICA ACA O SE HACE SOLO EN EL MASTER
        return -1;
      }
    }
    player->coord_x = x; //NO SE SI ES QUE SE MODIFICA ACA O DONDE SE HACE
    player->coord_y = y;
    return 0;
  }
  

void create_sem(sem_t * sem, int value){
    if (-1 == sem_init(sem, 1, value)){ 
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void initialize_board(Board * board){
    board->width = 10;
    board->hight = 10;
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
        } else if (strcmp(argv[i], "-h") == 0){
            param_array[1] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-d") == 0){
            param_array[2] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-t") == 0){
            param_array[3] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-s") == 0){
            param_array[4] = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-v") == 0){
            (*view) = argv[i+1];
        } else if(strcmp(argv[i], "-p")==0){
            for (; num_players < MAX_PLAYERS || i < argc; num_players++, i++){
                if (check_is_player(argv[i+num_players+1])){ 
                    player_array[num_players] = argv[i+num_players+1];
                }
            }
        }     
    }
    if(num_players == 0){
        printf("No se ingresaron jugadores\n");
        exit(EXIT_FAILURE);
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

void initialize_game(int param_array[], char * player_array[], char * view, Board * board, int num_players){
    board->width = param_array[0];
    board->hight = param_array[1];
    board->num_players = num_players;

    Point * points = generate_circle(board->width, board->hight, num_players);

    int player_it = 0;
    while(player_it < num_players || player_array[player_it]==NULL){
        
        strcpy(board->player_list[player_it].name, player_array[player_it]);
        board->player_list[player_it].points = 0;
        board->player_list[player_it].iligal_moves = 0;
        board->player_list[player_it].valid_moves = 0;
        board->player_list[player_it].coord_x = points[player_it].x;
        board->player_list[player_it].coord_y = points[player_it].y;
        board->player_list[player_it].is_bolcked = true;

        board->player_list[player_it].pid = fork(); //CHECKEAR SI ESTO ESTA BIEN
        if (board->player_list[player_it].pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (board->player_list[player_it].pid == 0){
            execve(player_array[player_it], NULL, NULL);
        }

        player_it++;
    }

    board->has_ended = false;

    srand(time(NULL));
    // Memory for board_pointer is already allocated with the Board structure
    for (int i = 0; i < board->width * board->hight; i++) {
        board->board_pointer[i] = rand() % 9 + 1; // Initialize board cells to 0
    }

    free(points);

    
}

void initialize_sync(Sinchronization * sync, int num_players){
    create_sem(&sync->changes, 0);
    create_sem(&sync->view_done, 0);
    create_sem(&sync->master_mutex, 1);
    create_sem(&sync->game_state_mutex, 1);
    create_sem(&sync->variable_mutex, 1);
    sync->readers_count = num_players;
}

int main(int argc, char * argv[]) {
    Board * board = (Board *) create_shm(SHM_NAME_BOARD, sizeof(Board));
    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization));
    
    int param_array[5] ={10, 10, 200, 10, time(NULL)}; 
    char * view = NULL;
    char * player_array[9]={NULL};

    int num_players = get_param(argc, argv, param_array, player_array, view);

    initialize_game(param_array, player_array, view, board, num_players);
    
    initialize_sync(sync, num_players);





    return 0;

}