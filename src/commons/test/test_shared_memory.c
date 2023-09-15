#include "testing.h"
#include <stdlib.h>
#include <stdio.h>
#include "shared_memory.h"


static ReturnCode testSingleMessage(void)
{
    FILE *receivedStream = popen("output\\receiver.exe 1", "r");
    ASSERT_MESSAGE(receivedStream != NULL, "Failed to launch receiver.exe");
    Sleep(100);
    ASSERT(system("output\\sender.exe Message") == 0);
    char received[SHMEM_MESSAGE_LENGTH];
    fgets(received, SHMEM_MESSAGE_LENGTH, receivedStream);
    ASSERT(strcmp(received, "Message\n") == 0);
    ASSERT(pclose(receivedStream) == 0);
    return RET_SUCCESS;
}


static ReturnCode testMultipleMessages(void)
{
    FILE *receivedStream = popen("output\\receiver.exe 3", "r");
    ASSERT_MESSAGE(receivedStream != NULL, "Failed to launch receiver.exe");
    Sleep(100);
    ASSERT(system("output\\sender.exe Message Hello World!") == 0);
    char received[SHMEM_MESSAGE_LENGTH];

    fgets(received, SHMEM_MESSAGE_LENGTH, receivedStream);
    ASSERT(strcmp(received, "Message\n") == 0);
    fgets(received, SHMEM_MESSAGE_LENGTH, receivedStream);
    ASSERT(strcmp(received, "Hello\n") == 0);
    fgets(received, SHMEM_MESSAGE_LENGTH, receivedStream);
    ASSERT(strcmp(received, "World!\n") == 0);

    ASSERT(pclose(receivedStream) == 0);
    return RET_SUCCESS;
}


static ReturnCode testOverQueueCapacity(void)
{
    if(SHMEM_QUEUE_LENGTH >= 20)
        return RET_FAILURE;
    FILE *receivedStream = popen("output\\receiver.exe 20", "r");
    ASSERT_MESSAGE(receivedStream != NULL, "Failed to launch receiver.exe");
    Sleep(100);
    ASSERT(system("output\\sender.exe 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19") == 0);
    char sent[4];
    char received[SHMEM_MESSAGE_LENGTH];

    for(int i = 0; i < 20; i++)
    {
        sprintf(sent, "%d\n", i);
        ASSERT(fgets(received, SHMEM_MESSAGE_LENGTH, receivedStream) != NULL);
        ASSERT(strcmp(received, sent) == 0);
    }
    ASSERT(pclose(receivedStream) == 0);
    return RET_SUCCESS;
}


PREPARE_TESTING(shared_memory,
    testSingleMessage,
    testMultipleMessages,
    testOverQueueCapacity
)
