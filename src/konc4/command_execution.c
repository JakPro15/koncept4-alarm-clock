#include "command_execution.h"
#include "logging.h"
#include "shared_memory.h"

#include <stdio.h>

#define SENDING_DELAY_MS 100


ReturnCode startKonc4d(void)
{
    if(system("cd /d \"%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\\" && explorer konc4d.exe.lnk") != 0)
    {
        LOG_LINE(LOG_ERROR, "Failed to launch konc4d");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode ensuredSendMessage(char *message)
{
    struct SharedMemoryFile sharedMemory;
    ENSURE(openSharedMemory(&sharedMemory));

    LOG_LINE(LOG_DEBUG, "Sending message: %s", message);
    ReturnCode sent;
    RETHROW(sent = sendMessage(sharedMemory, message));
    while(sent == RET_FAILURE)
    {
        Sleep(SENDING_DELAY_MS);
        RETHROW(sent = sendMessage(sharedMemory, message));
    }
    closeSharedMemory(sharedMemory);
    return RET_SUCCESS;
}
