#ifndef KONC4D_MESSAGE_PROCESSING_H
#define KONC4D_MESSAGE_PROCESSING_H

#include "actions.h"
#include "shared_memory.h"


ReturnCode handleMessages(struct AllActions *actions, struct SharedMemoryFile sharedMemory) NO_IGNORE;

#endif
