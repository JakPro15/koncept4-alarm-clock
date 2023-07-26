#include "command_execution.h"
#include "logging.h"
#include "shared_memory.h"
#include "input_loop.h"
#include "konc4d_starting.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#define SENDING_DELAY_MS 100


ReturnCode ensuredOpenSharedMemory(struct SharedMemoryFile *sharedMemory)
{
    ReturnCode isOn;
    RETHROW(isOn = isKonc4dOn());
    if(isOn == RET_FAILURE)
        RETURN_FAIL(promptForKonc4dStart());
    ENSURE(openSharedMemory(sharedMemory));
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
