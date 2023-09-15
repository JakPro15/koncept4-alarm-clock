#include "action_receiving.h"
#include "logging.h"

#define SHMEM_DELAY 100
#define SHMEM_TIMEOUT 5000
#define SHMEM_TICKS_TIMEOUT SHMEM_TIMEOUT / SHMEM_DELAY


ReturnCode timeoutReceive(struct SharedMemoryFile sharedMemory, char *toWrite)
{
    ReturnCode received;
    RETHROW(received = receiveMessage(sharedMemory, toWrite));
    unsigned ticks = 0;
    while(received == RET_FAILURE && ticks++ < SHMEM_TICKS_TIMEOUT)
    {
        Sleep(SHMEM_DELAY);
        RETHROW(received = receiveMessage(sharedMemory, toWrite));
    }
    if(ticks == SHMEM_TICKS_TIMEOUT)
    {
        LOG_LINE(LOG_ERROR, "Receiving timed out");
        return RET_FAILURE;
    }
    LOG_LINE(LOG_TRACE, "Received message: %s", toWrite);
    return RET_SUCCESS;
}


ReturnCode obtainActions(struct PassedAction **toWrite, unsigned *size)
{
    struct SharedMemoryFile actionTransferQueue;
    ENSURE(createSharedMemory(&actionTransferQueue, SHMEM_FROM_KONC4D));
    ENSURE(fullSendMessage("SEND"));
    char sizeMessage[SHMEM_MESSAGE_LENGTH];
    ENSURE(timeoutReceive(actionTransferQueue, sizeMessage));
    if(strcmp(sizeMessage, "SIZE") != 0)
    {
        LOG_LINE(LOG_ERROR, "Did not receive size message in response to actions request");
        closeSharedMemory(actionTransferQueue);
        return RET_ERROR;
    }
    *size = SHMEM_EMBEDDED_UNSIGNED(sizeMessage);
    *toWrite = malloc(*size * sizeof(**toWrite));
    for(unsigned i = 0; i < *size; i++)
    {
        char actionMessage[SHMEM_MESSAGE_LENGTH];
        ENSURE_CALLBACK(timeoutReceive(actionTransferQueue, actionMessage), closeSharedMemory(actionTransferQueue); free(*toWrite));
        memcpy(&(*toWrite)[i], actionMessage, sizeof(**toWrite));
    }
    closeSharedMemory(actionTransferQueue);
    return RET_SUCCESS;
}
