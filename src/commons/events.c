#include "events.h"


ReturnCode createEventObject(HANDLE *toWrite, const char *name)
{
    *toWrite = CreateEvent(NULL, FALSE, FALSE, name);
    if(*toWrite == NULL)
        return RET_ERROR;
    return RET_SUCCESS;
}


ReturnCode openEventObject(HANDLE *toWrite, const char *name)
{
    *toWrite = OpenEvent(SYNCHRONIZE, FALSE, name);
    if(*toWrite == NULL)
        return RET_ERROR;
    return RET_SUCCESS;
}


ReturnCode pingEventObject(HANDLE event)
{
    if(SetEvent(event))
        return RET_SUCCESS;
    else
        return RET_ERROR;
}


ReturnCode resetEventObject(HANDLE event)
{
    if(ResetEvent(event))
        return RET_SUCCESS;
    else
        return RET_ERROR;
}


ReturnCode waitOnEventObject(HANDLE event, unsigned timeoutMs)
{
    DWORD returned = WaitForSingleObject(event, timeoutMs);
    if(returned == WAIT_OBJECT_0)
    {
        ENSURE(resetEventObject(event));
        return RET_SUCCESS;
    }
    else if(returned == WAIT_TIMEOUT)
        return RET_FAILURE;
    else
        return RET_ERROR;
}
