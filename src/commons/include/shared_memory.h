#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <windows.h>

#include "error_handling.h"
#include "passed_action.h"
#include "assert.h"


#define SHMEM_TO_KONC4D 0
#define SHMEM_FROM_KONC4D 1

#define SHMEM_QUEUE_LENGTH 8
#define SHMEM_MESSAGE_LENGTH (unsigned) sizeof(struct PassedAction)
static_assert(SHMEM_MESSAGE_LENGTH >= 12, "SHMEM_MESSAGE_LENGTH smaller than was assumed");

#define SHMEM_EMBEDDED_UNSIGNED(message) *((unsigned*) &message[SHMEM_MESSAGE_LENGTH - sizeof(unsigned)])
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
ReturnCode createSharedMemory(struct SharedMemoryFile *sharedMemory, unsigned pipeNr) NO_IGNORE;
ReturnCode openSharedMemory(struct SharedMemoryFile *sharedMemory, unsigned pipeNr) NO_IGNORE;
ReturnCode sendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned length) NO_IGNORE;
ReturnCode receiveMessage(struct SharedMemoryFile sharedMemory, char *buffer) NO_IGNORE;
void closeSharedMemory(struct SharedMemoryFile sharedMemory);

#endif
