#include <inttypes.h>

#include "shared_memory.h"
#include "logging.h"
#include "events.h"


static const char *shmemFiles[] = {"konc4d_shared_memory_write", "konc4d_shared_memory_read"};
static const char *shmemMutexes[] = {"konc4d_shared_memory_write_mutex", "konc4d_shared_memory_read_mutex"};
static const char *shmemWrittenEvents[] = {"konc4d_shared_memory_write_written_event", "konc4d_shared_memory_read_written_event"};
static const char *shmemReadEvents[] = {"konc4d_shared_memory_write_read_event", "konc4d_shared_memory_read_read_event"};


#define PRINT_INT(x) LOG_LINE(LOG_TRACE, "%s = %d", #x, x)
#define PRINT_STR(x) LOG_LINE(LOG_TRACE, "%s = %s", #x, x)

static void printSharedMemory(struct SharedMemory *sharedMemory)
{
    PRINT_INT(sharedMemory->queueFirst);
    PRINT_INT(sharedMemory->queueLast);
}


ReturnCode isKonc4dOn(void)
{
    HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, shmemMutexes[SHMEM_TO_KONC4D]);
    if(mutex == NULL)
    {
        if(GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            LOG_LINE(LOG_DEBUG, "konc4d is off");
            return RET_FAILURE;
        }
        else
        {
            LOG_LINE(LOG_ERROR, "Checking if konc4d is on failed (OpenMutex failed)");
            return RET_ERROR;
        }
    }
    LOG_LINE(LOG_DEBUG, "konc4d is on");
    CloseHandle(mutex);
    return RET_SUCCESS;
}


static ReturnCode initializeMutex(HANDLE *mutex, unsigned pipeNr)
{
    *mutex = CreateMutex(NULL, FALSE, shmemMutexes[pipeNr]);
    if(*mutex == NULL)
    {
        LOG_LINE(LOG_ERROR, "Could not create mutex");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static ReturnCode initializeEvents(struct SharedMemoryFile *sharedMemory, unsigned pipeNr)
{
    ENSURE(createEventObject(&sharedMemory->writtenEvent, shmemWrittenEvents[pipeNr]));
    ENSURE(createEventObject(&sharedMemory->readEvent, shmemReadEvents[pipeNr]));
    return RET_SUCCESS;
}


static ReturnCode waitMutex(struct SharedMemoryFile sharedMemory)
{
    if(WaitForSingleObject(sharedMemory.mutex, 5000) != WAIT_OBJECT_0)
    {
        LOG_LINE(LOG_ERROR, "Failed to lock mutex");
        closeSharedMemory(sharedMemory);
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static ReturnCode initializeMapFile(struct SharedMemoryFile *sharedMemory, unsigned pipeNr)
{
    sharedMemory->mapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
        sizeof(struct SharedMemory), shmemFiles[pipeNr]
    );
    if(sharedMemory->mapFile == NULL)
    {
        LOG_LINE(LOG_ERROR, "Could not create shared memory file mapping object");
        CloseHandle(sharedMemory->mutex);
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static ReturnCode openMapFile(struct SharedMemoryFile *sharedMemory, unsigned pipeNr)
{
    sharedMemory->mapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, FALSE, shmemFiles[pipeNr]
    );
    if(sharedMemory->mapFile == NULL)
    {
        LOG_LINE(LOG_ERROR, "Could not open shared memory file mapping object");
        CloseHandle(sharedMemory->mutex);
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static ReturnCode initializeMapView(struct SharedMemoryFile *sharedMemory)
{
    sharedMemory->shared = (struct SharedMemory *) MapViewOfFile(
        sharedMemory->mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(struct SharedMemory)
    );
    if(sharedMemory->shared == NULL)
    {
        LOG_LINE(LOG_ERROR, "Could not map view of shared memory file");
        CloseHandle(sharedMemory->mutex);
        CloseHandle(sharedMemory->mapFile);
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode createSharedMemory(struct SharedMemoryFile *sharedMemory, unsigned pipeNr)
{
    ENSURE(initializeMutex(&sharedMemory->mutex, pipeNr));
    ENSURE(initializeEvents(sharedMemory, pipeNr));
    ENSURE_CALLBACK(initializeMapFile(sharedMemory, pipeNr), CloseHandle(sharedMemory->mutex));
    ENSURE_CALLBACK(initializeMapView(sharedMemory), CloseHandle(sharedMemory->mapFile); CloseHandle(sharedMemory->mutex));
    sharedMemory->shared->queueFirst = NO_MESSAGE;
    return RET_SUCCESS;
}


ReturnCode openSharedMemory(struct SharedMemoryFile *sharedMemory, unsigned pipeNr)
{
    ENSURE(initializeMutex(&sharedMemory->mutex, pipeNr));
    ENSURE(initializeEvents(sharedMemory, pipeNr));
    ENSURE_CALLBACK(openMapFile(sharedMemory, pipeNr), CloseHandle(sharedMemory->mutex));
    ENSURE_CALLBACK(initializeMapView(sharedMemory), CloseHandle(sharedMemory->mapFile); CloseHandle(sharedMemory->mutex));
    return RET_SUCCESS;
}


ReturnCode receiveSizedMessage(struct SharedMemoryFile sharedMemory, char **buffer, unsigned *size)
{
    ENSURE(waitMutex(sharedMemory));
    int received = sharedMemory.shared->queueFirst;
    if(received == NO_MESSAGE)
    {
        LOG_LINE(LOG_TRACE, "Recv failed");
        ENSURE(resetEventObject(sharedMemory.writtenEvent));
        ReleaseMutex(sharedMemory.mutex);
        return RET_FAILURE;
    }

    *size = (unsigned char) sharedMemory.shared->messageQueue[received++];
    *buffer = malloc(*size + 1);
    if(*buffer == NULL)
    {
        LOG_LINE(LOG_ERROR, "Memory allocation failure");
        return RET_ERROR;
    }
    if(received + *size < SHMEM_QUEUE_SIZE)
        memcpy(*buffer, &sharedMemory.shared->messageQueue[received], *size);
    else
    {
        memcpy(*buffer, &sharedMemory.shared->messageQueue[received], SHMEM_QUEUE_SIZE - received);
        memcpy(&(*buffer)[SHMEM_QUEUE_SIZE - received], &sharedMemory.shared->messageQueue[0], *size - (SHMEM_QUEUE_SIZE - received));
    }
    (*buffer)[*size] = '\0';
    received += *size;
    received %= SHMEM_QUEUE_SIZE;

    if(received == sharedMemory.shared->queueLast)
        sharedMemory.shared->queueFirst = NO_MESSAGE;
    else
        sharedMemory.shared->queueFirst = received;

    LOG_LINE(LOG_TRACE, "Received message: %s", *buffer);
    printSharedMemory(sharedMemory.shared);
    ReleaseMutex(sharedMemory.mutex);
    ENSURE(pingEventObject(sharedMemory.readEvent));
    return RET_SUCCESS;
}


static int getNextFreeIndex(struct SharedMemory *shared, unsigned size)
{
    if(shared->queueFirst == NO_MESSAGE)
    {
        shared->queueFirst = 0;
        shared->queueLast = (int) size + 1;
        return 0;
    }
    int first = shared->queueFirst, last = shared->queueLast;
    int newLast = (shared->queueLast + size + 1) % SHMEM_QUEUE_SIZE;
    if((first < newLast && newLast < last) || (newLast < last && last < first) || (last < first && first < newLast))
        return NO_MESSAGE;
    shared->queueLast = newLast;
    return last;
}


ReturnCode sendSizedMessage(struct SharedMemoryFile sharedMemory, const char *message, unsigned size)
{
    if(size + 1 > SHMEM_QUEUE_SIZE)
    {
        LOG_LINE(LOG_ERROR, "Message to be sent is too long");
        return RET_ERROR;
    }

    ENSURE(waitMutex(sharedMemory));
    int begin = getNextFreeIndex(sharedMemory.shared, size), end = (begin + size) % SHMEM_QUEUE_SIZE;
    if(begin == NO_MESSAGE)
    {
        LOG_LINE(LOG_TRACE, "Send failed");
        ENSURE(resetEventObject(sharedMemory.readEvent));
        ReleaseMutex(sharedMemory.mutex);
        return RET_FAILURE;
    }

    sharedMemory.shared->messageQueue[begin++] = (unsigned char) size;
    begin %= SHMEM_QUEUE_SIZE;
    if(end > begin)
        memcpy(&sharedMemory.shared->messageQueue[begin], message, size);
    else
    {
        memcpy(&sharedMemory.shared->messageQueue[begin], message, SHMEM_QUEUE_SIZE - begin);
        memcpy(&sharedMemory.shared->messageQueue[0], &message[SHMEM_QUEUE_SIZE - begin], size - (SHMEM_QUEUE_SIZE - begin));
    }

    LOG_LINE(LOG_TRACE, "Sent message: %s", message);
    printSharedMemory(sharedMemory.shared);
    ReleaseMutex(sharedMemory.mutex);
    ENSURE(pingEventObject(sharedMemory.writtenEvent));
    return RET_SUCCESS;
}


void closeSharedMemory(struct SharedMemoryFile sharedMemory)
{
    UnmapViewOfFile(sharedMemory.shared);
    CloseHandle(sharedMemory.mapFile);
    CloseHandle(sharedMemory.mutex);
}


ReturnCode timeoutSendSizedMessage(struct SharedMemoryFile sharedMemory, const char *message,
                                   unsigned size, unsigned timeout)
{
    ReturnCode sent;
    RETHROW(sent = sendSizedMessage(sharedMemory, message, size));
    while(sent == RET_FAILURE)
    {
        RETHROW(sent = waitOnEventObject(sharedMemory.readEvent, timeout));
        if(sent == RET_FAILURE)
        {
            LOG_LINE(LOG_WARNING, "Timed out");
            return RET_FAILURE;
        }
        RETHROW(sent = sendSizedMessage(sharedMemory, message, size));
    }
    return RET_SUCCESS;
}


ReturnCode sendMessage(struct SharedMemoryFile sharedMemory, const char *message, unsigned timeout)
{
    unsigned length = strlen(message) + 1;
    LOG_LINE(LOG_DEBUG, "Sending message \"%s\" with length %u", message, length);
    return timeoutSendSizedMessage(sharedMemory, message, length, timeout);
}


ReturnCode sendMessageWithArgument(struct SharedMemoryFile sharedMemory, const char *message, uint64_t argument, unsigned timeout)
{
    unsigned length = strlen(message) + 1;
    unsigned size = length + sizeof(argument);
    char fullMessage[size];
    strcpy(fullMessage, message);
    SHMEM_EMBEDDED_UINT64(fullMessage, length) = argument;
    LOG_LINE(LOG_DEBUG, "Sending message \"%s\" with length %u and argument %"PRIu64" (total size: %u)",
             message, length, argument, size);
    return timeoutSendSizedMessage(sharedMemory, fullMessage, size, timeout);
}


ReturnCode timeoutReceiveSizedMessage(struct SharedMemoryFile sharedMemory, char **toWrite,
                                      unsigned *size, unsigned timeout)
{
    ReturnCode received;
    RETHROW(received = receiveSizedMessage(sharedMemory, toWrite, size));
    if(received == RET_FAILURE)
    {
        RETHROW(received = waitOnEventObject(sharedMemory.writtenEvent, timeout));
        if(received == RET_FAILURE)
        {
            LOG_LINE(LOG_WARNING, "Timed out");
            return RET_FAILURE;
        }
        ENSURE(receiveSizedMessage(sharedMemory, toWrite, size));
    }
    return RET_SUCCESS;
}


ReturnCode receiveMessage(struct SharedMemoryFile sharedMemory, char **message, unsigned timeout)
{
    unsigned size;
    RETURN_FAIL(timeoutReceiveSizedMessage(sharedMemory, message, &size, timeout));
    unsigned length = strlen(*message) + 1;
    if(size != length)
    {
        LOG_LINE(LOG_ERROR, "Expected argumentless message with length %u, received message of size %u: \"%s\"",
                 length, size, *message);
        return RET_ERROR;
    }
    LOG_LINE(LOG_DEBUG, "Received message \"%s\" with length %u", *message, length);
    return RET_SUCCESS;
}


ReturnCode receiveMessageWithArgument(struct SharedMemoryFile sharedMemory, char **message, uint64_t *argument, unsigned timeout)
{
    unsigned size;
    RETURN_FAIL(timeoutReceiveSizedMessage(sharedMemory, message, &size, timeout));
    unsigned length = strlen(*message) + 1;
    if(size == length + sizeof(*argument))
    {
        *argument = SHMEM_EMBEDDED_UINT64(*message, length);
        LOG_LINE(LOG_DEBUG, "Received message \"%s\" with length %u and argument %"PRIu64" (total size %u)",
                 *message, length, *argument, size);
        return RET_SUCCESS;
    }
    else if(size != length)
    {
        LOG_LINE(LOG_ERROR, "Message with invalid size received: \"%s\" of length %u, with size %u", *message, length, size);
        return RET_ERROR;
    }
    else
    {
        LOG_LINE(LOG_DEBUG, "Received message \"%s\" with length %u", *message, length);
        return RET_SUCCESS;
    }
}
