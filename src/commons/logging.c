#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "logging.h"

#define LOG_MUTEX "konc4_log_mutex"
#define LOG_FILE_NAME "konc4log.txt"


static HANDLE mutex;
static const char *level_strings[6] = {"TRC", "DEB", "INF", "WAR", "ERR", "SIL"};


static ReturnCode outputLogMessage(char *message, unsigned int length)
{
    HANDLE logFile = CreateFile(
        LOG_FILE_NAME, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if(logFile == INVALID_HANDLE_VALUE)
        return RET_ERROR;

    DWORD bytesWritten;
    WINBOOL written = WriteFile(logFile, message, length, &bytesWritten, NULL);
    CloseHandle(logFile);

    if(bytesWritten != length || !written)
        return RET_ERROR;
    return RET_SUCCESS;
}


static char* getMessage(enum LOGGING_LEVEL level, SYSTEMTIME currentTime, int *messageLength, char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    // 8 chars for "konc4d: " 6 chars for "[ERR] ", 14 chars for the time, 1 for \n
    int len = vsnprintf(NULL, 0, format, args) + 29;
    char *buffer = malloc(len * sizeof(char));
    if(!buffer)
        return (char*) NULL;
    *messageLength = len;

    sprintf(buffer, "%6s: [%3s] %02d:%02d:%02d.%03d: ", logging_exe, level_strings[level], currentTime.wHour,
            currentTime.wMinute, currentTime.wSecond, currentTime.wMilliseconds);
    vsprintf(buffer + 28, format, args2);
    buffer[len - 1] = '\n';

    va_end(args2);
    return buffer;
}


static ReturnCode writeLog(enum LOGGING_LEVEL level, char *format, va_list args)
{
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);

    int messageLength;
    char *message = getMessage(level, currentTime, &messageLength, format, args);
    if(!message)
        return RET_ERROR;

    ENSURE_CALLBACK(outputLogMessage(message, messageLength), free(message));
    free(message);

    return RET_SUCCESS;
}


ReturnCode logLine(enum LOGGING_LEVEL level, char *format, ...)
{
    if(level < logging_level)
        return RET_SUCCESS;

    if(!mutex)
        mutex = CreateMutex(NULL, FALSE, LOG_MUTEX);
    if(WaitForSingleObject(mutex, 5000) != WAIT_OBJECT_0)
    {
        fprintf(stderr, "Failed to lock mutex for log writing\n");
        return RET_ERROR;
    }

    va_list args;
    va_start(args, format);
    ENSURE_CALLBACK(writeLog(level, format, args), ReleaseMutex(mutex); fprintf(stderr, "Failed to write log\n"));
    va_end(args);

    ReleaseMutex(mutex);
    return RET_SUCCESS;
}
