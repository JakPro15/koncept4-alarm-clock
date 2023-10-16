#include "konc4d_ipc.h"
#include "logging.h"
#include "input_loop.h"
#include "shared_memory.h"
#include "events.h"

#include <stdio.h>
#include <string.h>


static ReturnCode checkKonc4dOff(void)
{
    ReturnCode checked;
    RETHROW(checked = isKonc4dOn());
    if(checked == RET_SUCCESS)
    {
        printf("konc4d is already on.\n");
        LOG_LINE(LOG_WARNING, "Tried to start konc4d when it is already started");
        return RET_FAILURE;
    }
    else
        return RET_SUCCESS;
}


ReturnCode startKonc4d(void)
{
    RETURN_FAIL(checkKonc4dOff());

    LOG_LINE(LOG_DEBUG, "Launching konc4d");
    system("cd /d \"%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\\" && explorer konc4d.exe.lnk");

    LOG_LINE(LOG_DEBUG, "konc4d launched properly");
    return RET_SUCCESS;
}


static enum CallbackReturn parseLaunchAnswer(char *answer)
{
    answer[strlen(answer) - 1] = '\0';
    LOG_LINE(LOG_DEBUG, "Answer received on whether to launch konc4d: %s", answer);
    if(answer[0] == '\0' || stricmp(answer, "y") == 0)
    {
        HANDLE startupEvent;
        if(createEventObject(&startupEvent, EVENT_KONC4D_STARTUP) != RET_SUCCESS)
            return END_SPINNING_ERROR;

        if(startKonc4d() != RET_SUCCESS)
            return END_SPINNING_ERROR;

        ReturnCode done = waitOnEventObject(startupEvent, EVENT_TIMEOUT);
        if(done == RET_FAILURE)
        {
            puts("konc4d did not start up - see logs for more information.");
            LOG_LINE(LOG_WARNING, "startKonc4d timed out; no startup detected");
            return END_SPINNING_FAILURE;
        }
        else if(done == RET_ERROR)
            return END_SPINNING_ERROR;
        return END_SPINNING_SUCCESS;
    }
    else if(stricmp(answer, "n") == 0)
        return END_SPINNING_FAILURE;
    else
        return KEEP_SPINNING;
}


ReturnCode promptForKonc4dStart(void)
{
    LOG_LINE(LOG_WARNING, "konc4d is off when trying to send message");
    RETURN_FAIL(parseInput(20, "konc4d is off - do you want to launch it? [Y/n]: ", parseLaunchAnswer));
    return RET_SUCCESS;
}


ReturnCode checkKonc4dResponse(HANDLE *events, const char *commandName, unsigned timeout)
{
    ReturnCode responseReceived;
    unsigned response;
    RETHROW(response = waitOnEventObjects(events, 2, timeout, &response));
    if(responseReceived == RET_FAILURE)
    {
        puts("Command not received by konc4d - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 %s command timed out", commandName);
        return RET_FAILURE;
    }
    else if(response == 0)
    {
        printf("%s command executed successfully.\n", commandName);
        LOG_LINE(LOG_INFO, "%s command executed successfully", commandName);
        return RET_SUCCESS;
    }
    else if(response == 1)
    {
        puts("Command failed in konc4d - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 %s command failed in konc4d", commandName);
        return RET_FAILURE;
    }
    else
    {
        LOG_LINE(LOG_ERROR, "Unreachable");
        return RET_ERROR;
    }
}


ReturnCode ensuredOpenSharedMemory(struct SharedMemoryFile *sharedMemory)
{
    ReturnCode isOn;
    RETHROW(isOn = isKonc4dOn());
    if(isOn == RET_FAILURE)
        RETURN_FAIL(promptForKonc4dStart());
    ENSURE(openSharedMemory(sharedMemory, SHMEM_TO_KONC4D));
    return RET_SUCCESS;
}


ReturnCode fullSendMessage(const char *message)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));
    ENSURE_CALLBACK(sendMessage(sharedMemory, message, SHMEM_TIMEOUT),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(sendNotification(EVENT_NOTIFY_KONC4D));
    return RET_SUCCESS;
}


ReturnCode fullSendMessageWithArgument(const char *message, uint64_t argument)
{
    struct SharedMemoryFile sharedMemory;
    RETURN_FAIL(ensuredOpenSharedMemory(&sharedMemory));
    ENSURE_CALLBACK(sendMessageWithArgument(sharedMemory, message, argument, SHMEM_TIMEOUT),
                    closeSharedMemory(sharedMemory));
    closeSharedMemory(sharedMemory);
    ENSURE(sendNotification(EVENT_NOTIFY_KONC4D));
    return RET_SUCCESS;
}
