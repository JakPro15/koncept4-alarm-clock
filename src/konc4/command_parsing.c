#include "command_parsing.h"
#include "command_execution.h"
#include "shared_memory.h"
#include "logging.h"
#include "konc4d_ipc.h"
#include "action_printing.h"

#include <string.h>
#include <stdio.h>


static ReturnCode getNumberShowArgument(const char *argument, struct ShowArgument *toWrite)
{
    unsigned long parsed = strtoul(argument, NULL, 0);
    if(parsed == 0 || parsed > INT_MAX)
    {
        puts("Invalid argument for show command");
        LOG_LINE(LOG_WARNING, "Invalid argument for show command: %s", argument);
        return RET_FAILURE;
    }
    toWrite->number = (int) parsed;
    LOG_LINE(LOG_DEBUG, "Determined show argument to be %d", toWrite->number);
    return RET_SUCCESS;
}


static ReturnCode parseShowArgument(const char *argument, struct ShowArgument *toWrite)
{
    if(argument == NULL)
    {
        toWrite->number = INT_MAX;
        LOG_LINE(LOG_DEBUG, "Determined show argument to be empty");
        return RET_SUCCESS;
    }
    int itemsRead = sscanf(argument, "%u:%u", &toWrite->until.hour, &toWrite->until.minute);
    if(itemsRead < 2)
        return getNumberShowArgument(argument, toWrite);
    else
        toWrite->number = TIMESTAMP_PRESENT;
    if(!isTimeValid(toWrite->until))
    {
        puts("Invalid argument for show command");
        LOG_LINE(LOG_WARNING, "Invalid argument for show command: %u:%u", toWrite->until.hour, toWrite->until.minute);
        return RET_FAILURE;
    }
    LOG_LINE(LOG_DEBUG, "Determined show argument to be (until) %u:%u",
             toWrite->until.hour, toWrite->until.minute);
    return RET_SUCCESS;
}


static ReturnCode getNumberSkipArgument(const char *argument, unsigned *toWrite)
{
    unsigned long parsed = strtoul(argument, NULL, 0);
    if(parsed == 0 || parsed > UINT_MAX)
    {
        puts("Invalid argument for skip command");
        LOG_LINE(LOG_WARNING, "Invalid argument for skip command: %s", argument);
        return RET_FAILURE;
    }
    *toWrite = (unsigned) parsed;
    LOG_LINE(LOG_DEBUG, "Determined skip argument to be %u", *toWrite);
    return RET_SUCCESS;
}


static ReturnCode getMinutesToSkip(const char *argument, unsigned *toWrite, struct YearTimestamp now)
{
    if(argument == NULL)
    {
        LOG_LINE(LOG_WARNING, "Skip command given with no argument");
        puts("skip command must take an argument (time until which to skip, or minutes to skip)");
        return RET_FAILURE;
    }
    struct TimeOfDay untilTime;
    int itemsRead = sscanf(argument, "%u:%u", &untilTime.hour, &untilTime.minute);
    if(itemsRead < 2)
        return getNumberSkipArgument(argument, toWrite);
    if(!isTimeValid(untilTime))
    {
        puts("Invalid argument for skip command");
        LOG_LINE(LOG_WARNING, "Invalid argument for skip command: %02u:%02u", untilTime.hour, untilTime.minute);
        return RET_FAILURE;
    }
    *toWrite = difference(now, deduceTimestamp(untilTime, now));
    LOG_LINE(LOG_DEBUG, "Determined skip argument to be (until) %02u:%02u, equivalent to %u minutes",
             untilTime.hour, untilTime.minute, *toWrite);
    return RET_SUCCESS;
}


ReturnCode parseCommand(char *command)
{
    if(command == NULL)
    {
        LOG_LINE(LOG_DEBUG, "Parsing command: (null)");
        return RET_SUCCESS;
    }
    LOG_LINE(LOG_DEBUG, "Parsing command: %s", command);
    char *saveptr;
    char *token = strtok_r(command, " \t", &saveptr);
    if(stricmp(token, "reset") == 0)
        return executeReset();
    else if(stricmp(token, "stop") == 0)
        return executeStop();
    else if(stricmp(token, "start") == 0)
        return executeStart();
    else if(stricmp(token, "show") == 0)
    {
        char *argument = strtok_r(NULL, " \t", &saveptr);
        struct ShowArgument parsedArgument;
        RETURN_FAIL(parseShowArgument(argument, &parsedArgument));
        return executeShow(parsedArgument);
    }
    else if(stricmp(token, "skip") == 0)
    {
        char *toSkipStr = strtok_r(NULL, " \t", &saveptr);
        unsigned minutesToSkip;
        RETURN_FAIL(getMinutesToSkip(toSkipStr, &minutesToSkip, getCurrentTimestamp()));
        return executeSkip(minutesToSkip);
    }
    else
    {
        puts("Unrecognized command");
        LOG_LINE(LOG_WARNING, "Unrecognized command received: %s", command);
        return RET_FAILURE;
    }
    if(strtok_r(NULL, " \t", &saveptr) != NULL)
    {
        puts("Extra tokens in command detected");
        LOG_LINE(LOG_WARNING, "Extra tokens in command detected");
    }
    return RET_SUCCESS;
}


enum CallbackReturn parseCommandLine(char *commandLine)
{
    if(stricmp(commandLine, "exit\n") == 0 ||
       stricmp(commandLine, "quit\n") == 0)
        return END_SPINNING_SUCCESS;
    char *saveptr, *command = strtok_r(commandLine, ";\n", &saveptr);
    do
    {
        if(parseCommand(command) == RET_ERROR)
            return END_SPINNING_ERROR;
        command = strtok_r(NULL, ";\n", &saveptr);
    } while(command != NULL);
    return KEEP_SPINNING;
}
