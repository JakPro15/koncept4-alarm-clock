#include "shared_memory.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
    if(argc < 1)
        return 1;
    int toReceive = atoi(argv[1]);

    struct SharedMemoryFile sharedMemory;
    ENSURE(createSharedMemory(&sharedMemory));
    char received[SHMEM_MESSAGE_LENGTH];
    ReturnCode receivedCode;
    for(int i = 0; i < toReceive; i++)
    {
        RETHROW(receivedCode = receiveMessage(sharedMemory, received));
        while(receivedCode == RET_FAILURE)
        {
            Sleep(500);
            RETHROW(receivedCode = receiveMessage(sharedMemory, received));
        }
        printf("%s\n", received);
    }
    closeSharedMemory(sharedMemory);
}
