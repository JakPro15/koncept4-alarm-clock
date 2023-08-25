#ifndef PASSED_ACTION_H
#define PASSED_ACTION_H

#include "timestamps.h"

#include <stdbool.h>


enum ActionType
{
    SHUTDOWN=0,
    NOTIFY,
    RESET
};

#define MONTHLY_REPEAT -1


struct PassedAction
{
    struct Timestamp timestamp;
    enum ActionType type;
    int repeatPeriod;
};

#endif
