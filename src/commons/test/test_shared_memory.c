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
    char received[100];
    fgets(received, 100, receivedStream);
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
    char received[100];

    fgets(received, 100, receivedStream);
    ASSERT(strcmp(received, "Message\n") == 0);
    fgets(received, 100, receivedStream);
    ASSERT(strcmp(received, "Hello\n") == 0);
    fgets(received, 100, receivedStream);
    ASSERT(strcmp(received, "World!\n") == 0);

    ASSERT(pclose(receivedStream) == 0);
    return RET_SUCCESS;
}


static ReturnCode testOverQueueCapacity(void)
{
    FILE *receivedStream = popen("output\\receiver.exe 20", "r");
    ASSERT_MESSAGE(receivedStream != NULL, "Failed to launch receiver.exe");
    Sleep(100);
    char command[600] = "output\\sender.exe";
    for(int i = 0; i < 20; i++)
    {
        char nextMessage[30];
        sprintf(nextMessage, " messageMessage%d", i);
        strcpy(&command[strlen(command)], nextMessage);
    }
    ASSERT(system(command) == 0);
    char sent[30];
    char received[100];

    for(int i = 0; i < 20; i++)
    {
        sprintf(sent, "messageMessage%d\n", i);
        ASSERT(fgets(received, 100, receivedStream) != NULL);
        ASSERT(strcmp(received, sent) == 0);
    }
    ASSERT(pclose(receivedStream) == 0);
    return RET_SUCCESS;
}


static ReturnCode testSendMessageWithArgument(void)
{
    FILE *receivedStream = popen("..\\commons\\output\\receiver.exe 20", "r");
    ASSERT_MESSAGE(receivedStream != NULL, "Failed to launch receiver.exe");
    Sleep(100);

    struct SharedMemoryFile sharedMemory;
    ASSERT_ENSURE(openSharedMemory(&sharedMemory, SHMEM_TO_KONC4D));

    for(int i = 0; i < 20; i++)
        ASSERT_ENSURE(sendMessageWithArgument(sharedMemory, "SKIP", 20 - i, 2000));

    char sent[20], received[20];
    for(int i = 0; i < 20; i++)
    {
        sprintf(sent, "SKIP %d\n", 20 - i);
        ASSERT(fgets(received, sizeof(received), receivedStream) != NULL);
        ASSERT(strcmp(received, sent) == 0);
    }
    ASSERT(pclose(receivedStream) == 0);
    return RET_SUCCESS;
}


PREPARE_TESTING(shared_memory,
    testSingleMessage,
    testMultipleMessages,
    testOverQueueCapacity,
    testSendMessageWithArgument
)
