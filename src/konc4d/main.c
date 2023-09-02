#include "logging.h"
#include "settings_reading.h"
#include "shared_memory.h"
#include "message_processing.h"

#include <stdio.h>

#define INITIAL_DELAY_MINUTES 1
#if INITIAL_DELAY_MINUTES < 1
    #error("INITIAL_DELAY_MINUTES should be at least 1")
#endif
#define WAIT_CHECK_PERIOD_SECONDS 1


bool message_exit;


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

    ENSURE(createSharedMemory(sharedMemory, SHMEM_TO_KONC4D));
    ENSURE(loadActions(actions));

    struct YearTimestamp now = getCurrentTimestamp();
    struct YearTimestamp until = addMinutes(now, INITIAL_DELAY_MINUTES);
    ENSURE(skipUntilTimestamp(actions, until.timestamp, now));

    return RET_SUCCESS;
}


ReturnCode actionLoop(struct ActionQueue **actions, struct SharedMemoryFile sharedMemory)
{
    while(*actions != NULL)
    {
        struct Action current = (*actions)->action;
        struct YearTimestamp now = getCurrentTimestamp();
        RETURN_FAIL(waitUntil(now.timestamp, current.timestamp, actions, sharedMemory));
        RETURN_FAIL(doAction(&current));
        ENSURE(popActionWithRepeat(actions, NULL, now));
    }
    return RET_SUCCESS;
}


int main(void)
{
    logging_level = LOG_DEBUG;
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
