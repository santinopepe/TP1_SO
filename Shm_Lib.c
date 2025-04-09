#include "Shm_Lib.h"


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


void * open_shm(char * name, int size, int flags){
    
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

void close_shm(void * mem, int size){
    if (munmap(mem, size) == -1) {
        perror("munmap");
    }
}



void delete_shm(char * name, void * mem, int size){
    close_shm(mem, size);
    
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
    }
}