#ifndef KONC4_ACTION_PRINTING
#define KONC4_ACTION_PRINTING

#include "timestamps.h"
#include "action_clock.h"


struct ShowArgument
{
    int number;
    struct TimeOfDay until;
};
#define TIMESTAMP_PRESENT -1


struct ReceivedActions
{
    struct PassedAction *actionVector;
    unsigned actionVectorSize;
    struct ActionClock shutdownClock;
    unsigned clockCooldown;
};


void printAllActions(struct ShowArgument showArgument, struct ReceivedActions *actions);

#endif
