#include "Master_Lib.h"

#define PI 3.14159265358979323846 // We use this define, because M_PI isn't working. 

#define CAN_MOVE(x,y,width,height,new_pos)  (((x) >= 0 && (x) < (width)) && ((y) >= 0 && (y) < (height)) && ((new_pos) > 0))
#define READ_END 0 //Read end of the pipe
#define WRITE_END 1 //Write end of the pipe

const int movement[8][2] = {
    {0, -1},  // Up
    {1, -1},  // Up-Right
    {1, 0},   // Right
    {1, 1},    // Down-Right
    {0, 1},   // Down
    {-1, 1},  // Down-Left
    {-1, 0},  // Left
    {-1, -1}, // Up-Left
};

/*---------------------------------------------------------------------Functions----------------------------------------------------------------------------------------------------------*/



int set_params(int argc, char *argv[], int param_array[], char *player_array[], char **view) {
    int num_players = 0;
    for (int i = 0; i < argc && argv[i] != NULL; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            param_array[0] = atoi(argv[i + 1]);
            if (param_array[0] < 10) {
                fprintf(stderr, "Invalid width\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-h") == 0) {
            param_array[1] = atoi(argv[i + 1]);
            if (param_array[1] < 10) {
                fprintf(stderr, "Invalid height\n");
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
            for (;i < argc; num_players++, i++) {
                if (num_players > MAX_PLAYERS) {
                    fprintf(stderr, "Invalid amount of players\n");
                    exit(EXIT_FAILURE);
                }
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

void initialize_sync(Sinchronization * sync){
    create_sem(&sync->changes, 0);
    create_sem(&sync->view_done, 0);
    create_sem(&sync->master_mutex, 1);
    create_sem(&sync->game_state_mutex, 1);
    create_sem(&sync->variable_mutex, 1);
    sync->readers_count = 0;  
}

pid_t initialize_game(Board * board, int param_array[], char * player_array[], int num_players, char * view, int write_fd[][2]){
    board->width = param_array[0];
    board->height = param_array[1];
    board->num_players = num_players;
    board->has_ended = false;

    generate_board(board->board_pointer, board->width, board->height, param_array[4]);

    Point points[MAX_PLAYERS];

    generate_circle(board->width, board->height, num_players,points);

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
            close(write_fd[i][WRITE_END]); 
        }else {
            for (int j = 0; j < num_players; j++) {
                if (j != i) {
                    close(write_fd[j][READ_END]); 
                    close(write_fd[j][WRITE_END]); 
                }
            }
            close(write_fd[i][READ_END]); 
            dup2(write_fd[i][WRITE_END], STDOUT_FILENO); 
            close(write_fd[i][WRITE_END]);

            argv[0] = player_array[i];

            if (execve(player_array[i], argv ,NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);

            }
        }


    }
    
    pid_t pid;
    
    if (view != NULL){
        pid = fork();
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
    }

    return pid; 
}

void generate_board(int * board_pointer, int width, int height, int seed){
    srand(seed);
    for (int i = 0; i < width * height; i++){
        board_pointer[i] = rand() % 9 + 1;
    }
}

Point * generate_circle(int width, int height, int num_points, Point  * points) {
    
    int center_x = width / 2;
    int center_y = height / 2;


    if(num_points == 1){
        points[0].x = center_x;
        points[0].y = center_y;
    } else{
        double angle_increment = 2 * PI / num_points;
        for (int i = 0; i < num_points; i++) {
            double angle = i * angle_increment;
            points[i].x = (int)(center_x + (width / 3) * cos(angle));
            points[i].y = (int)(center_y + (height / 3) * sin(angle));
        }
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

void create_sem(sem_t * sem, int value){
    if (-1 == sem_init(sem, 1, value)){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}