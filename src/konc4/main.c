#include "logging.h"
#include "command_parsing.h"
#include "input_loop.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>


int main(void)
{
    logging_level = LOG_DEBUG;
    LOG_LINE(LOG_INFO, "konc4 started");
    RETHROW_CALLBACK(parseInput(256, "konc4> ", parseCommandLine), LOG_LINE(LOG_ERROR, "konc4 exiting on error"));
    LOG_LINE(LOG_INFO, "konc4 stopped");
}
