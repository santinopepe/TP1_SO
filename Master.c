#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

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

sem_t * create_sem(char * name, int value){
    sem_t * sem =  (sem_t * ) create_shm(name, sizeof(sem_t));
    if (-1 == sem_init(sem, 1, 0)){ {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}



int main(int argc, char * argv[]) {

    

}