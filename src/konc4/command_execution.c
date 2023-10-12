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
    ENSURE(notifyKonc4d());
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
    ENSURE(notifyKonc4d());
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
    ENSURE(notifyKonc4d());
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


ReturnCode executeShow(const char *argument)
{
    struct ShowArgument parsedArgument;
    RETURN_FAIL(parseShowArgument(argument, &parsedArgument));

    HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE), duplicatedStdout;
    if(stdoutHandle == INVALID_HANDLE_VALUE)
    {
        LOG_LINE(LOG_ERROR, "GetStdHandle failed");
        return RET_ERROR;
    }
    ReturnCode duplicated;
    RETHROW(duplicated = duplicateHandleForKonc4d(stdoutHandle, &duplicatedStdout));
    if(duplicated == RET_FAILURE)
        RETURN_FAIL(promptForKonc4dStart());

    HANDLE confirmEvent;
    ENSURE(createEventObject(&confirmEvent, EVENT_COMMAND_CONFIRM));

    struct SharedMemoryFile sharedMemory;
    ENSURE(openSharedMemory(&sharedMemory, SHMEM_TO_KONC4D));
    ENSURE_CALLBACK(sendMessageWithArgument(sharedMemory, "SHOW", (uint64_t) duplicatedStdout, KONC4D_MAX_WAITING_MS),
                    closeSharedMemory(sharedMemory));
    ENSURE_CALLBACK(timeoutSendSizedMessage(sharedMemory, (char*) &parsedArgument, sizeof(parsedArgument), KONC4D_MAX_WAITING_MS),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(notifyKonc4d());

    ReturnCode received;
    RETHROW(received = waitOnEventObject(confirmEvent, KONC4D_MAX_WAITING_MS));
    if(received == RET_FAILURE)
    {
        LOG_LINE(LOG_ERROR, "Did not receive confirmation event from konc4d in show command");
        printf("Show command failed. Check logs for more information.");
        return RET_ERROR;
    }
    CloseHandle(confirmEvent);
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
