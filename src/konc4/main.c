#include "logging.h"
#include "command_parsing.h"

#include <stdio.h>
#include <string.h>


int main(void)
{
    logging_level = LOG_DEBUG;
    LOG_LINE(LOG_INFO, "konc4 started");
    char commandLine[256];

    printf("konc4> ");
    while(fgets(commandLine, 256, stdin) != NULL &&
          stricmp(commandLine, "exit\n") != 0 &&
          stricmp(commandLine, "quit\n") != 0)
    {
        ENSURE(parseCommandLine(commandLine));
        printf("konc4> ");
    }
    LOG_LINE(LOG_INFO, "konc4 stopped");
}
