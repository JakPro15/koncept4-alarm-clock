#ifndef KONC4_COMMAND_EXECUTION_H
#define KONC4_COMMAND_EXECUTION_H

#include "shared_memory.h"

#include <stdarg.h>


ReturnCode ensuredOpenSharedMemory(struct SharedMemoryFile *sharedMemory) NO_IGNORE;
ReturnCode ensuredSendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned length) NO_IGNORE;
ReturnCode fullSendMessage(char *message, ...) NO_IGNORE;

#endif
