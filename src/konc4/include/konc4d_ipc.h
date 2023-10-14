#ifndef KONC4_KONC4D_STARTING_H
#define KONC4_KONC4D_STARTING_H

#include "error_handling.h"
#include <windows.h>


ReturnCode startKonc4d(void) NO_IGNORE;
ReturnCode promptForKonc4dStart(void) NO_IGNORE;
ReturnCode checkKonc4dResponse(HANDLE *events, const char *commandName, unsigned timeout) NO_IGNORE;

#endif
