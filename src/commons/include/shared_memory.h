#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <windows.h>

#include "error_handling.h"
#include "passed_action.h"
#include "assert.h"


#define SHMEM_TO_KONC4D 0
#define SHMEM_FROM_KONC4D 1

#define SHMEM_QUEUE_SIZE 256

#define SHMEM_EMBEDDED_UNSIGNED(message, position) *((unsigned*) &message[position])
#define NO_MESSAGE -1


struct SharedMemoryFile
{
    HANDLE mapFile, mutex;
    struct SharedMemory *shared;
};


struct SharedMemory
{
    int queueFirst, queueLast;
    char messageQueue[SHMEM_QUEUE_SIZE];
};


ReturnCode isKonc4dOn(void);
ReturnCode createSharedMemory(struct SharedMemoryFile *sharedMemory, unsigned pipeNr) NO_IGNORE;
ReturnCode openSharedMemory(struct SharedMemoryFile *sharedMemory, unsigned pipeNr) NO_IGNORE;
ReturnCode sendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned size) NO_IGNORE;
ReturnCode receiveMessage(struct SharedMemoryFile sharedMemory, char **buffer, unsigned *size) NO_IGNORE;
void closeSharedMemory(struct SharedMemoryFile sharedMemory);

#endif
