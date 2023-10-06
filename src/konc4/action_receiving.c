#include "action_receiving.h"
#include "logging.h"
#include "events.h"

#define SHMEM_DELAY 500
#define SHMEM_TIMEOUT 5000
#define SHMEM_TICKS_TIMEOUT SHMEM_TIMEOUT / SHMEM_DELAY


ReturnCode timeoutReceive(struct SharedMemoryFile sharedMemory, char **toWrite, unsigned *size)
{
    ReturnCode received;
    RETHROW(received = receiveMessage(sharedMemory, toWrite, size));
    if(received == RET_FAILURE)
    {
        RETHROW(received = waitOnEventObject(sharedMemory.writtenEvent, SHMEM_TIMEOUT));
        if(received == RET_FAILURE)
        {
            LOG_LINE(LOG_ERROR, "Receiving timed out");
            return RET_FAILURE;
        }
        ENSURE(receiveMessage(sharedMemory, toWrite, size));
    }
    LOG_LINE(LOG_TRACE, "Received message: %s", *toWrite);
    return RET_SUCCESS;
}


static ReturnCode obtainActionVectorSize(struct SharedMemoryFile sharedMemory, unsigned *size)
{
    char *sizeMessage;
    unsigned sizeMessageSize;
    ENSURE(timeoutReceive(sharedMemory, &sizeMessage, &sizeMessageSize));
    if(sizeMessageSize < sizeof("SIZE") + sizeof(unsigned))
    {
        LOG_LINE(LOG_ERROR, "Received SIZE message too short");
        free(sizeMessage);
        return RET_ERROR;
    }
    if(strcmp(sizeMessage, "SIZE") != 0)
    {
        LOG_LINE(LOG_ERROR, "Did not receive size message in response to actions request");
        free(sizeMessage);
        return RET_ERROR;
    }
    *size = SHMEM_EMBEDDED_UNSIGNED(sizeMessage, sizeof("SIZE"));
    free(sizeMessage);
    LOG_LINE(LOG_DEBUG, "Received action vector size: %u", *size);
    return RET_SUCCESS;
}


static ReturnCode obtainActionVector(struct SharedMemoryFile sharedMemory, struct PassedAction **toWrite, unsigned size)
{
    *toWrite = malloc(size * sizeof(**toWrite));
    if(*toWrite == NULL)
    {
        LOG_LINE(LOG_ERROR, "Memory allocation failure");
        return RET_ERROR;
    }
    for(unsigned i = 0; i < size; i++)
    {
        char *actionMessage;
        unsigned actionMessageSize;
        ENSURE_CALLBACK(timeoutReceive(sharedMemory, &actionMessage, &actionMessageSize), free(*toWrite));
        if(actionMessageSize < sizeof(struct PassedAction))
        {
            LOG_LINE(LOG_ERROR, "Received PassedAction message too short");
            free(actionMessage);
            free(*toWrite);
            return RET_ERROR;
        }
        memcpy(&(*toWrite)[i], actionMessage, sizeof(**toWrite));
        free(actionMessage);
    }
    LOG_LINE(LOG_DEBUG, "Received action vector");
    return RET_SUCCESS;
}


static ReturnCode obtainActionClocks(struct SharedMemoryFile sharedMemory, struct ActionClock *shutdownClock)
{
    char *clockMessage;
    unsigned clockMessageSize;
    ENSURE(timeoutReceive(sharedMemory, &clockMessage, &clockMessageSize));
    if(clockMessageSize < sizeof(*shutdownClock))
    {
        LOG_LINE(LOG_ERROR, "Received action clock message too short");
        free(clockMessage);
        return RET_ERROR;
    }
    memcpy((char*) shutdownClock, clockMessage, sizeof(*shutdownClock));
    free(clockMessage);
    LOG_LINE(LOG_DEBUG, "Received shutdown action clock");
    return RET_SUCCESS;
}


static ReturnCode obtainClockCooldown(struct SharedMemoryFile sharedMemory, unsigned *clockCooldown)
{
    char *cooldownMessage;
    unsigned cooldownMessageSize;
    ENSURE(timeoutReceive(sharedMemory, &cooldownMessage, &cooldownMessageSize));
    if(cooldownMessageSize < sizeof(*clockCooldown))
    {
        LOG_LINE(LOG_ERROR, "Received action clock message too short");
        free(cooldownMessage);
        return RET_ERROR;
    }
    if(strcmp(cooldownMessage, "COOLDOWN") != 0)
    {
        LOG_LINE(LOG_ERROR, "Did not receive cooldown message when expected");
        free(cooldownMessage);
        return RET_ERROR;
    }
    *clockCooldown = SHMEM_EMBEDDED_UNSIGNED(cooldownMessage, sizeof("COOLDOWN"));
    free(cooldownMessage);
    LOG_LINE(LOG_DEBUG, "Received clock cooldown: %u", *clockCooldown);
    return RET_SUCCESS;
}


ReturnCode obtainActions(struct ReceivedActions *toWrite)
{
    struct SharedMemoryFile actionTransferQueue;
    ENSURE(createSharedMemory(&actionTransferQueue, SHMEM_FROM_KONC4D));
    ENSURE(fullSendMessage("SEND"));

    ENSURE_CALLBACK(obtainActionVectorSize(actionTransferQueue, &toWrite->actionVectorSize), closeSharedMemory(actionTransferQueue));
    ENSURE_CALLBACK(obtainActionVector(actionTransferQueue, &toWrite->actionVector, toWrite->actionVectorSize),
                    closeSharedMemory(actionTransferQueue));
    ENSURE_CALLBACK(obtainActionClocks(actionTransferQueue, &toWrite->shutdownClock),
                    closeSharedMemory(actionTransferQueue); free(toWrite->actionVector));
    ENSURE_CALLBACK(obtainClockCooldown(actionTransferQueue, &toWrite->clockCooldown),
                    closeSharedMemory(actionTransferQueue); free(toWrite->actionVector));

    closeSharedMemory(actionTransferQueue);
    return RET_SUCCESS;
}
