#include "settings_reading.h"
#include "logging.h"

#include <stdbool.h>
#include <stdio.h>


ReturnCode getLine(FILE *file, struct SizedString *toWrite)
{
    int firstUnreadOffset = 0;
    int sizeToWrite = toWrite->capacity;
    bool firstRead = true;
    while(fgets(toWrite->data + firstUnreadOffset, sizeToWrite, file) != NULL)
    {
        toWrite->size = strlen(toWrite->data) + 1;
        if(feof(file) || toWrite->data[toWrite->size - 2] == '\n')
            return RET_SUCCESS;
        if(firstRead)
        {
            firstUnreadOffset += toWrite->capacity - 1;
            sizeToWrite = CAPACITY_INCREMENT + 1;
            firstRead = false;
        }
        else
            firstUnreadOffset += CAPACITY_INCREMENT;
        ENSURE(increaseSizedStringCapacity(toWrite));
    }
    if(ferror(file))
    {
        LOG_LINE(LOG_ERROR, "Error on reading from file");
        return RET_ERROR;
    }
    return RET_FAILURE;
}


ReturnCode getNextAction(FILE *settingsFile, struct Action *toWrite, struct SizedString *buffer,
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
    } while(strcmp(buffer->data, "\r\n") == 0);
    RETURN_FAIL(parseAction(buffer->data, toWrite, now));
    return RET_SUCCESS;
}


ReturnCode loadActionsFromFile(struct ActionQueue **toWrite, char *fileName, struct YearTimestamp now)
{
    FILE *settingsFile;
    if((settingsFile = fopen(fileName, "rb")) == NULL)
    {
        LOG_LINE(LOG_ERROR, "Failed to open settings file %s", fileName);
        return RET_ERROR;
    }
    struct SizedString lineBuffer;
    ENSURE_CALLBACK(createSizedString(&lineBuffer), fclose(settingsFile));

    struct Action newAction;
    ReturnCode readLine;
    do
    {
        RETHROW_CALLBACK(readLine = getNextAction(settingsFile, &newAction, &lineBuffer, now),
                         freeSizedString(lineBuffer); fclose(settingsFile));
        if(readLine == RET_FAILURE)
        {
            if(newAction.timestamp.date.day == 29 && newAction.timestamp.date.month == 2)
                continue;
            else
                break;
        }
        ENSURE_CALLBACK(addAction(toWrite, &newAction, now.timestamp),
                        freeSizedString(lineBuffer); fclose(settingsFile));
    } while(true);

    freeSizedString(lineBuffer);
    if(fclose(settingsFile) != 0)
    {
        LOG_LINE(LOG_ERROR, "fclose failed");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode loadActions(struct ActionQueue **toWrite);
