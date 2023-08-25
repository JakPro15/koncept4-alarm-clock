#ifndef ACTION_RECEIVING
#define ACTION_RECEIVING

#include "shared_memory.h"
#include "command_execution.h"


ReturnCode obtainActions(struct PassedAction **toWrite, unsigned *size);

#endif
