#include "command_execution.h"
#include "logging.h"
#include "shared_memory.h"
#include "input_loop.h"
#include "konc4d_ipc.h"
#include "action_receiving.h"
#include "timestamps.h"
#include "events.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>


ReturnCode executeStart(void)
{
    HANDLE startupEvent;
    ENSURE(createEventObject(&startupEvent, EVENT_KONC4D_STARTUP));
    RETURN_FAIL(startKonc4d());
    ReturnCode done;
    RETHROW(done = waitOnEventObject(startupEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d did not start up - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 start command timed out; no startup detected");
        return RET_FAILURE;
    }
    puts("start command executed successfully.");
    LOG_LINE(LOG_INFO, "konc4 start command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeStop(void)
{
    if(isKonc4dOn() != RET_SUCCESS)
    {
        puts("konc4d is off.");
        return RET_FAILURE;
    }
    HANDLE shutdownEvent;
    ENSURE(createEventObject(&shutdownEvent, EVENT_KONC4D_SHUTDOWN));
    RETURN_FAIL(fullSendMessage("STOP"));
    ReturnCode done;
    RETHROW(done = waitOnEventObject(shutdownEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d did not shut down - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 stop command timed out; no shutdown detected");
        return RET_FAILURE;
    }
    puts("stop command executed successfully.");
    LOG_LINE(LOG_INFO, "konc4 stop command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeReset(void)
{
    if(isKonc4dOn() != RET_SUCCESS)
    {
        puts("konc4d is off.");
        return RET_FAILURE;
    }
    HANDLE shutdownEvent, startupEvent;
    ENSURE(createEventObject(&shutdownEvent, EVENT_KONC4D_SHUTDOWN));
    ENSURE(createEventObject(&startupEvent, EVENT_KONC4D_STARTUP));
    RETURN_FAIL(fullSendMessage("RESET"));
    ReturnCode done;
    RETHROW(done = waitOnEventObject(shutdownEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d did not shut down - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 reset command timed out; no shutdown detected");
        return RET_FAILURE;
    }
    RETHROW(done = waitOnEventObject(startupEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d shut down, but did not start up again - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 reset command failed in konc4d; no startup detected after shutdown");
        return RET_FAILURE;
    }
    puts("reset command executed successfully.");
    LOG_LINE(LOG_INFO, "konc4 reset command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeSkip(unsigned minutesToSkip)
{
    if(minutesToSkip == 0)
    {
        printf("Skip command expects a positive integer argument.\n");
        LOG_LINE(LOG_WARNING, "skip command received invalid argument: %d", minutesToSkip);
        return RET_FAILURE;
    }
    if(minutesToSkip > 7200)
    {
        printf("Skipping more than five days at once is not supported.\n");
        LOG_LINE(LOG_WARNING, "skip command received invalid argument: %d", minutesToSkip);
        return RET_FAILURE;
    }
    HANDLE events[2];
    ENSURE(createEventObject(&events[0], EVENT_COMMAND_CONFIRM));
    ENSURE(createEventObject(&events[1], EVENT_COMMAND_ERROR));
    RETURN_FAIL(fullSendMessageWithArgument("SKIP", minutesToSkip));
    return checkKonc4dResponse(events, "skip", EVENT_TIMEOUT);
}


struct ShowArgument
{
    int number;
    struct TimeOfDay until;
};
#define TIMESTAMP_PRESENT -1


static ReturnCode parseShowArgument(const char *argument, struct ShowArgument *toWrite)
{
    if(argument == NULL)
    {
        toWrite->number = INT_MAX;
        LOG_LINE(LOG_DEBUG, "Determined show argument to be empty");
        return RET_SUCCESS;
    }
    unsigned hour, minute;
    int itemsRead = sscanf(argument, "%u:%u", &hour, &minute);
    if(itemsRead < 2)
    {
        long parsed = strtoul(argument, NULL, 0);
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
    else
        toWrite->number = TIMESTAMP_PRESENT;
    if(hour > 23 || minute > 59)
    {
        puts("Invalid argument for show command");
        LOG_LINE(LOG_WARNING, "Invalid argument for show command: %u:%u", hour, minute);
        return RET_FAILURE;
    }
    toWrite->until = (struct TimeOfDay) {hour, minute};
    LOG_LINE(LOG_DEBUG, "Determined show argument to be (until) %u:%u",
             toWrite->until.hour, toWrite->until.minute);
    return RET_SUCCESS;
}


static const char* actionType[3] = {"shutdown", "notify", "reset"};


static void printAction(const struct PassedAction *actions, unsigned index)
{
    printf("%2d) {%02d.%02d %02d:%02d, type: %8s, ", index + 1,
           actions[index].timestamp.date.day, actions[index].timestamp.date.month,
           actions[index].timestamp.time.hour, actions[index].timestamp.time.minute,
           actionType[actions[index].type]);
    if(actions[index].repeatPeriod)
        printf("repeated with period: %d minutes}\n", actions[index].repeatPeriod);
    else
        puts("not repeated}");
}


static void printActionVector(struct ShowArgument parsedArgument, struct ReceivedActions *received, struct YearTimestamp now)
{
    printf("Actions:\n");
    unsigned i = 0;
    if(parsedArgument.number == TIMESTAMP_PRESENT)
    {
        struct Timestamp until = deduceTimestamp(parsedArgument.until, now).timestamp;
        while(compareTimestamp(received->actionVector[i].timestamp, until, now.timestamp) <= 0 &&
              i < received->actionVectorSize)
            printAction(received->actionVector, i++);
    }
    else
    {
        unsigned noActionsToPrint = (received->actionVectorSize < (unsigned) parsedArgument.number) ?
                                    received->actionVectorSize :
                                    (unsigned) parsedArgument.number;
        for(; i < noActionsToPrint; i++)
            printAction(received->actionVector, i);
    }
    if(i == 0)
        puts("none");
}


static void printAllActionClocks(struct ReceivedActions *received)
{
    if(checkActionsInPeriod(&received->shutdownClock, (struct TimeOfDay){0, 0}, (struct TimeOfDay){23, 59}, 0))
        puts("No further shutdowns will be made.");
    else
    {
        puts("Shutdowns will also be made in the following periods:");
        struct TimeOfDay begin, current = {0, 0};
        bool lastAction = 0;
        while(basicCompareTime(current, (struct TimeOfDay){24, 0}) < 0)
        {
            bool currentAction = checkActionAtTime(&received->shutdownClock, current);
            if(lastAction == 0 && currentAction == 1)
                begin = current;
            else if(lastAction == 1 && currentAction == 0)
            {
                struct TimeOfDay end = decrementedTime(current);
                printf("between %02u:%02u and %02u:%02u\n", begin.hour, begin.minute, end.hour, end.minute);
            }
            lastAction = currentAction;
            incrementTime(&current);
        }
        if(lastAction == 1)
            printf("between %02u:%02u and 23:59\n", begin.hour, begin.minute);
    }
    if(received->clockCooldown / 60 > 0)
    {
        printf("No actions will be made for the next %u %s though.\n",
               received->clockCooldown / 60, (received->clockCooldown / 60 == 1) ? "minute" : "minutes");
    }
}


ReturnCode executeShow(const char *argument)
{
    struct YearTimestamp now = getCurrentTimestamp();
    struct ShowArgument parsedArgument;
    RETURN_FAIL(parseShowArgument(argument, &parsedArgument));

    struct ReceivedActions received;
    RETURN_FAIL(obtainActions(&received));
    printActionVector(parsedArgument, &received, now);
    free(received.actionVector);
    putchar('\n');
    printAllActionClocks(&received);

    LOG_LINE(LOG_INFO, "konc4 show command executed successfully");
    return RET_SUCCESS;
}


ReturnCode ensuredOpenSharedMemory(struct SharedMemoryFile *sharedMemory)
{
    ReturnCode isOn;
    RETHROW(isOn = isKonc4dOn());
    if(isOn == RET_FAILURE)
        RETURN_FAIL(promptForKonc4dStart());
    ENSURE(openSharedMemory(sharedMemory, SHMEM_TO_KONC4D));
    return RET_SUCCESS;
}


ReturnCode fullSendMessage(const char *message)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));
    ENSURE_CALLBACK(sendMessage(sharedMemory, message, SHMEM_TIMEOUT),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(sendNotification(EVENT_NOTIFY_KONC4D));
    return RET_SUCCESS;
}


ReturnCode fullSendMessageWithArgument(const char *message, uint64_t argument)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));
    ENSURE_CALLBACK(sendMessageWithArgument(sharedMemory, message, argument, SHMEM_TIMEOUT),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(sendNotification(EVENT_NOTIFY_KONC4D));
    return RET_SUCCESS;
}
