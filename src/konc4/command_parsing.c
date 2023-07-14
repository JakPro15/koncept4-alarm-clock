#include "command_parsing.h"
#include "command_execution.h"
#include "shared_memory.h"
#include "logging.h"

#include <string.h>
#include <stdio.h>


ReturnCode parseCommand(char *command)
{
    LOG_LINE(LOG_DEBUG, "Parsing command: %s", command);
    char *token = strtok(command, " \t");
    if(stricmp(token, "reset") == 0)
        ENSURE(ensuredSendMessage("RESET"));
    else if(stricmp(token, "stop") == 0)
        ENSURE(ensuredSendMessage("STOP"));
    else if(stricmp(token, "start") == 0)
        ENSURE(startKonc4d());
    else
    {
        fprintf(stderr, "Unrecognized command\n");
        LOG_LINE(LOG_WARNING, "Unrecognized command received: %s", command);
        return RET_FAILURE;
    }
    if(strtok(NULL, " \t") != NULL)
    {
        fprintf(stderr, "Extra tokens in command detected\n");
        LOG_LINE(LOG_WARNING, "Extra tokens in command detected");
    }
    return RET_SUCCESS;
}


ReturnCode parseCommandLine(char *commandLine)
{
    char *command = strtok(commandLine, ";\n");
    while(command != NULL)
    {
        RETHROW(parseCommand(command));
        command = strtok(NULL, ";\n");
    }
    return RET_SUCCESS;
}
