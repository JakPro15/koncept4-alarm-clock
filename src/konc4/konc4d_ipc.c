#include "konc4d_ipc.h"
#include "logging.h"
#include "input_loop.h"
#include "shared_memory.h"
#include "events.h"

#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <inttypes.h>


#define OPENING_DELAY_MS 200
#define OPENING_TIMEOUT_MS 3000
#define MAX_OPENING_ITERATIONS OPENING_TIMEOUT_MS / OPENING_DELAY_MS


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


static ReturnCode waitUntilKonc4dOn(void)
{
    ReturnCode checked;
    int i = 0;
    do
    {
        Sleep(OPENING_DELAY_MS);
        RETHROW(checked = isKonc4dOn());
    } while(checked != RET_SUCCESS && i++ < MAX_OPENING_ITERATIONS);

    if(i == MAX_OPENING_ITERATIONS)
    {
        LOG_LINE(LOG_ERROR, "Launching konc4d timed out");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode startKonc4d(void)
{
    RETURN_FAIL(checkKonc4dOff());

    LOG_LINE(LOG_DEBUG, "Launching konc4d");
    system("cd /d \"%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\\" && explorer konc4d.exe.lnk");

    ENSURE(waitUntilKonc4dOn());
    LOG_LINE(LOG_DEBUG, "konc4d launched properly");
    return RET_SUCCESS;
}


static enum CallbackReturn parseLaunchAnswer(char *answer)
{
    answer[strlen(answer) - 1] = '\0';
    LOG_LINE(LOG_DEBUG, "Answer received on whether to launch konc4d: %s", answer);
    if(answer[0] == '\0' || stricmp(answer, "y") == 0)
    {
        if(startKonc4d() != RET_SUCCESS)
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


ReturnCode notifyKonc4d(void)
{
    HANDLE event;
    ENSURE(openEventObject(&event, EVENT_TO_KONC4D));
    ENSURE(pingEventObject(event));
    CloseHandle(event);
    return RET_SUCCESS;
}


ReturnCode getKonc4dHandle(HANDLE *toWrite)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if(!Process32First(snapshot, &entry))
    {
        LOG_LINE(LOG_ERROR, "Process32First failed");
        return RET_ERROR;
    }
    do
    {
        if (stricmp(entry.szExeFile, "konc4d.exe") == 0)
        {
            *toWrite = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
            if(*toWrite == NULL)
            {
                LOG_LINE(LOG_ERROR, "Failed to open konc4d.exe process");
                return RET_ERROR;
            }
            CloseHandle(snapshot);
            return RET_SUCCESS;
        }
    } while(Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return RET_FAILURE;
}


ReturnCode duplicateHandleForKonc4d(HANDLE toDuplicate, HANDLE *toWrite)
{
    HANDLE ownHandle = GetCurrentProcess();
    HANDLE konc4dHandle;
    RETURN_FAIL(getKonc4dHandle(&konc4dHandle));
    if(!DuplicateHandle(ownHandle, toDuplicate, konc4dHandle, toWrite, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        LOG_LINE(LOG_ERROR, "DuplicateHandle failed");
        return RET_ERROR;
    }
    LOG_LINE(LOG_TRACE, "Duplicated handle %p into handle %p for process %p", ownHandle, *toWrite, konc4dHandle);
    return RET_SUCCESS;
}
