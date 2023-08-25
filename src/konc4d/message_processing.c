#include "message_processing.h"
#include "logging.h"
#include "shared_memory.h"

#define SHMEM_DELAY 500
#define SHMEM_TIMEOUT 5000
#define SHMEM_TICKS_TIMEOUT SHMEM_TIMEOUT / SHMEM_DELAY


ReturnCode sendAction(struct SharedMemoryFile sharedMemory, struct Action *toSend)
{
    struct PassedAction sentAction = getPassedAction(toSend);
    LOG_LINE(LOG_DEBUG, "Sending action: {%02d.%02d %02d:%02d, type: %d}",
             toSend->timestamp.date.day, toSend->timestamp.date.month,
             toSend->timestamp.time.hour, toSend->timestamp.time.minute, toSend->type);
    ReturnCode sent;
    RETHROW(sent = sendMessage(sharedMemory, (char*) &sentAction, sizeof(struct PassedAction)));
    unsigned ticks = 0;
    while(sent == RET_FAILURE && ticks++ < SHMEM_TICKS_TIMEOUT)
    {
        Sleep(SHMEM_DELAY);
        RETHROW(sent = sendMessage(sharedMemory, (char*) &sentAction, sizeof(struct PassedAction)));
    }
    if(ticks == SHMEM_TICKS_TIMEOUT)
    {
        LOG_LINE(LOG_WARNING, "Sending action timed out");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}


unsigned countActions(struct ActionQueue *actions)
{
    unsigned result = 0;
    for(; actions != NULL; actions = actions->next)
        ++result;
    return result;
}


ReturnCode actionTransfer(struct ActionQueue *actions)
{
    struct SharedMemoryFile actionTransferQueue;
    ENSURE(openSharedMemory(&actionTransferQueue, SHMEM_FROM_KONC4D));

    char lengthMessage[SHMEM_MESSAGE_LENGTH] = "SIZE";
    SHMEM_EMBEDDED_UNSIGNED(lengthMessage) = countActions(actions);
    ENSURE_CALLBACK(sendMessage(actionTransferQueue, lengthMessage, sizeof(lengthMessage)), closeSharedMemory(actionTransferQueue));

    for(; actions != NULL; actions = actions->next)
        RETURN_FAIL_CALLBACK(sendAction(actionTransferQueue, &actions->action), closeSharedMemory(actionTransferQueue));
    LOG_LINE(LOG_INFO, "Successfully finished actions transfer");
    closeSharedMemory(actionTransferQueue);
    return RET_SUCCESS;
}


extern bool message_exit;


ReturnCode processMessage(struct ActionQueue **actions, char *message)
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
        unsigned minutesToSkip = SHMEM_EMBEDDED_UNSIGNED(message);
        struct YearTimestamp now = getCurrentTimestamp();
        struct YearTimestamp until = addMinutes(now, minutesToSkip);
        LOG_LINE(LOG_INFO, "SKIP message received, skipping by %u minutes", minutesToSkip);
        ENSURE(skipUntilTimestamp(actions, until.timestamp, now));
        return RET_SUCCESS;
    }
    else if(strcmp(message, "SEND") == 0)
    {
        LOG_LINE(LOG_INFO, "SEND message received, sending");
        RETHROW(actionTransfer(*actions));
        return RET_SUCCESS;
    }
    else
    {
        LOG_LINE(LOG_ERROR, "Unknown message received");
        return RET_ERROR;
    }
}


ReturnCode handleMessages(struct ActionQueue **actions, struct SharedMemoryFile sharedMemory)
{
    char message[SHMEM_MESSAGE_LENGTH];
    ReturnCode received;
    RETHROW(received = receiveMessage(sharedMemory, message));
    while(received != RET_FAILURE)
    {
        RETURN_FAIL(processMessage(actions, message));
        RETHROW(received = receiveMessage(sharedMemory, message));
    }
    return RET_SUCCESS;
}
