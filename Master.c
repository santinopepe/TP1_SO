
#include "Master_Lib.h"
#include "Shm_Structs.h" 


#define TIMEOUT 3   //Timeout index
#define READ_END 0 //Read end of the pipe 
#define SLEEP 2 //Sleep index
#define INITIAL_SLEEP 1 //Initial sleep
#define TO_MILI_SEC 1000 //Conversion form microseconds to milliseconds



int main(int argc, char *argv[]) {


    int param_array[5] = {10, 10, 200, 10, time(NULL)};
    char *player_array[9] = {NULL};
    char *view = NULL;
    int pipe_fd[MAX_PLAYERS][2]; 

    int num_players = set_params(argc, argv, param_array, player_array, &view);

    print_game_vals(param_array, player_array, view, num_players);

    Board * board = (Board *) create_shm(SHM_NAME_BOARD, sizeof(Board) + sizeof(int)*param_array[0]*param_array[1]); 

    Sinchronization * sync = (Sinchronization *) create_shm(SHM_NAME_SYNC, sizeof(Sinchronization));
    
    initialize_sync(sync);
    int max_fd=0;
    for (int i = 0; i < num_players; i++){
        if (pipe(pipe_fd[i]) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        
    }

    pid_t viewPid =  initialize_game(board, param_array, player_array, num_players, view, pipe_fd);

    fd_set read_fds;
    unsigned char move;
    int blocked_players = 0;
    int ready; 
    struct timeval timeout, current_time, last_valid_move_time;


    for(int i = 0; i < num_players; i++){
        if (pipe_fd[i][READ_END] > max_fd){
            max_fd = pipe_fd[i][READ_END];
        }
    }
    sleep(INITIAL_SLEEP); //Delay to let the players start before the game starts.



    // This variables are used to give equal distribution of priority to the players. 
    // Without this, the players that are in the first positions will always play first.
    int index =0;
    int start = 0;
    gettimeofday(&last_valid_move_time, NULL); 
    while(!board->has_ended){
        
        FD_ZERO(&read_fds);
        for (int i = 0; i < num_players; i++){
            FD_SET(pipe_fd[i][READ_END], &read_fds);
            
        }

        timeout.tv_sec = param_array[TIMEOUT];
        timeout.tv_usec = 0;

        ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == -1){
            perror("select");
            exit(EXIT_FAILURE);
        } else if(ready == 0){
            perror("Timeout");
            break; //Exit the game if timeout occurs. So we can cleanup the memory an pipes correctly 
        }

        gettimeofday(&current_time, NULL);
        long elapsed_time = (current_time.tv_sec - last_valid_move_time.tv_sec) * 1000 +
                            (current_time.tv_usec - last_valid_move_time.tv_usec) / 1000;
 
        if (elapsed_time > param_array[TIMEOUT] * TO_MILI_SEC) {
            board->has_ended = true;
            break;
        }
        
        for(int i = 0; i < num_players; i++){
            index = (start + i) % num_players;
            if(board->player_list[index].is_blocked){
                continue;
            }
            if(FD_ISSET(pipe_fd[index][READ_END], &read_fds)){
                read(pipe_fd[index][READ_END], &move, sizeof(unsigned char));

                sem_wait(&sync->game_state_mutex);
                sem_wait(&sync->master_mutex); 
                sem_post(&sync->game_state_mutex); 

                if (!is_valid_move(board, &board->player_list[index], move, board->width, board->height)){ 
                    
                    board->player_list[index].ilegal_moves++;
                } else{
                    gettimeofday(&last_valid_move_time, NULL);
                    move_player(board, &board->player_list[index], move, board->width, index);
                     

                    // This forloop is used to check if the player that moved blocked another player and it also checks if the player that moved is blocked.
                    for(int j = 0; j < num_players; j++){
                        if (board->player_list[j].is_blocked){
                            continue;
                        }
                        check_can_move(board, &board->player_list[j], board->width, board->height);      
                        if(board->player_list[j].is_blocked){
                            blocked_players++;
                        } 
                    }
                }
                sem_post(&sync->master_mutex);
            }
            
            if (blocked_players == num_players){ 
                board->has_ended = true;
                break;
            } 

            if(view!=NULL){
                sem_post(&sync->changes); 
                sem_wait(&sync->view_done); 
                usleep(param_array[SLEEP]*TO_MILI_SEC); // Sleep to let the view print the state of the game
            }  
                     
        }

        start++;

    }

    int status;
    pid_t pid;

    if(view != NULL){
        sem_post(&sync->changes); 
        pid = waitpid(viewPid,&status, 0); 
        printf("View exited (%d)\n", WEXITSTATUS(status)); 
    }
    

    for (int i = 0; i < num_players; i++)
    {
        pid = waitpid(board->player_list[i].pid, &status, 0);
        if (pid >= 0) {
            printf("Player %s (%d) exited (%d) with a score of %d/%d/%d \n", board->player_list[i].name, i, WEXITSTATUS(status), board->player_list[i].points, board->player_list[i].valid_moves, board->player_list[i].ilegal_moves);
        
        }
        close(pipe_fd[i][READ_END]); // Close the read end of the pipe
    }


    sem_destroy(&sync->changes);
    sem_destroy(&sync->view_done);
    sem_destroy(&sync->master_mutex);
    sem_destroy(&sync->game_state_mutex);
    sem_destroy(&sync->variable_mutex);

    
    delete_shm(SHM_NAME_BOARD, board, sizeof(Board) + sizeof(int)*param_array[0]*param_array[1]);

    delete_shm(SHM_NAME_SYNC, sync, sizeof(Sinchronization));

    return 0;

}


