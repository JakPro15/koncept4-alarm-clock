#include "shared_memory.h"
#include "logging.h"

#include <windows.h>
#include <stdio.h>
#include <inttypes.h>


const enum LOGGING_LEVEL logging_level = LOG_SILENT;
const char logging_exe[7] = "recver";


int main(int argc, char *argv[])
{
    if(!SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS))
        LOG_LINE(LOG_ERROR, "SetPriorityClass failed\n");

    if(argc < 1)
        return 1;
    int toReceive = atoi(argv[1]);

    struct SharedMemoryFile sharedMemory;
    ENSURE(createSharedMemory(&sharedMemory, SHMEM_TO_KONC4D));
    char *received;
    uint64_t receivedArgument;
    for(int i = 0; i < toReceive; i++)
    {
        ENSURE_CALLBACK(receiveMessageWithArgument(sharedMemory, &received, &receivedArgument, 2000), closeSharedMemory(sharedMemory));

        if(strcmp(received, "SKIP") == 0)
            printf("%s %"PRIu64"\n", received, receivedArgument);
        else
            printf("%s\n", received);

        if(strcmp(received, "SKIP") == 0)
            LOG_LINE(LOG_DEBUG, "%s %"PRIu64"\n", received, receivedArgument);
        else
            LOG_LINE(LOG_DEBUG, "%s\n", received);
        free(received);
    }
    closeSharedMemory(sharedMemory);
}
