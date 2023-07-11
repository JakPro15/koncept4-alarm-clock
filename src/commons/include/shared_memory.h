#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <windows.h>

#include "error_handling.h"


#define SHMEM_FILE_NAME "konc4_shared_memory"
#define MUTEX_NAME "konc4d_shared_memory_mutex"
#define SHMEM_QUEUE_LENGTH 8
#define SHMEM_MESSAGE_LENGTH 12
#define NO_NODE -1


struct SharedMemoryFile
{
    HANDLE mapFile, mutex;
    struct SharedMemory *shared;
};


struct SharedMemory
{
    int queueFirst, queueLast;
    char messageQueue[SHMEM_QUEUE_LENGTH][SHMEM_MESSAGE_LENGTH];
};


ReturnCode createSharedMemory(struct SharedMemoryFile *sharedMemory);
ReturnCode openSharedMemory(struct SharedMemoryFile *sharedMemory);
ReturnCode sendMessage(struct SharedMemoryFile *sharedMemory, char *message);
ReturnCode receiveMessage(struct SharedMemoryFile *sharedMemory, char *buffer);
void closeSharedMemory(struct SharedMemoryFile sharedMemory);

#endif
