#ifndef KONC4_COMMAND_EXECUTION_H
#define KONC4_COMMAND_EXECUTION_H

#include "shared_memory.h"

#include <stdarg.h>


ReturnCode executeStart(void) NO_IGNORE;
ReturnCode executeStop(void) NO_IGNORE;
ReturnCode executeReset(void) NO_IGNORE;
ReturnCode executeSkip(unsigned minutesToSkip) NO_IGNORE;

ReturnCode ensuredOpenSharedMemory(struct SharedMemoryFile *sharedMemory) NO_IGNORE;
ReturnCode ensuredSendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned length) NO_IGNORE;
ReturnCode fullSendMessage(char *message, ...) NO_IGNORE;

#endif
