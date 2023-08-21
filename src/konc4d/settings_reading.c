#include "settings_reading.h"
#include "logging.h"
#include "preprocessing.h"

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


static ReturnCode getNonemptyLine(FILE *settingsFile, struct SizedString *toWrite)
{
    ReturnCode lineReading;
    do {
        toWrite->size = 0;
        RETURN_FAIL(lineReading = getLine(settingsFile, toWrite));
    } while(strcmp(toWrite->data, "\r\n") == 0);
    return RET_SUCCESS;
}


static ReturnCode analyzeLine(struct SizedString line, struct ActionQueue **actions,
                              struct GatheredDefines preprocessingDefines, struct YearTimestamp now)
{
    TRY_END_RETHROW(fitDefine(line.data, line.size, actions, preprocessingDefines, now));
    struct Action newAction;
    ReturnCode parseResult;
    RETHROW(parseResult = parseAction(line.data, &newAction, now));
    if(parseResult == RET_FAILURE) // 29.02
        return RET_SUCCESS;
    ENSURE(addAction(actions, &newAction, now.timestamp));
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
    struct GatheredDefines preprocessingDefines;
    ENSURE_CALLBACK(gatherDefines(settingsFile, &preprocessingDefines), fclose(settingsFile));
    rewind(settingsFile);

    struct SizedString lineBuffer;
    ENSURE_CALLBACK(createSizedString(&lineBuffer), fclose(settingsFile); freeGatheredDefines(preprocessingDefines));

    ReturnCode getNonemptyLineResult;
    while(true)
    {
        RETHROW_CALLBACK(getNonemptyLineResult = getNonemptyLine(settingsFile, &lineBuffer),
                         fclose(settingsFile); freeGatheredDefines(preprocessingDefines); freeSizedString(lineBuffer));
        if(getNonemptyLineResult == RET_FAILURE)
            break;
        ENSURE_CALLBACK(analyzeLine(lineBuffer, toWrite, preprocessingDefines, now),
                        fclose(settingsFile); freeGatheredDefines(preprocessingDefines); freeSizedString(lineBuffer));
    }

    freeGatheredDefines(preprocessingDefines);
    freeSizedString(lineBuffer);
    if(fclose(settingsFile) != 0)
    {
        LOG_LINE(LOG_ERROR, "fclose failed");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode loadActions(struct ActionQueue **toWrite);
