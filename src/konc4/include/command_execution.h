#ifndef KONC4_COMMAND_EXECUTION_H
#define KONC4_COMMAND_EXECUTION_H

#include "shared_memory.h"
#include "action_printing.h"

#include <stdarg.h>


ReturnCode executeStart(void) NO_IGNORE;
ReturnCode executeStop(void) NO_IGNORE;
ReturnCode executeReset(void) NO_IGNORE;
ReturnCode executeSkip(unsigned minutesToSkip) NO_IGNORE;
ReturnCode executeShow(struct ShowArgument argument) NO_IGNORE;

#endif
