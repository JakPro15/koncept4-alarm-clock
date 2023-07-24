#include "command_execution.h"
#include "logging.h"
#include "shared_memory.h"
#include "input_loop.h"
#include "konc4d_starting.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

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


ReturnCode ensuredSendMessage(char *message, ...)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));

    unsigned length;
    if(strcmp(message, "SKIP") == 0)
    {
        va_list args;
        va_start(args, message);
        unsigned minutesToSkip = va_arg(args, unsigned);
        va_end(args);

        char newMessage[SHMEM_MESSAGE_LENGTH];
        strcpy(newMessage, message);
        *((unsigned*) &newMessage[SHMEM_MESSAGE_LENGTH - sizeof(unsigned)]) = minutesToSkip;

        message = newMessage;
        length = SHMEM_MESSAGE_LENGTH;
    }
    else
        length = strlen(message) + 1;

    LOG_LINE(LOG_DEBUG, "Sending message: %s", message);
    ReturnCode sent;
    RETHROW(sent = sendMessage(sharedMemory, message, length));
    while(sent == RET_FAILURE)
    {
        Sleep(SENDING_DELAY_MS);
        RETHROW(sent = sendMessage(sharedMemory, message, length));
    }
    closeSharedMemory(sharedMemory);
    return RET_SUCCESS;
}
