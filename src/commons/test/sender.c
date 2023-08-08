#include "shared_memory.h"
#include "stdio.h"
#include "logging.h"


int main(int argc, char *argv[])
{
    logging_level = LOG_SILENT;
    struct SharedMemoryFile sharedMemory;
    ENSURE(openSharedMemory(&sharedMemory));
    ReturnCode sentCode;
    for(int i = 1; i < argc; i++)
    {
        RETHROW_CALLBACK(sentCode = sendMessage(sharedMemory, argv[i], strlen(argv[i]) + 1), closeSharedMemory(sharedMemory));
        int j = 0;
        while(sentCode == RET_FAILURE && j++ < 40)
        {
            Sleep(50);
            RETHROW_CALLBACK(sentCode = sendMessage(sharedMemory, argv[i], strlen(argv[i]) + 1), closeSharedMemory(sharedMemory));
        }
        if(j >= 40)
            return 1;
    }
    closeSharedMemory(sharedMemory);
}
