#include "logging.h"
#include "settings_reading.h"

#include <stdio.h>

#define INITIAL_DELAY_MINUTES 5


int main(void)
{
    LOG_LINE(LOG_INFO, "konc4d started");
    struct ActionQueue *actions = NULL;
    ENSURE(loadActions(&actions));

    struct YearTimestamp now = getCurrentTimestamp();
    struct YearTimestamp until = addMinutes(now, INITIAL_DELAY_MINUTES);
    ENSURE(skipUntilTimestamp(&actions, until.timestamp, now));

    struct Action current;
    while(actions != NULL)
    {
        popActionWithRepeat(&actions, &current, now);
        printf("Type: %d, %d.%d %d:%d, repeated: %d\n", current.type, current.timestamp.date.day, current.timestamp.date.month,
               current.timestamp.time.hour, current.timestamp.time.minute, current.repeated);
    }
}
