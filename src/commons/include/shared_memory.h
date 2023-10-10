#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <windows.h>
#include <stdint.h>

#include "error_handling.h"
#include "passed_action.h"


#define SHMEM_TO_KONC4D 0
#define SHMEM_FROM_KONC4D 1

#define SHMEM_QUEUE_SIZE 256

#define SHMEM_EMBEDDED_UINT64(message, position) (*((uint64_t*) &(message)[(position)]))
#define NO_MESSAGE -1


struct SharedMemoryFile
{
    HANDLE mapFile, mutex, writtenEvent, readEvent;
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
ReturnCode sendSizedMessage(struct SharedMemoryFile sharedMemory, const char *message, unsigned size) NO_IGNORE;
ReturnCode receiveSizedMessage(struct SharedMemoryFile sharedMemory, char **buffer, unsigned *size) NO_IGNORE;
void closeSharedMemory(struct SharedMemoryFile sharedMemory);

#define NO_WAIT 0
#define INFINITE_WAIT UINT_MAX

ReturnCode timeoutSendSizedMessage(struct SharedMemoryFile sharedMemory, const char *message,
                                   unsigned size, unsigned timeout) NO_IGNORE;
ReturnCode sendMessage(struct SharedMemoryFile sharedMemory, const char *message, unsigned timeout) NO_IGNORE;
ReturnCode sendMessageWithArgument(struct SharedMemoryFile sharedMemory, const char *message,
                                   uint64_t argument, unsigned timeout) NO_IGNORE;

ReturnCode timeoutReceiveSizedMessage(struct SharedMemoryFile sharedMemory, char **toWrite,
                                      unsigned *size, unsigned timeout) NO_IGNORE;
ReturnCode receiveMessage(struct SharedMemoryFile sharedMemory, char **message, unsigned timeout) NO_IGNORE;
ReturnCode receiveMessageWithArgument(struct SharedMemoryFile sharedMemory, char **message,
                                      uint64_t *argument, unsigned timeout) NO_IGNORE;

#endif
