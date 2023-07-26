#include "shared_memory.h"
#include "logging.h"
#include <windows.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    if(!SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS))
        LOG_LINE(LOG_ERROR, "SetPriorityClass failed\n");

    logging_level = LOG_DEBUG;
    if(argc < 1)
        return 1;
    int toReceive = atoi(argv[1]);

    struct SharedMemoryFile sharedMemory;
    ENSURE(createSharedMemory(&sharedMemory));
    char received[SHMEM_MESSAGE_LENGTH];
    ReturnCode receivedCode;

    for(int i = 0; i < toReceive; i++)
    {
        RETHROW_CALLBACK(receivedCode = receiveMessage(sharedMemory, received), closeSharedMemory(sharedMemory));
        int j = 0;
        while(receivedCode == RET_FAILURE && j++ < 40)
        {
            Sleep(50);
            RETHROW_CALLBACK(receivedCode = receiveMessage(sharedMemory, received), closeSharedMemory(sharedMemory));
        }
        if(j >= 40)
            return 1;

        if(strcmp(received, "SKIP") == 0)
            printf("%s %u\n", received, SHMEM_EMBEDDED_UNSIGNED(received));
        else
            printf("%s\n", received);

        if(strcmp(received, "SKIP") == 0)
            LOG_LINE(LOG_DEBUG, "%s %u\n", received, SHMEM_EMBEDDED_UNSIGNED(received));
        else
            LOG_LINE(LOG_DEBUG, "%s\n", received);
    }
    closeSharedMemory(sharedMemory);
}
