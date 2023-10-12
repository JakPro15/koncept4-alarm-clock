#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "message_processing.h"
#include "logging.h"
#include "shared_memory.h"
#include "events.h"

#define SHMEM_TIMEOUT 5000


struct ShowArgument
{
    int number;
    struct TimeOfDay until;
};
#define TIMESTAMP_PRESENT -1

static const char* actionType[3] = {"shutdown", "notify", "reset"};


static ReturnCode obtainFileFromHandle(HANDLE fileHandle, FILE **toWrite)
{
    int fd = _open_osfhandle((intptr_t) fileHandle, _O_WRONLY);
    if(fd == -1)
    {
        LOG_LINE(LOG_ERROR, "_open_osfhandle failed");
        return RET_ERROR;
    }
    *toWrite = _fdopen(fd, "w");
    if(*toWrite == NULL)
    {
        LOG_LINE(LOG_ERROR, "_fdopen failed");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static void printAction(FILE *stdoutFile, const struct Action *action, unsigned index)
{
    fprintf(stdoutFile, "%2d) {%02d.%02d %02d:%02d, type: %8s, ", index + 1,
            action->timestamp.date.day, action->timestamp.date.month,
            action->timestamp.time.hour, action->timestamp.time.minute,
            actionType[action->type]);
    if(action->repeatPeriod)
        fprintf(stdoutFile, "repeated with period: %d minutes}\n", action->repeatPeriod);
    else
        fprintf(stdoutFile, "not repeated}\n");
}


static void printActionVector(FILE *stdoutFile, struct ShowArgument argument, struct ActionQueue *head, struct YearTimestamp now)
{
    fprintf(stdoutFile, "Actions:\n");
    int i = 0;
    if(argument.number == TIMESTAMP_PRESENT)
    {
        struct Timestamp until = deduceTimestamp(argument.until, now).timestamp;
        while(compareTimestamp(head->action.timestamp, until, now.timestamp) <= 0 && head != NULL)
        {
            printAction(stdoutFile, &head->action, i++);
            head = head->next;
        }
    }
    else
    {
        while(i < argument.number && head != NULL)
        {
            printAction(stdoutFile, &head->action, i++);
            head = head->next;
        }
    }
    if(i == 0)
        fprintf(stdoutFile, "none\n");
}


static void printAllActionClocks(FILE *stdoutFile, struct AllActions *actions)
{
    if(checkActionsInPeriod(&actions->shutdownClock, (struct TimeOfDay){0, 0}, (struct TimeOfDay){23, 59}, 0))
        fprintf(stdoutFile, "No further shutdowns will be made\n");
    else
    {
        fprintf(stdoutFile, "Shutdowns will also be made in the following periods:\n");
        struct TimeOfDay begin, current = {0, 0};
        bool lastAction = 0;
        while(basicCompareTime(current, (struct TimeOfDay){24, 0}) < 0)
        {
            bool currentAction = checkActionAtTime(&actions->shutdownClock, current);
            if(lastAction == 0 && currentAction == 1)
                begin = current;
            else if(lastAction == 1 && currentAction == 0)
            {
                struct TimeOfDay end = decrementedTime(current);
                fprintf(stdoutFile, "between %02u:%02u and %02u:%02u\n", begin.hour, begin.minute, end.hour, end.minute);
            }
            lastAction = currentAction;
            incrementTime(&current);
        }
        if(lastAction == 1)
            fprintf(stdoutFile, "between %02u:%02u and 23:59\n", begin.hour, begin.minute);
    }
    if(actions->clockCooldown / 60 > 0)
    {
        fprintf(stdoutFile, "No actions will be made for the next %u %s though.\n",
                actions->clockCooldown / 60, (actions->clockCooldown / 60 == 1) ? "minute" : "minutes");
    }
}


extern bool message_exit;


ReturnCode processMessage(struct SharedMemoryFile sharedMemory, struct AllActions *actions, char *message, uint64_t argument)
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
        ENSURE(skipUntilTimestamp(&actions->queueHead, until.timestamp, now));
        unsigned newCooldown = difference(now, until) * SECONDS_IN_MINUTE;
        actions->clockCooldown = max(actions->clockCooldown, newCooldown);
        LOG_LINE(LOG_DEBUG, "Action clock actions disabled for %u seconds", actions->clockCooldown);
        return RET_SUCCESS;
    }
    else if(strcmp(message, "SHOW") == 0)
    {
        LOG_LINE(LOG_INFO, "SHOW message received, showing");
        HANDLE stdoutHandle = (HANDLE) argument;
        FILE *stdoutFile;
        ENSURE(obtainFileFromHandle(stdoutHandle, &stdoutFile));
        char *showArgumentMessage;
        unsigned showArgumentSize;
        if(timeoutReceiveSizedMessage(sharedMemory, &showArgumentMessage, &showArgumentSize, SHMEM_TIMEOUT) != RET_SUCCESS)
        {
            LOG_LINE(LOG_WARNING, "Did not receive show argument message after receiving SHOW");
            fprintf(stdoutFile, "konc4d did not receive show argument message from konc4");
            return RET_SUCCESS;
        }
        if(showArgumentSize != sizeof(struct ShowArgument))
        {
            LOG_LINE(LOG_WARNING, "Show argument message received with wrong size: %u", showArgumentSize);
            fprintf(stdoutFile, "konc4d did received show argument message from konc4 with wrong size: %u", showArgumentSize);
            return RET_SUCCESS;
        }
        struct ShowArgument showArgument = *((struct ShowArgument*) showArgumentMessage);
        if(showArgument.number == TIMESTAMP_PRESENT)
            LOG_LINE(LOG_DEBUG, "Received show argument: %02u:%02u", showArgument.until.hour, showArgument.until.minute);
        else
            LOG_LINE(LOG_DEBUG, "Received show argument: %d", showArgument.number);
        free(showArgumentMessage);

        printActionVector(stdoutFile, showArgument, actions->queueHead, now);
        printAllActionClocks(stdoutFile, actions);
        fclose(stdoutFile);
        CloseHandle(stdoutHandle);

        LOG_LINE(LOG_DEBUG, "konc4 show command executed successfully, sending confirmation");
        HANDLE confirmEvent;
        ENSURE(createEventObject(&confirmEvent, EVENT_COMMAND_CONFIRM));
        ENSURE(pingEventObject(confirmEvent));
        CloseHandle(confirmEvent);
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
        RETURN_FAIL(processMessage(sharedMemory, actions, message, argument));
        free(message);
        RETHROW(received = receiveMessageWithArgument(sharedMemory, &message, &argument, NO_WAIT));
    }
    return RET_SUCCESS;
}
