#include "logging.h"
#include "settings_reading.h"
#include "shared_memory.h"
#include "message_processing.h"
#include "events.h"

#include <stdio.h>

#define INITIAL_DELAY_MINUTES 1
#if INITIAL_DELAY_MINUTES < 1
    #error("INITIAL_DELAY_MINUTES should be at least 1")
#endif
#define WAIT_CHECK_PERIOD_SECONDS 30

#define MAX_CLOCK_COOLDOWN_SECONDS 59
#if MAX_CLOCK_COOLDOWN_SECONDS >= 60
    #error("MAX_CLOCK_COOLDOWN_SECONDS mustn't exceed 59")
#endif


bool message_exit;


static const struct Action clockShutdown = {.type = SHUTDOWN, .args.shutdown.delay = DEFAULT_SHUTDOWN_DELAY};

static ReturnCode checkActionClocks(struct AllActions *actions, struct TimeOfDay now)
{
    if(actions->clockCooldown <= 0 && checkActionAtTime(&actions->shutdownClock, now))
    {
        ENSURE(doAction(&clockShutdown));
        actions->clockCooldown = MAX_CLOCK_COOLDOWN_SECONDS;
    }
    else
    {
        if(actions->clockCooldown >= WAIT_CHECK_PERIOD_SECONDS)
            actions->clockCooldown -= WAIT_CHECK_PERIOD_SECONDS;
        else
            actions->clockCooldown = 0;
    }
    return RET_SUCCESS;
}


static ReturnCode waitUntil(struct Timestamp start, struct Timestamp until, HANDLE konc4Event,
                            struct AllActions *actions, struct SharedMemoryFile sharedMemory)
{
    struct Timestamp now = getCurrentTimestamp().timestamp;
    LOG_LINE(LOG_INFO, "Sleeping until %02d.%02d %02d:%02d", until.date.day,
             until.date.month, until.time.hour, until.time.minute);
    while(compareTimestamp(now, until, start) < 0)
    {
        RETURN_FAIL(checkActionClocks(actions, now.time));
        ReturnCode waitResult;
        RETHROW(waitResult = waitOnEventObject(konc4Event, WAIT_CHECK_PERIOD_SECONDS * 1000));
        if(waitResult == RET_SUCCESS)
            RETURN_FAIL(handleMessages(actions, sharedMemory));
        now = getCurrentTimestamp().timestamp;
    }
    return RET_SUCCESS;
}


ReturnCode initialize(struct AllActions *actions, struct SharedMemoryFile *sharedMemory, HANDLE *konc4Event)
{
    message_exit = false;
    LOG_LINE(LOG_INFO, "konc4d started");

    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);

    ENSURE(createSharedMemory(sharedMemory, SHMEM_TO_KONC4D));
    ENSURE(createEventObject(konc4Event, EVENT_TO_KONC4D));
    ENSURE(loadActions(actions));

    struct YearTimestamp now = getCurrentTimestamp();
    struct YearTimestamp until = addMinutes(now, INITIAL_DELAY_MINUTES);
    ENSURE(skipUntilTimestamp(&actions->queueHead, until.timestamp, now));

    return RET_SUCCESS;
}


ReturnCode actionLoop(struct AllActions *actions, struct SharedMemoryFile sharedMemory, HANDLE konc4Event)
{
    while(actions->queueHead != NULL)
    {
        struct Action current = (actions->queueHead)->action;
        struct YearTimestamp now = getCurrentTimestamp();
        RETURN_FAIL(waitUntil(now.timestamp, current.timestamp, konc4Event, actions, sharedMemory));
        if(!actionsEqual(&current, &(actions->queueHead)->action))
            continue;
        RETURN_FAIL(doAction(&current));
        ENSURE(popActionWithRepeat(&actions->queueHead, NULL, now));
    }
    return RET_SUCCESS;
}


int main(void)
{
    struct AllActions actions = {.queueHead = NULL};
    struct SharedMemoryFile sharedMemory;
    HANDLE konc4Event;
    while(true)
    {
        ENSURE(initialize(&actions, &sharedMemory, &konc4Event));
        ReturnCode returned = actionLoop(&actions, sharedMemory, konc4Event);

        destroyActionQueue(&actions.queueHead);
        closeSharedMemory(sharedMemory);
        CloseHandle(konc4Event);
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
