#include "logging.h"
#include "command_parsing.h"
#include "input_loop.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>


const enum LOGGING_LEVEL logging_level = LOG_DEBUG;
const char logging_exe[7] = " konc4";


int main(void)
{
    LOG_LINE(LOG_INFO, "konc4 started");
    RETHROW_CALLBACK(parseInput(256, "konc4> ", parseCommandLine), LOG_LINE(LOG_ERROR, "konc4 exiting on error"));
    LOG_LINE(LOG_INFO, "konc4 stopped");
}
