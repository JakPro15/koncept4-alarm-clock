#include "shared_memory.h"
#include "logging.h"


#define PRINT_INT(x) LOG_LINE(LOG_TRACE, "%s = %d", #x, x)
#define PRINT_STR(x) LOG_LINE(LOG_TRACE, "%s = %s", #x, x)

static void printSharedMemory(struct SharedMemory *sharedMemory)
{
    PRINT_INT(sharedMemory->queueFirst);
    PRINT_INT(sharedMemory->queueLast);
    PRINT_STR(sharedMemory->messageQueue[0]);
    PRINT_STR(sharedMemory->messageQueue[1]);
    PRINT_STR(sharedMemory->messageQueue[2]);
    PRINT_STR(sharedMemory->messageQueue[3]);
    PRINT_STR(sharedMemory->messageQueue[4]);
    PRINT_STR(sharedMemory->messageQueue[5]);
    PRINT_STR(sharedMemory->messageQueue[6]);
    PRINT_STR(sharedMemory->messageQueue[7]);
    LOG_LINE(LOG_TRACE, "");
}


ReturnCode isKonc4dOn(void)
{
    HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, SHMEM_MUTEX_NAME);
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


static ReturnCode initializeMutex(HANDLE *mutex)
{
    *mutex = CreateMutex(NULL, FALSE, SHMEM_MUTEX_NAME);
    if(*mutex == NULL)
    {
        LOG_LINE(LOG_ERROR, "Could not create mutex");
        return RET_ERROR;
    }
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


static ReturnCode initializeMapFile(struct SharedMemoryFile *sharedMemory)
{
    sharedMemory->mapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
        sizeof(struct SharedMemory), SHMEM_FILE_NAME
    );
    if(sharedMemory->mapFile == NULL)
    {
        LOG_LINE(LOG_ERROR, "Could not create shared memory file mapping object");
        CloseHandle(sharedMemory->mutex);
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static ReturnCode openMapFile(struct SharedMemoryFile *sharedMemory)
{
    sharedMemory->mapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, FALSE, SHMEM_FILE_NAME
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


ReturnCode createSharedMemory(struct SharedMemoryFile *sharedMemory)
{
    ENSURE(initializeMutex(&sharedMemory->mutex));
    ENSURE_CALLBACK(initializeMapFile(sharedMemory), CloseHandle(sharedMemory->mutex));
    ENSURE_CALLBACK(initializeMapView(sharedMemory), CloseHandle(sharedMemory->mapFile); CloseHandle(sharedMemory->mutex));
    sharedMemory->shared->queueFirst = NO_NODE;
    return RET_SUCCESS;
}


ReturnCode openSharedMemory(struct SharedMemoryFile *sharedMemory)
{
    ENSURE(initializeMutex(&sharedMemory->mutex));
    ENSURE_CALLBACK(openMapFile(sharedMemory), CloseHandle(sharedMemory->mutex));
    ENSURE_CALLBACK(initializeMapView(sharedMemory), CloseHandle(sharedMemory->mapFile); CloseHandle(sharedMemory->mutex));
    return RET_SUCCESS;
}


ReturnCode receiveMessage(struct SharedMemoryFile sharedMemory, char *buffer)
{
    ENSURE(waitMutex(sharedMemory));
    int received = sharedMemory.shared->queueFirst;
    if(received == NO_NODE)
    {
        LOG_LINE(LOG_TRACE, "Recv failed");
        ReleaseMutex(sharedMemory.mutex);
        return RET_FAILURE;
    }

    memcpy(buffer, sharedMemory.shared->messageQueue[received], SHMEM_MESSAGE_LENGTH);

    if(received == sharedMemory.shared->queueLast)
        sharedMemory.shared->queueFirst = NO_NODE;
    else
    {
        ++sharedMemory.shared->queueFirst;
        sharedMemory.shared->queueFirst %= SHMEM_QUEUE_LENGTH;
    }

    LOG_LINE(LOG_DEBUG, "Received message: %s", buffer);
    printSharedMemory(sharedMemory.shared);
    ReleaseMutex(sharedMemory.mutex);
    return RET_SUCCESS;
}


static int getNextFreeIndex(struct SharedMemory *shared)
{
    int next;
    if(shared->queueFirst == NO_NODE)
    {
        next = 0;
        shared->queueFirst = 0;
    }
    else
    {
        next = (shared->queueLast + 1) % SHMEM_QUEUE_LENGTH;
        if(next == shared->queueFirst)
            return NO_NODE;
    }
    return next;
}


ReturnCode sendMessage(struct SharedMemoryFile sharedMemory, char *message, unsigned length)
{
    if(length > SHMEM_MESSAGE_LENGTH)
    {
        LOG_LINE(LOG_ERROR, "Message to be sent is too long");
        return RET_ERROR;
    }

    ENSURE(waitMutex(sharedMemory));
    int next = getNextFreeIndex(sharedMemory.shared);
    if(next == NO_NODE)
    {
        LOG_LINE(LOG_ERROR, "Send failed");
        ReleaseMutex(sharedMemory.mutex);
        return RET_FAILURE;
    }

    sharedMemory.shared->queueLast = next;
    memcpy(sharedMemory.shared->messageQueue[next], message, length);

    LOG_LINE(LOG_DEBUG, "Sent message: %s", message);
    printSharedMemory(sharedMemory.shared);
    ReleaseMutex(sharedMemory.mutex);
    return RET_SUCCESS;
}


void closeSharedMemory(struct SharedMemoryFile sharedMemory)
{
    UnmapViewOfFile(sharedMemory.shared);
    CloseHandle(sharedMemory.mapFile);
    CloseHandle(sharedMemory.mutex);
}
