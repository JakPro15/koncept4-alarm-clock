#include "command_execution.h"
#include "logging.h"
#include "shared_memory.h"
#include "input_loop.h"
#include "konc4d_starting.h"
#include "action_receiving.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#define SENDING_DELAY_MS 100
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
        fprintf(stderr, "Skip command expects a positive integer argument.\n");
        LOG_LINE(LOG_WARNING, "skip command received invalid argument: %d", minutesToSkip);
        return RET_FAILURE;
    }
    if(minutesToSkip > 7200)
    {
        fprintf(stderr, "Skipping more than five days at once is not supported.\n");
        LOG_LINE(LOG_WARNING, "skip command received invalid argument: %d", minutesToSkip);
        return RET_FAILURE;
    }
    RETHROW(fullSendMessage("SKIP", minutesToSkip));
    printf("Skip %d message sent.\n", minutesToSkip);
    LOG_LINE(LOG_INFO, "konc4 skip command executed successfully");
    return RET_SUCCESS;
}


static const char* actionType[3] = {"shutdown", "notify", "reset"};


ReturnCode executeShow(void)
{
    printf("Trying to obtain action data from konc4d.\n");
    struct PassedAction *actions;
    unsigned size;
    ReturnCode obtained;
    RETHROW(obtained = obtainActions(&actions, &size));
    if(obtained == RET_FAILURE)
    {
        printf("Failed to obtain action data from konc4d.\n");
        LOG_LINE(LOG_WARNING, "Failed to obtain actions from konc4d");
        return RET_FAILURE;
    }
    printf("Actions:\n");
    for(unsigned i = 0; i < size; i++)
    {
        printf("%d) {%02d.%02d %02d:%02d, type: %s, %s}\n", i,
               actions[i].timestamp.date.day, actions[i].timestamp.date.month,
               actions[i].timestamp.time.hour, actions[i].timestamp.time.minute,
               actionType[actions[i].type], actions[i].repeated ? "repeated" : "not repeated");
    }
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


static_assert(SHMEM_MESSAGE_LENGTH - sizeof(unsigned) > sizeof("SKIP"),
              "SHMEM_MESSAGE_LENGTH is too small to hold SKIP embedded argument");

void embedArgsInMessage(char *toWrite, const char *message, va_list args)
{
    strcpy(toWrite, message);
    if(strcmp(message, "SKIP") == 0)
    {
        unsigned minutesToSkip = va_arg(args, unsigned);
        SHMEM_EMBEDDED_UNSIGNED(toWrite) = minutesToSkip;
    }
}


ReturnCode ensuredSendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned length)
{
    LOG_LINE(LOG_DEBUG, "Sending message: %s", message);
    ReturnCode sent;
    RETHROW(sent = sendMessage(sharedMemory, message, length));
    while(sent == RET_FAILURE)
    {
        Sleep(SENDING_DELAY_MS);
        RETHROW(sent = sendMessage(sharedMemory, message, length));
    }
    return RET_SUCCESS;
}


ReturnCode fullSendMessage(char *message, ...)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));

    va_list args;
    va_start(args, message);
    char formattedMessage[SHMEM_MESSAGE_LENGTH];
    embedArgsInMessage(formattedMessage, message, args);
    va_end(args);

    ENSURE_CALLBACK(ensuredSendMessage(sharedMemory, formattedMessage, SHMEM_MESSAGE_LENGTH), closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    return RET_SUCCESS;
}
