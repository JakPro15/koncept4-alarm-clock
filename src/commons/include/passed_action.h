#ifndef PASSED_ACTION_H
#define PASSED_ACTION_H

#include "timestamps.h"

#include <stdbool.h>


enum ActionType
{
    SHUTDOWN,
    NOTIFY,
    RESET
};


struct PassedAction
{
    struct Timestamp timestamp;
    enum ActionType type;
    bool repeated;
};

#endif
