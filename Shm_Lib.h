#ifndef SHM_LIB_H
#define SHM_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * @brief Create a shared memory segment.
 * @param name The name of the shared memory segment.
 * @param size The size of the shared memory segment.
 * @param flags The flags to open the shared memory segment.
 * @return A pointer to the shared memory segment.
*/
void * create_shm(char * name, int size);

/**
 * @brief Open a shared memory segment.
 * @param name The name of the shared memory segment.
 * @param size The size of the shared memory segment.
 * @param flags The flags to open the shared memory segment.
*/
void * open_shm(char * name, int size, int flags);

/**
 * @brief Close a shared memory segment.
 * @param mem The pointer to the shared memory segment.
 * @param size The size of the shared memory segment.
*/
void close_shm(void * mem, int size);

/**
 * @brief Delete a shared memory segment.
 * @param name The name of the shared memory segment.
 * @param mem The pointer to the shared memory segment.
 * @param size The size of the shared memory segment.
*/
void delete_shm(char * name, void * mem, int size);


#endif // SHM_LIB_H
