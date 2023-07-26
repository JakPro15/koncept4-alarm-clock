#include "testing.h"
#include "command_execution.h"
#include "shared_memory.h"

#include <stdarg.h>


void embedArgsInMessage(char *toWrite, const char *message, va_list args);


ReturnCode testEmbedArgsInMessageNormal(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
    va_list unused;
    const char message[] = "STOP";
    char embedded[SHMEM_MESSAGE_LENGTH];
    for(int i = 0; i < SHMEM_MESSAGE_LENGTH; i++)
        embedded[i] = '\xff';

    embedArgsInMessage(embedded, message, unused);
    ASSERT(strcmp(embedded, message) == 0);
    for(int i = sizeof(message); i < SHMEM_MESSAGE_LENGTH; i++)
        ASSERT(embedded[i] == '\xff');
    return RET_SUCCESS;
#pragma GCC diagnostic pop
}


static void embedArgsInMessageWrapper(char *toWrite, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    embedArgsInMessage(toWrite, message, args);
    va_end(args);
}


ReturnCode testEmbedArgsInMessageSkip(void)
{
    const char message[] = "SKIP";
    char embedded[SHMEM_MESSAGE_LENGTH];
    for(int i = 0; i < SHMEM_MESSAGE_LENGTH; i++)
        embedded[i] = '\xff';

    embedArgsInMessageWrapper(embedded, message, 233);
    ASSERT(strcmp(embedded, message) == 0);
    for(int i = sizeof(message); (unsigned) i < SHMEM_MESSAGE_LENGTH - sizeof(unsigned); i++)
        ASSERT(embedded[i] == '\xff');
    ASSERT(SHMEM_EMBEDDED_UNSIGNED(embedded) == 233);
    return RET_SUCCESS;
}


ReturnCode testEnsuredSendMessage(void)
{
    system("powershell.exe rm konc4log.txt");
    FILE *receivedStream = popen("..\\commons\\output\\receiver.exe 20", "r");
    ASSERT_MESSAGE(receivedStream != NULL, "Failed to launch receiver.exe");
    Sleep(100);

    struct SharedMemoryFile sharedMemory;
    ASSERT_ENSURE(openSharedMemory(&sharedMemory));
    char message[SHMEM_MESSAGE_LENGTH] = "SKIP";

    for(int i = 0; i < 20; i++)
    {
        SHMEM_EMBEDDED_UNSIGNED(message) = 20 - i;
        ASSERT_ENSURE(ensuredSendMessage(sharedMemory, message, SHMEM_MESSAGE_LENGTH));
    }

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


PREPARE_TESTING(command_execution,
    testEmbedArgsInMessageNormal,
    testEmbedArgsInMessageSkip,
    testEnsuredSendMessage
)
