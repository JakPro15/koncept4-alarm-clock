#include "logging.h"
#include "settings_reading.h"

#include <stdio.h>

#define INITIAL_DELAY_MINUTES 1
#if INITIAL_DELAY_MINUTES < 1
    #error("INITIAL_DELAY_MINUTES should be at least 1")
#endif
#define WAIT_CHECK_PERIOD_SECONDS 30


void waitUntil(struct Timestamp start, struct Timestamp until)
{
    struct Timestamp now = getCurrentTimestamp().timestamp;
    LOG_LINE(LOG_INFO, "Sleeping until %02d.%02d %02d:%02d", until.date.day, until.date.month, until.time.hour, until.time.minute);
    while(compareTimestamp(now, until, start) < 0)
    {
        Sleep(WAIT_CHECK_PERIOD_SECONDS * 1000);
        now = getCurrentTimestamp().timestamp;
    }
}


ReturnCode initialize(struct ActionQueue **actions)
{
    logging_level = LOG_DEBUG;
    LOG_LINE(LOG_INFO, "konc4d started");

    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);

    ENSURE(loadActions(actions));

    struct YearTimestamp now = getCurrentTimestamp();
    struct YearTimestamp until = addMinutes(now, INITIAL_DELAY_MINUTES);
    ENSURE(skipUntilTimestamp(actions, until.timestamp, now));

    return RET_SUCCESS;
}


ReturnCode actionLoop(struct ActionQueue **actions)
{
    struct Action current;
    while(*actions != NULL)
    {
        struct YearTimestamp now = getCurrentTimestamp();
        ENSURE(popActionWithRepeat(actions, &current, now));
        waitUntil(now.timestamp, current.timestamp);
        RETURN_FAIL(doAction(&current));
    }
    return RET_SUCCESS;
}


int main(void)
{
    while(true)
    {
        struct ActionQueue *actions = NULL;
        ENSURE(initialize(&actions));
        switch(actionLoop(&actions))
        {
        case RET_SUCCESS:
            LOG_LINE(LOG_INFO, "All actions done, exiting");
            return 0;
        case RET_FAILURE: /* RESET */
            continue;
        case RET_ERROR:
            LOG_LINE(LOG_ERROR, "Exiting on error");
            return 1;
        }
    }
}
