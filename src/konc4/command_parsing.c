#include "command_parsing.h"
#include "command_execution.h"
#include "shared_memory.h"
#include "logging.h"
#include "konc4d_starting.h"

#include <string.h>
#include <stdio.h>


ReturnCode parseCommand(char *command)
{
    LOG_LINE(LOG_DEBUG, "Parsing command: %s", command);
    char *saveptr;
    char *token = strtok_r(command, " \t", &saveptr);
    if(stricmp(token, "reset") == 0)
        RETHROW(ensuredSendMessage("RESET"));
    else if(stricmp(token, "stop") == 0)
        RETHROW(ensuredSendMessage("STOP"));
    else if(stricmp(token, "start") == 0)
        RETHROW(startKonc4d());
    else if(stricmp(token, "skip") == 0)
    {
        char *minutesToSkipStr = strtok_r(NULL, " \t", &saveptr);
        unsigned minutesToSkip = strtoul(minutesToSkipStr, NULL, 0);
        if(minutesToSkip == 0)
        {
            fprintf(stderr, "skip command expects a positive integer argument\n");
            LOG_LINE(LOG_WARNING, "skip command received invalid argument: %s", minutesToSkipStr);
            return RET_FAILURE;
        }
        RETHROW(ensuredSendMessage("SKIP", minutesToSkip));
    }
    else
    {
        fprintf(stderr, "Unrecognized command\n");
        LOG_LINE(LOG_WARNING, "Unrecognized command received: %s", command);
        return RET_FAILURE;
    }
    if(strtok_r(NULL, " \t", &saveptr) != NULL)
    {
        fprintf(stderr, "Extra tokens in command detected\n");
        LOG_LINE(LOG_WARNING, "Extra tokens in command detected");
    }
    return RET_SUCCESS;
}


enum CallbackReturn parseCommandLine(char *commandLine)
{
    if(stricmp(commandLine, "exit\n") == 0 ||
       stricmp(commandLine, "quit\n") == 0)
        return END_SPINNING_SUCCESS;
    char *saveptr;
    char *command = strtok_r(commandLine, ";\n", &saveptr);
    while(command != NULL)
    {
        if(parseCommand(command) == RET_ERROR)
            return END_SPINNING_ERROR;
        command = strtok_r(NULL, ";\n", &saveptr);
    }
    return KEEP_SPINNING;
}
