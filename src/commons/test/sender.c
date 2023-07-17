#include "shared_memory.h"
#include "stdio.h"


int main(int argc, char *argv[])
{
    struct SharedMemoryFile sharedMemory;
    ENSURE(openSharedMemory(&sharedMemory));
    ReturnCode sentCode;
    for(int i = 1; i < argc; i++)
    {
        RETHROW_CALLBACK(sentCode = sendMessage(sharedMemory, argv[i]), closeSharedMemory(sharedMemory));
        while(sentCode == RET_FAILURE)
        {
            Sleep(50);
            RETHROW_CALLBACK(sentCode = sendMessage(sharedMemory, argv[i]), closeSharedMemory(sharedMemory));
        }
    }
    closeSharedMemory(sharedMemory);
}
