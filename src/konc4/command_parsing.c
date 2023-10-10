#include "command_parsing.h"
#include "command_execution.h"
#include "shared_memory.h"
#include "logging.h"
#include "konc4d_starting.h"

#include <string.h>
#include <stdio.h>


static ReturnCode getMinutesToSkip(const char *argument, unsigned *toWrite, struct YearTimestamp now)
{
    if(argument == NULL)
    {
        LOG_LINE(LOG_WARNING, "Skip command given with no argument");
        puts("skip command must take an argument (time until which to skip, or minutes to skip)");
        return RET_FAILURE;
    }
    unsigned hour, minute;
    int itemsRead = sscanf(argument, "%u:%u", &hour, &minute);
    if(itemsRead < 2)
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
    struct TimeOfDay untilTime = {hour, minute};
    if(!isTimeValid(untilTime))
    {
        puts("Invalid argument for skip command");
        LOG_LINE(LOG_WARNING, "Invalid argument for skip command: %02u:%02u", hour, minute);
        return RET_FAILURE;
    }
    struct YearTimestamp until;
    until.timestamp.time = untilTime;
    if(basicCompareTime(untilTime, now.timestamp.time) <= 0)
        until.timestamp.date = getNextDay(now);
    else
        until.timestamp.date = now.timestamp.date;
    until = deduceYear(until.timestamp, now);

    *toWrite = difference(now, until);
    LOG_LINE(LOG_DEBUG, "Determined skip argument to be (until) %02u:%02u, equivalent to %u minutes",
             hour, minute, *toWrite);
    return RET_SUCCESS;
}


ReturnCode parseCommand(char *command)
{
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
        return executeShow(argument);
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
