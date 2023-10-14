#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "message_processing.h"
#include "logging.h"
#include "shared_memory.h"
#include "events.h"

#define SHMEM_TIMEOUT 5000


static ReturnCode sendItem(struct SharedMemoryFile sharedMemory, char *item, unsigned size)
{
    ReturnCode sent;
    RETHROW(sent = sendSizedMessage(sharedMemory, item, size));
    while(sent == RET_FAILURE)
    {
        RETHROW(sent = waitOnEventObject(sharedMemory.readEvent, SHMEM_TIMEOUT));
        if(sent == RET_FAILURE)
        {
            LOG_LINE(LOG_WARNING, "Sending action or action clock timed out");
            return RET_FAILURE;
        }
        RETHROW(sent = sendSizedMessage(sharedMemory, item, size));
    }
    return RET_SUCCESS;
}


static ReturnCode sendAction(struct SharedMemoryFile sharedMemory, struct Action *toSend)
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


static ReturnCode actionTransfer(struct AllActions *actions)
{
    struct SharedMemoryFile actionTransferQueue;
    ENSURE(openSharedMemory(&actionTransferQueue, SHMEM_FROM_KONC4D));

    unsigned noActions = countActions(actions->queueHead);
    LOG_LINE(LOG_DEBUG, "Sending SIZE %d message", noActions);
    ENSURE_CALLBACK(sendMessageWithArgument(actionTransferQueue, "SIZE", noActions, NO_WAIT),
                    closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_DEBUG, "Sending actions");
    struct ActionQueue *queue = actions->queueHead;
    for(; queue != NULL; queue = queue->next)
        RETURN_FAIL_CALLBACK(sendAction(actionTransferQueue, &queue->action), closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_DEBUG, "Sending shutdown action clock");
    RETURN_FAIL_CALLBACK(sendItem(actionTransferQueue, (char*) &actions->shutdownClock, sizeof(actions->shutdownClock)),
                         closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_DEBUG, "Sending action clock cooldown: %u", actions->clockCooldown);
    ENSURE_CALLBACK(sendMessageWithArgument(actionTransferQueue, "COOLDOWN", actions->clockCooldown, NO_WAIT),
                    closeSharedMemory(actionTransferQueue));

    LOG_LINE(LOG_INFO, "Successfully finished actions transfer");
    closeSharedMemory(actionTransferQueue);
    return RET_SUCCESS;
}


extern bool message_exit;


static ReturnCode processMessage(struct AllActions *actions, char *message, uint64_t argument)
{
    struct YearTimestamp now = getCurrentTimestamp();
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
        unsigned minutesToSkip = (unsigned) argument;
        struct YearTimestamp until = addMinutes(now, minutesToSkip);
        LOG_LINE(LOG_INFO, "SKIP message received, skipping by %u minutes", minutesToSkip);
        ENSURE_CALLBACK(skipUntilTimestamp(&actions->queueHead, until.timestamp, now),
                        sendNotification(EVENT_COMMAND_ERROR));
        unsigned newCooldown = difference(now, until) * SECONDS_IN_MINUTE;
        actions->clockCooldown = max(actions->clockCooldown, newCooldown);
        LOG_LINE(LOG_DEBUG, "Action clock actions disabled for %u seconds", actions->clockCooldown);
        ENSURE(sendNotification(EVENT_COMMAND_ERROR));
        return RET_SUCCESS;
    }
    else if(strcmp(message, "SEND") == 0)
    {
        LOG_LINE(LOG_INFO, "SEND message received, sending");
        ENSURE(actionTransfer(actions));
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
    uint64_t argument;
    ReturnCode received;
    RETHROW(received = receiveMessageWithArgument(sharedMemory, &message, &argument, NO_WAIT));
    while(received != RET_FAILURE)
    {
        RETURN_FAIL_CALLBACK(processMessage(actions, message, argument),
                             free(message); sendNotification(EVENT_COMMAND_ERROR));
        free(message);
        RETHROW(received = receiveMessageWithArgument(sharedMemory, &message, &argument, NO_WAIT));
    }
    return RET_SUCCESS;
}
