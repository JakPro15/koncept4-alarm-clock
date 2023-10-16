#ifndef KONC4_KONC4D_STARTING_H
#define KONC4_KONC4D_STARTING_H

#include "error_handling.h"
#include "shared_memory.h"

#include <windows.h>

ReturnCode startKonc4d(void) NO_IGNORE;
ReturnCode promptForKonc4dStart(void) NO_IGNORE;
ReturnCode checkKonc4dResponse(HANDLE *events, const char *commandName, unsigned timeout) NO_IGNORE;

ReturnCode ensuredOpenSharedMemory(struct SharedMemoryFile *sharedMemory) NO_IGNORE;
ReturnCode fullSendMessage(const char *message) NO_IGNORE;
ReturnCode fullSendMessageWithArgument(const char *message, uint64_t argument) NO_IGNORE;

#endif
