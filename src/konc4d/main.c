#include "logging.h"
#include "settings_reading.h"
#include "shared_memory.h"

#include <stdio.h>

#define INITIAL_DELAY_MINUTES 1
#if INITIAL_DELAY_MINUTES < 1
    #error("INITIAL_DELAY_MINUTES should be at least 1")
#endif
#define WAIT_CHECK_PERIOD_SECONDS 1


static bool message_exit;


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


ReturnCode waitUntil(struct Timestamp start, struct Timestamp until,
                     struct ActionQueue **actions, struct SharedMemoryFile sharedMemory)
{
    struct Timestamp now = getCurrentTimestamp().timestamp;
    LOG_LINE(LOG_INFO, "Sleeping until %02d.%02d %02d:%02d", until.date.day,
             until.date.month, until.time.hour, until.time.minute);
    while(compareTimestamp(now, until, start) < 0)
    {
        RETURN_FAIL(handleMessages(actions, sharedMemory));
        Sleep(WAIT_CHECK_PERIOD_SECONDS * 1000);
        now = getCurrentTimestamp().timestamp;
    }
    return RET_SUCCESS;
}


ReturnCode initialize(struct ActionQueue **actions, struct SharedMemoryFile *sharedMemory)
{
    message_exit = false;
    LOG_LINE(LOG_INFO, "konc4d started");

    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);

    ENSURE(createSharedMemory(sharedMemory, SHMEM_KONC4D_WRITE));
    ENSURE(loadActions(actions));

    struct YearTimestamp now = getCurrentTimestamp();
    struct YearTimestamp until = addMinutes(now, INITIAL_DELAY_MINUTES);
    ENSURE(skipUntilTimestamp(actions, until.timestamp, now));

    return RET_SUCCESS;
}


ReturnCode actionLoop(struct ActionQueue **actions, struct SharedMemoryFile sharedMemory)
{
    struct Action current;
    while(*actions != NULL)
    {
        struct YearTimestamp now = getCurrentTimestamp();
        ENSURE(popActionWithRepeat(actions, &current, now));
        RETURN_FAIL(waitUntil(now.timestamp, current.timestamp, actions, sharedMemory));
        RETURN_FAIL(doAction(&current));
    }
    return RET_SUCCESS;
}


int main(void)
{
    struct ActionQueue *actions = NULL;
    struct SharedMemoryFile sharedMemory;
    while(true)
    {
        ENSURE(initialize(&actions, &sharedMemory));
        ReturnCode returned = actionLoop(&actions, sharedMemory);

        destroyActionQueue(&actions);
        closeSharedMemory(sharedMemory);
        if(returned == RET_SUCCESS || message_exit)
        {
            LOG_LINE(LOG_INFO, "konc4d stopped");
            return 0;
        }
        else if(returned == RET_ERROR)
        {
            LOG_LINE(LOG_ERROR, "konc4d exiting on error");
            return 1;
        }
    }
}
