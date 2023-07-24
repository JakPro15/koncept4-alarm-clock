#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <windows.h>

#include "error_handling.h"


#define SHMEM_FILE_NAME "konc4_shared_memory"
#define SHMEM_MUTEX_NAME "konc4_shared_memory_mutex"
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


ReturnCode isKonc4dOn(void);
ReturnCode createSharedMemory(struct SharedMemoryFile *sharedMemory) NO_IGNORE;
ReturnCode openSharedMemory(struct SharedMemoryFile *sharedMemory) NO_IGNORE;
ReturnCode sendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned length) NO_IGNORE;
ReturnCode receiveMessage(struct SharedMemoryFile sharedMemory, char *buffer) NO_IGNORE;
void closeSharedMemory(struct SharedMemoryFile sharedMemory);

#endif
