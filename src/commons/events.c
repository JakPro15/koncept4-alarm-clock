#include "events.h"
#include "logging.h"


ReturnCode createEventObject(HANDLE *toWrite, const char *name)
{
    *toWrite = CreateEvent(NULL, FALSE, FALSE, name);
    if(*toWrite == NULL)
    {
        LOG_LINE(LOG_ERROR, "CreateEvent failed");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode openEventObject(HANDLE *toWrite, const char *name)
{
    *toWrite = OpenEvent(SYNCHRONIZE | EVENT_ALL_ACCESS, FALSE, name);
    if(*toWrite == NULL)
    {
        LOG_LINE(LOG_DEBUG, "OpenEvent failed");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}


ReturnCode pingEventObject(HANDLE event)
{
    if(SetEvent(event))
        return RET_SUCCESS;
    else
    {
        LOG_LINE(LOG_ERROR, "SetEvent failed");
        return RET_ERROR;
    }
}


ReturnCode resetEventObject(HANDLE event)
{
    if(ResetEvent(event))
        return RET_SUCCESS;
    else
    {
        LOG_LINE(LOG_ERROR, "ResetEvent failed");
        return RET_ERROR;
    }
}


ReturnCode waitOnEventObject(HANDLE event, unsigned timeoutMs)
{
    DWORD returned = WaitForSingleObject(event, timeoutMs);
    if(returned == WAIT_OBJECT_0)
        return RET_SUCCESS;
    else if(returned == WAIT_TIMEOUT)
        return RET_FAILURE;
    else
    {
        LOG_LINE(LOG_ERROR, "WaitForSingleObject on event failed");
        return RET_ERROR;
    }
}


ReturnCode waitOnEventObjects(HANDLE *events, unsigned count, unsigned timeoutMs, unsigned *pingedIndex)
{
    DWORD returned = WaitForMultipleObjects(count, events, false, timeoutMs);
    if(returned == WAIT_TIMEOUT)
        return RET_FAILURE;
    if(returned == WAIT_FAILED)
    {
        LOG_LINE(LOG_ERROR, "WaitForMultipleObjects on events failed");
        return RET_ERROR;
    }
    *pingedIndex = returned - WAIT_OBJECT_0;
    return RET_SUCCESS;
}


ReturnCode sendNotification(const char *eventName)
{
    HANDLE event;
    if(openEventObject(&event, eventName) != RET_SUCCESS)
        return RET_FAILURE;
    ENSURE(pingEventObject(event));
    CloseHandle(event);
    return RET_SUCCESS;
}
