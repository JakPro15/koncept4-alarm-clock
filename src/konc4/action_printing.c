#include "action_printing.h"
#include "passed_action.h"

#include <stdio.h>


static const char* actionType[3] = {"shutdown", "notify", "reset"};


static void printAction(const struct PassedAction *actions, unsigned index)
{
    printf("%2d) {%02d.%02d %02d:%02d, type: %8s, ", index + 1,
           actions[index].timestamp.date.day, actions[index].timestamp.date.month,
           actions[index].timestamp.time.hour, actions[index].timestamp.time.minute,
           actionType[actions[index].type]);
    if(actions[index].repeatPeriod)
        printf("repeated with period: %d minutes}\n", actions[index].repeatPeriod);
    else
        puts("not repeated}");
}


static void printActionVector(struct ShowArgument parsedArgument, struct ReceivedActions *received, struct YearTimestamp now)
{
    printf("Actions:\n");
    unsigned i = 0;
    if(parsedArgument.number == TIMESTAMP_PRESENT)
    {
        struct Timestamp until = deduceTimestamp(parsedArgument.until, now).timestamp;
        while(compareTimestamp(received->actionVector[i].timestamp, until, now.timestamp) <= 0 &&
              i < received->actionVectorSize)
            printAction(received->actionVector, i++);
    }
    else
    {
        unsigned noActionsToPrint = (received->actionVectorSize < (unsigned) parsedArgument.number) ?
                                    received->actionVectorSize :
                                    (unsigned) parsedArgument.number;
        for(; i < noActionsToPrint; i++)
            printAction(received->actionVector, i);
    }
    if(i == 0)
        puts("none");
}


static void printAllActionClocks(struct ReceivedActions *received)
{
    if(checkActionsInPeriod(&received->shutdownClock, (struct TimeOfDay){0, 0}, (struct TimeOfDay){23, 59}, 0))
        puts("No further shutdowns will be made.");
    else
    {
        puts("Shutdowns will also be made in the following periods:");
        struct TimeOfDay begin, current = {0, 0};
        bool lastAction = 0;
        while(basicCompareTime(current, (struct TimeOfDay){24, 0}) < 0)
        {
            bool currentAction = checkActionAtTime(&received->shutdownClock, current);
            if(lastAction == 0 && currentAction == 1)
                begin = current;
            else if(lastAction == 1 && currentAction == 0)
            {
                struct TimeOfDay end = decrementedTime(current);
                printf("between %02u:%02u and %02u:%02u\n", begin.hour, begin.minute, end.hour, end.minute);
            }
            lastAction = currentAction;
            incrementTime(&current);
        }
        if(lastAction == 1)
            printf("between %02u:%02u and 23:59\n", begin.hour, begin.minute);
    }
    if(received->clockCooldown / 60 > 0)
    {
        printf("No actions will be made for the next %u %s though.\n",
               received->clockCooldown / 60, (received->clockCooldown / 60 == 1) ? "minute" : "minutes");
    }
}


void printAllActions(struct ShowArgument showArgument, struct ReceivedActions *actions)
{
    struct YearTimestamp now = getCurrentTimestamp();
    printActionVector(showArgument, actions, now);
    putchar('\n');
    printAllActionClocks(actions);
}
