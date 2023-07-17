#include <stdbool.h>
#include "settings_reading.h"
#include "logging.h"


ReturnCode openBufferedFile(struct BufferedFile *toWrite, char *fileName)
{
    toWrite->handle = CreateFile(
        fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if(toWrite->handle == INVALID_HANDLE_VALUE)
    {
        LOG_LINE(LOG_ERROR, "Failed to open %s file.", fileName);
        return RET_ERROR;
    }
    toWrite->buffIndex = 0;
    toWrite->buffEnd = 0;
    return RET_SUCCESS;
}


ReturnCode closeBufferedFile(struct BufferedFile *file)
{
    if(!CloseHandle(file->handle))
    {
        LOG_LINE(LOG_ERROR, "CloseHandle failed");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode getCharacter(struct BufferedFile *file, char *toWrite)
{
    if(file->buffIndex >= file->buffEnd)
    {
        DWORD bytesRead = 0;
        if(!ReadFile(file->handle, file->buffer, 256, &bytesRead, NULL))
        {
            LOG_LINE(LOG_ERROR, "Failed to read file");
            return RET_ERROR;
        }
        if(bytesRead == 0)
            return RET_FAILURE;

        file->buffIndex = 0;
        file->buffEnd = bytesRead;
    }
    *toWrite = file->buffer[file->buffIndex++];
    return RET_SUCCESS;
}


ReturnCode getLine(struct BufferedFile *file, struct SizedString *toWrite)
{
    char character;
    ReturnCode code;
    bool first = true;
    while(true)
    {
        RETHROW(code = getCharacter(file, &character));
        if(code == RET_FAILURE)
        {
            if(first)
                return RET_FAILURE;
            else
                break;
        }
        if(character == '\r')
            continue;
        if(character == '\n')
            break;
        ENSURE(appendToSizedString(toWrite, character));
        first = false;
    }
    ENSURE(appendToSizedString(toWrite, '\0'));
    return RET_SUCCESS;
}


ReturnCode getNextAction(struct BufferedFile *settingsFile, struct Action *toWrite, struct SizedString *buffer,
                         struct YearTimestamp now)
{
    ReturnCode lineReading;
    do {
        buffer->size = 0;
        RETHROW(lineReading = getLine(settingsFile, buffer));
        if(lineReading == RET_FAILURE)
        {
            toWrite->timestamp.date = (struct DateOfYear) {0, 0};
            return RET_FAILURE;
        }
    } while(strcmp(buffer->data, "") == 0);
    RETURN_FAIL(parseAction(buffer->data, toWrite, now));
    return RET_SUCCESS;
}


ReturnCode loadActionsFromFile(struct ActionQueue **toWrite, char *fileName, struct YearTimestamp now)
{
    struct BufferedFile settingsFile;
    ENSURE(openBufferedFile(&settingsFile, fileName));
    struct SizedString lineBuffer;
    ENSURE_CALLBACK(createSizedString(&lineBuffer), closeBufferedFile(&settingsFile));

    struct Action newAction;
    ReturnCode readLine;
    do
    {
        RETHROW_CALLBACK(readLine = getNextAction(&settingsFile, &newAction, &lineBuffer, now),
                         freeSizedString(lineBuffer); closeBufferedFile(&settingsFile));
        if(readLine == RET_FAILURE)
        {
            if(newAction.timestamp.date.day == 29 && newAction.timestamp.date.month == 2)
                continue;
            else
                break;
        }
        ENSURE_CALLBACK(addAction(toWrite, &newAction, now.timestamp),
                        freeSizedString(lineBuffer); closeBufferedFile(&settingsFile));
    } while(true);

    freeSizedString(lineBuffer);
    ENSURE(closeBufferedFile(&settingsFile));
    if(readLine == RET_ERROR)
        return RET_ERROR;
    else
        return RET_SUCCESS;
}


ReturnCode loadActions(struct ActionQueue **toWrite);
