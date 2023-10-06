#ifndef ACTION_RECEIVING
#define ACTION_RECEIVING

#include "shared_memory.h"
#include "command_execution.h"
#include "action_clock.h"


struct ReceivedActions
{
    struct PassedAction *actionVector;
    unsigned actionVectorSize;
    struct ActionClock shutdownClock;
    unsigned clockCooldown;
};


ReturnCode obtainActions(struct ReceivedActions *toWrite) NO_IGNORE;

#endif
