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

#define SENDING_DELAY_MS 500
#define KONC4D_WAITING_DELAY_MS 500
#define KONC4D_MAX_WAITING_MS 5000
#define KONC4D_MAX_ATTEMPTS KONC4D_MAX_WAITING_MS / KONC4D_WAITING_DELAY_MS


ReturnCode executeStart(void)
{
    RETURN_FAIL(startKonc4d());
    printf("konc4d starting command executed, waiting for actual konc4d startup.\n");
    ReturnCode isOn;
    unsigned attempts = 0;
    do
    {
        Sleep(KONC4D_WAITING_DELAY_MS);
        RETHROW(isOn = isKonc4dOn());
        if(++attempts >= KONC4D_MAX_ATTEMPTS)
        {
            LOG_LINE(LOG_WARNING, "Failed to start konc4d");
            printf("Failed to start konc4d. Check konc4log.txt for more information.\n");
            return RET_FAILURE;
        }
    } while(isOn == RET_FAILURE);
    printf("konc4d successfully started.\n");
    LOG_LINE(LOG_INFO, "konc4 start command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeStop(void)
{
    RETURN_FAIL(fullSendMessage("STOP"));
    printf("Stop message sent. Waiting for konc4d to receive it and shut down.\n");
    ReturnCode isOn;
    unsigned attempts = 0;
    do
    {
        Sleep(KONC4D_WAITING_DELAY_MS);
        RETHROW(isOn = isKonc4dOn());
        if(++attempts >= KONC4D_MAX_ATTEMPTS)
        {
            LOG_LINE(LOG_WARNING, "Failed to stop konc4d");
            printf("Failed to stop konc4d. Check konc4log.txt for more information.\n");
            return RET_FAILURE;
        }
    } while(isOn == RET_SUCCESS);
    printf("konc4d successfully stopped.\n");
    LOG_LINE(LOG_INFO, "konc4 stop command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeReset(void)
{
    printf("Beware that reset of konc4d will cancel any further pending messages to konc4d.\n");
    RETHROW(fullSendMessage("RESET"));
    printf("Reset message sent.\n");
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
    RETHROW(fullSendMessageWithArgument("SKIP", minutesToSkip));
    printf("Skip %d message sent.\n", minutesToSkip);
    LOG_LINE(LOG_INFO, "konc4 skip command executed successfully");
    return RET_SUCCESS;
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
        puts("No further shutdowns will be made");
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

    printf("Trying to obtain action data from konc4d.\n");
    struct ReceivedActions received;
    ReturnCode obtained;
    RETHROW(obtained = obtainActions(&received));
    if(obtained == RET_FAILURE)
    {
        printf("Failed to obtain action data from konc4d.\n");
        LOG_LINE(LOG_WARNING, "Failed to obtain actions from konc4d");
        return RET_FAILURE;
    }
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
    ENSURE_CALLBACK(sendMessage(sharedMemory, message, KONC4D_MAX_WAITING_MS),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(notifyKonc4d());
    return RET_SUCCESS;
}


ReturnCode fullSendMessageWithArgument(const char *message, uint64_t argument)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));
    ENSURE_CALLBACK(sendMessageWithArgument(sharedMemory, message, argument, KONC4D_MAX_WAITING_MS),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(notifyKonc4d());
    return RET_SUCCESS;
}
