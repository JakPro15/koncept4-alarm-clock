#include "command_parsing.h"

#include <string.h>

ReturnCode parseCommand(char *command, char **message)
{
    (void) command;
    (void) message;
    return RET_SUCCESS;
}

ReturnCode parseCommandLine(char *commandLine, char **messages)
{
    (void) messages;
    char *command = strtok(commandLine, ";\n");
    while(command != NULL)
    {
        char *message;
        ENSURE(parseCommand(command, &message));
        command = strtok(NULL, ";\n");
    }
    return RET_SUCCESS;
}
