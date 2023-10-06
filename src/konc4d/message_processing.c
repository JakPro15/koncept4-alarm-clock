#include "message_processing.h"
#include "logging.h"
#include "shared_memory.h"

#define SHMEM_DELAY 100
#define SHMEM_TIMEOUT 5000
#define SHMEM_TICKS_TIMEOUT SHMEM_TIMEOUT / SHMEM_DELAY


ReturnCode sendItem(struct SharedMemoryFile sharedMemory, char *item, unsigned size)
{
    ReturnCode sent;
    RETHROW(sent = sendMessage(sharedMemory, item, size));
    unsigned ticks = 0;
    while(sent == RET_FAILURE && ticks++ < SHMEM_TICKS_TIMEOUT)
    {
        Sleep(SHMEM_DELAY);
        RETHROW(sent = sendMessage(sharedMemory, item, size));
    }
    if(ticks == SHMEM_TICKS_TIMEOUT)
    {
        LOG_LINE(LOG_WARNING, "Sending action or action clock timed out");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}


ReturnCode sendAction(struct SharedMemoryFile sharedMemory, struct Action *toSend)
{
    struct PassedAction sentAction = getPassedAction(toSend);
    LOG_LINE(LOG_TRACE, "Sending action: {%02d.%02d %02d:%02d, type: %d}",
             toSend->timestamp.date.day, toSend->timestamp.date.month,
             toSend->timestamp.time.hour, toSend->timestamp.time.minute, toSend->type);
    return sendItem(sharedMemory, (char*) &sentAction, sizeof(sentAction));
}


static unsigned countActions(struct ActionQueue *actions)
{
    unsigned result = 0;
    for(; actions != NULL; actions = actions->next)
        ++result;
    return result;
}


ReturnCode actionTransfer(struct AllActions *actions)
{
    struct SharedMemoryFile actionTransferQueue;
    ENSURE(openSharedMemory(&actionTransferQueue, SHMEM_FROM_KONC4D));

    unsigned noActions = countActions(actions->queueHead);
    LOG_LINE(LOG_DEBUG, "Sending SIZE %d message", noActions);
    unsigned sizeMessageSize = sizeof("SIZE") + sizeof(unsigned);
    char sizeMessage[sizeMessageSize];
    strcpy(sizeMessage, "SIZE");
    SHMEM_EMBEDDED_UNSIGNED(sizeMessage, sizeof("SIZE")) = noActions;
    ENSURE_CALLBACK(sendMessage(actionTransferQueue, sizeMessage, sizeMessageSize), closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_DEBUG, "Sending actions");
    struct ActionQueue *queue = actions->queueHead;
    for(; queue != NULL; queue = queue->next)
        RETURN_FAIL_CALLBACK(sendAction(actionTransferQueue, &queue->action), closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_DEBUG, "Sending shutdown action clock");
    RETURN_FAIL_CALLBACK(sendItem(actionTransferQueue, (char*) &actions->shutdownClock, sizeof(actions->shutdownClock)),
                         closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_DEBUG, "Sending action clock cooldown: %u", actions->clockCooldown);
    unsigned cooldownMessageSize = sizeof("COOLDOWN") + sizeof(unsigned);
    char cooldownMessage[cooldownMessageSize];
    strcpy(cooldownMessage, "COOLDOWN");
    SHMEM_EMBEDDED_UNSIGNED(cooldownMessage, sizeof("COOLDOWN")) = actions->clockCooldown;
    RETURN_FAIL_CALLBACK(sendItem(actionTransferQueue, cooldownMessage, cooldownMessageSize),
                         closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_INFO, "Successfully finished actions transfer");
    closeSharedMemory(actionTransferQueue);
    return RET_SUCCESS;
}


extern bool message_exit;


ReturnCode processMessage(struct AllActions *actions, char *message)
{
    if(strcmp(message, "RESET") == 0)
    {
        LOG_LINE(LOG_INFO, "RESET message received, resetting");
        return RET_FAILURE;
    }
    else if(strcmp(message, "STOP") == 0)
    {
        LOG_LINE(LOG_INFO, "STOP message received, stopping");
        message_exit = true;
        return RET_ERROR;
    }
    else if(strcmp(message, "SKIP") == 0)
    {
        unsigned minutesToSkip = SHMEM_EMBEDDED_UNSIGNED(message, sizeof("SKIP"));
        struct YearTimestamp now = getCurrentTimestamp();
        struct YearTimestamp until = addMinutes(now, minutesToSkip);
        LOG_LINE(LOG_INFO, "SKIP message received, skipping by %u minutes", minutesToSkip);
        ENSURE(skipUntilTimestamp(&actions->queueHead, until.timestamp, now));
        actions->clockCooldown = difference(now, until) * SECONDS_IN_MINUTE;
        LOG_LINE(LOG_DEBUG, "Action clock actions disabled for %u seconds", actions->clockCooldown);
        return RET_SUCCESS;
    }
    else if(strcmp(message, "SEND") == 0)
    {
        LOG_LINE(LOG_INFO, "SEND message received, sending");
        RETHROW(actionTransfer(actions));
        return RET_SUCCESS;
    }
    else
    {
        LOG_LINE(LOG_ERROR, "Unknown message received");
        return RET_ERROR;
    }
}


ReturnCode handleMessages(struct AllActions *actions, struct SharedMemoryFile sharedMemory)
{
    char *message;
    unsigned size;
    ReturnCode received;
    RETHROW(received = receiveMessage(sharedMemory, &message, &size));
    while(received != RET_FAILURE)
    {
        RETURN_FAIL(processMessage(actions, message));
        free(message);
        RETHROW(received = receiveMessage(sharedMemory, &message, &size));
    }
    return RET_SUCCESS;
}
