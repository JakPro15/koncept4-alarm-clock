#include "settings_reading.h"
#include "logging.h"
#include "preprocessing.h"

#include <stdbool.h>
#include <stdio.h>

#define SETTINGS_FILE_BUFFER_SIZE 4 * 1024  // fatter buffer to fit entire file in memory


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


static ReturnCode getCharacter(FILE *file, char *toWrite)
{
    *toWrite = fgetc(file);
    if(*toWrite != EOF)
        return RET_SUCCESS;
    if(feof(file))
        return RET_FAILURE;
    if(ferror(file))
    {
        LOG_LINE(LOG_ERROR, "Input/output error occurred");
        return RET_ERROR;
    }
    LOG_LINE(LOG_ERROR, "Should never be reached");
    return RET_ERROR;
}


static ReturnCode unGetCharacter(FILE *file, char toWrite)
{
    if(ungetc(toWrite, file) == EOF)
    {
        LOG_LINE(LOG_ERROR, "ungetc failed");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


static ReturnCode skipUntil(FILE *file, bool (*until)(char), char *lastCharacter)
{
    do
    {
        RETURN_FAIL(getCharacter(file, lastCharacter));
    } while(!until(*lastCharacter));
    return RET_SUCCESS;
}
static bool isPreprocessingDelimiter(char character) { return character == '#'; }
static bool isNotWhitespace(char character)          { return !isspace(character); }


// RET_FAILURE implies EOF reached
ReturnCode skipPreprocessingDirectives(FILE *settingsFile)
{
    char lastCharacter;
    RETURN_FAIL(skipUntil(settingsFile, isNotWhitespace, &lastCharacter));
    while(lastCharacter == '#')
    {
        RETURN_FAIL(skipUntil(settingsFile, isPreprocessingDelimiter, &lastCharacter));
        RETURN_FAIL(skipUntil(settingsFile, isNotWhitespace, &lastCharacter));
    }
    return unGetCharacter(settingsFile, lastCharacter);
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
    ENSURE(parseActionLine(line.data, actions, now));
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
    if(setvbuf(settingsFile, NULL, _IOFBF, SETTINGS_FILE_BUFFER_SIZE) != 0)
    {
        LOG_LINE(LOG_ERROR, "setvbuf failed");
        fclose(settingsFile);
        return RET_ERROR;
    }
    struct GatheredDefines preprocessingDefines;
    ENSURE_CALLBACK(gatherDefines(settingsFile, &preprocessingDefines), fclose(settingsFile));
    rewind(settingsFile);

    struct SizedString lineBuffer;
    ENSURE_CALLBACK(createSizedString(&lineBuffer), fclose(settingsFile); freeGatheredDefines(preprocessingDefines));

    ReturnCode getLineResult, skipResult;
    while(true)
    {
        RETHROW_CALLBACK(skipResult = skipPreprocessingDirectives(settingsFile),
                         fclose(settingsFile); freeGatheredDefines(preprocessingDefines); freeSizedString(lineBuffer));
        if(skipResult == RET_FAILURE)
            break;
        RETHROW_CALLBACK(getLineResult = getNonemptyLine(settingsFile, &lineBuffer),
                         fclose(settingsFile); freeGatheredDefines(preprocessingDefines); freeSizedString(lineBuffer));
        if(getLineResult == RET_FAILURE)
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
