#include "preprocessing.h"
#include "settings_reading.h"
#include "logging.h"

#include <stdlib.h>

ReturnCode verifyDefineName(const char *name, unsigned *size)
{
    *size = 1;
    if(*name == '\0')
    {
        LOG_LINE(LOG_ERROR, "Empty define name encountered");
        return RET_ERROR;
    }
    const char *namePointer = name;
    char character;
    while((character = *(namePointer++)) != '\0')
    {
        if(isspace(character))
        {
            LOG_LINE(LOG_ERROR, "Invalid define name (contains whitespace character): %s", name);
            return RET_ERROR;
        }
        if(character == '#' || character == ',' || character == '(' || character == ')' || character == '$')
        {
            LOG_LINE(LOG_ERROR, "Invalid define name (contains %c character): %s", character, name);
            return RET_ERROR;
        }
        ++(*size);
    }
    return RET_SUCCESS;
}


static ReturnCode loadDefineBody(FILE *settingsFile, struct SizedString *defineBody)
{
    char character;
    while(character = fgetc(settingsFile), character != EOF && character != '#')
        ENSURE(appendToSizedString(defineBody, character));

    if(ferror(settingsFile))
    {
        LOG_LINE(LOG_ERROR, "Read error on settings file");
        return RET_ERROR;
    }
    ENSURE(appendToSizedString(defineBody, '\0'));
    return RET_SUCCESS;
}


static ReturnCode addGatheredDefine(struct GatheredDefines *toWrite, unsigned *definesCapacity, struct StringPair newDefine)
{
    ++toWrite->size;
    if(toWrite->size > *definesCapacity)
    {
        *definesCapacity += 8;
        toWrite->defines = realloc(toWrite->defines, *definesCapacity);
        if(toWrite->defines == NULL)
        {
            LOG_LINE(LOG_ERROR, "Memory allocation error");
            return RET_ERROR;
        }
    }
    toWrite->defines[toWrite->size - 1] = newDefine;
    return RET_SUCCESS;
}


struct StringPair* findDefine(struct GatheredDefines defines, const char *name)
{
    for(unsigned i = 0; i < defines.size; i++)
    {
        if(strcmp(defines.defines[i].key, name) == 0)
            return &defines.defines[i];
    }
    return NULL;
}


static ReturnCode loadDefine(FILE *settingsFile, struct GatheredDefines *toWrite, const char *name, unsigned *definesCapacity)
{
    unsigned nameSize;
    ENSURE(verifyDefineName(name, &nameSize));
    if(findDefine(*toWrite, name) != NULL)
    {
        LOG_LINE(LOG_ERROR, "Duplicated define name: %s", name);
        return RET_ERROR;
    }

    char *key = malloc(nameSize);
    if(key == NULL)
    {
        LOG_LINE(LOG_ERROR, "Memory allocation error");
        return RET_ERROR;
    }
    strcpy(key, name);

    struct SizedString value;
    ENSURE_CALLBACK(createSizedString(&value), free(key));
    ENSURE_CALLBACK(loadDefineBody(settingsFile, &value),
                    free(key); freeSizedString(value));

    ENSURE_CALLBACK(addGatheredDefine(toWrite, definesCapacity, (struct StringPair) {key, value.data, value.size}),
                    free(key); freeSizedString(value));
    return RET_SUCCESS;
}


ReturnCode gatherDefines(FILE *settingsFile, struct GatheredDefines *toWrite)
{
    unsigned gatheredDefinesCapacity = 8;
    toWrite->defines = malloc(gatheredDefinesCapacity * sizeof(*toWrite->defines));
    if(toWrite->defines == NULL)
    {
        LOG_LINE(LOG_ERROR, "Memory allocation error");
        return RET_ERROR;
    }
    toWrite->size = 0;
    ReturnCode getLineResult = RET_FAILURE;
    struct SizedString lineBuffer;
    ENSURE(createSizedString(&lineBuffer));

    RETHROW_CALLBACK(getLineResult = getLine(settingsFile, &lineBuffer), freeSizedString(lineBuffer));
    while(getLineResult != RET_FAILURE)
    {
        if(strncmp(lineBuffer.data, "#define ", 8) == 0)
        {
            char *defineName = lineBuffer.data + 8;
            lineBuffer.data[lineBuffer.size - 3] = '\0';
            lineBuffer.size -= 2;
            skipWhitespace(&defineName);
            ENSURE_CALLBACK(loadDefine(settingsFile, toWrite, defineName, &gatheredDefinesCapacity), freeSizedString(lineBuffer));
        }
        RETHROW_CALLBACK(getLineResult = getLine(settingsFile, &lineBuffer), freeSizedString(lineBuffer));
    }

    freeSizedString(lineBuffer);
    return RET_SUCCESS;
}


ReturnCode fitDefine(const char *settingsLine, unsigned size, struct AllActions *actions,
                     struct GatheredDefines defines, struct YearTimestamp now)
{
    if(size > 2 && settingsLine[0] == '/' && settingsLine[1] == '/')
        return RET_SUCCESS;

    char defineInvocation[size];
    strcpy(defineInvocation, settingsLine);
    if(defineInvocation[size - 2] == '\n')
    {
        defineInvocation[size - 3] = '\0';
        size -= 2;
    }

    char *saveptr, *defineName = strtok_r(defineInvocation, "(),", &saveptr), *arguments[10];
    for(int i = 0; i < 9; i++)
        arguments[i] = strtok_r(NULL, "(),", &saveptr);

    struct StringPair *defineBodyOriginal = findDefine(defines, defineName);
    if(defineBodyOriginal == NULL)
        return RET_FAILURE;
    char defineBody[defineBodyOriginal->valueSize];
    strcpy(defineBody, defineBodyOriginal->value);

    char *line = strtok_r(defineBody, "\r\n", &saveptr);
    struct SizedString substitutedLine;
    ENSURE(createSizedString(&substitutedLine));
    do
    {
        substitutedLine.size = 0;
        while(true)
        {
            while(*line != '$' && *line != '\0')
                ENSURE(appendToSizedString(&substitutedLine, *line++));
            if(*line == '\0')
                break;
            if(*(line + 1) == '\0')
            {
                LOG_LINE(LOG_ERROR, "Hanging $ character in %s define body", defineName);
                return RET_ERROR;
            }
            if(*(line + 1) < '0' || *(line + 1) > '9')
            {
                LOG_LINE(LOG_ERROR, "Invalid character %c after $ in %s define body", *(line + 1), defineName);
                return RET_ERROR;
            }
            char *argument = arguments[*(line + 1) - '0'];
            if(argument != NULL)
            {
                while(*argument != '\0')
                    ENSURE(appendToSizedString(&substitutedLine, *argument++));
            }
            line += 2;
        }
        ENSURE(appendToSizedString(&substitutedLine, '\0'));
        ENSURE(parseActionLine(substitutedLine.data, actions, now));
    } while((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL);
    return RET_SUCCESS;
}


void freeGatheredDefines(struct GatheredDefines defines)
{
    for(unsigned i = 0; i < defines.size; i++)
    {
        free(defines.defines[i].key);
        free(defines.defines[i].value);
    }
    free(defines.defines);
}
