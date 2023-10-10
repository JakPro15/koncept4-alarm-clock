#include "shared_memory.h"
#include "stdio.h"
#include "logging.h"


const enum LOGGING_LEVEL logging_level = LOG_SILENT;
const char logging_exe[7] = "sender";


int main(int argc, char *argv[])
{
    struct SharedMemoryFile sharedMemory;
    ENSURE(openSharedMemory(&sharedMemory, SHMEM_TO_KONC4D));
    for(int i = 1; i < argc; i++)
        ENSURE_CALLBACK(sendMessage(sharedMemory, argv[i], 2000), closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
}
