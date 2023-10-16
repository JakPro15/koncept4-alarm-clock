#ifndef ACTION_CLOCK_H
#define ACTION_CLOCK_H

#include "passed_action.h"
#include "error_handling.h"

#include <stdint.h>
#include <stdbool.h>


struct ActionClock
{
    uint32_t data[45];
};


void setActionClock(struct ActionClock *toWrite, struct TimeOfDay since, struct TimeOfDay until, bool value);
bool checkActionAtTime(struct ActionClock *clock, struct TimeOfDay time) NO_IGNORE;
bool checkActionsInPeriod(struct ActionClock *clock, struct TimeOfDay from, struct TimeOfDay until, bool value) NO_IGNORE;

#endif
