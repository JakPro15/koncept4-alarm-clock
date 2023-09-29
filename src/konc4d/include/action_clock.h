#include "passed_action.h"
#include "error_handling.h"

#include <stdint.h>
#include <stdbool.h>


struct ActionClock
{
    enum ActionType type;
    uint32_t data[45];
};


void setActionClock(struct ActionClock *toWrite, struct TimeOfDay since, struct TimeOfDay until, bool value);
bool checkActionAtTime(struct ActionClock *clock, struct TimeOfDay time) NO_IGNORE;