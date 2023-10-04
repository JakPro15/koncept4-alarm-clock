#include "action_receiving.h"
#include "logging.h"

#define SHMEM_DELAY 100
#define SHMEM_TIMEOUT 5000
#define SHMEM_TICKS_TIMEOUT SHMEM_TIMEOUT / SHMEM_DELAY


ReturnCode timeoutReceive(struct SharedMemoryFile sharedMemory, char **toWrite, unsigned *size)
{
    ReturnCode received;
    RETHROW(received = receiveMessage(sharedMemory, toWrite, size));
    unsigned ticks = 0;
    while(received == RET_FAILURE && ticks++ < SHMEM_TICKS_TIMEOUT)
    {
        Sleep(SHMEM_DELAY);
        RETHROW(received = receiveMessage(sharedMemory, toWrite, size));
    }
    if(ticks == SHMEM_TICKS_TIMEOUT)
    {
        LOG_LINE(LOG_ERROR, "Receiving timed out");
        return RET_FAILURE;
    }
    LOG_LINE(LOG_TRACE, "Received message: %s", *toWrite);
    return RET_SUCCESS;
}


ReturnCode obtainActions(struct PassedAction **toWrite, unsigned *size)
{
    struct SharedMemoryFile actionTransferQueue;
    ENSURE(createSharedMemory(&actionTransferQueue, SHMEM_FROM_KONC4D));
    ENSURE(fullSendMessage("SEND"));
    char *sizeMessage;
    unsigned sizeMessageSize;
    ENSURE(timeoutReceive(actionTransferQueue, &sizeMessage, &sizeMessageSize));
    if(sizeMessageSize < sizeof("SIZE") + sizeof("unsigned"))
    {
        LOG_LINE(LOG_ERROR, "Received SIZE message too short");
        closeSharedMemory(actionTransferQueue);
        return RET_ERROR;
    }
    if(strcmp(sizeMessage, "SIZE") != 0)
    {
        LOG_LINE(LOG_ERROR, "Did not receive size message in response to actions request");
        closeSharedMemory(actionTransferQueue);
        free(sizeMessage);
        return RET_ERROR;
    }
    *size = SHMEM_EMBEDDED_UNSIGNED(sizeMessage, sizeof("SIZE"));
    *toWrite = malloc(*size * sizeof(**toWrite));
    for(unsigned i = 0; i < *size; i++)
    {
        char *actionMessage;
        unsigned actionMessageSize;
        ENSURE_CALLBACK(timeoutReceive(actionTransferQueue, &actionMessage, &actionMessageSize),
                        closeSharedMemory(actionTransferQueue); free(*toWrite));
        if(actionMessageSize < sizeof(struct PassedAction))
        {
            LOG_LINE(LOG_ERROR, "Received PassedAction message too short");
            closeSharedMemory(actionTransferQueue);
            return RET_ERROR;
        }
        memcpy(&(*toWrite)[i], actionMessage, sizeof(**toWrite));
        free(actionMessage);
    }
    closeSharedMemory(actionTransferQueue);
    return RET_SUCCESS;
}
