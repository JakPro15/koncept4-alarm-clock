#ifndef KONC4_ACTION_RECEIVING
#define KONC4_ACTION_RECEIVING

#include "shared_memory.h"
#include "command_execution.h"


ReturnCode obtainActions(struct ReceivedActions *toWrite) NO_IGNORE;

#endif
