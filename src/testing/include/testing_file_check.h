#ifndef TESTING_FILE_CHECK_H
#define TESTING_FILE_CHECK_H

#include "testing.h"

#include <stdio.h>
#include <string.h>
#include <re.h>


static inline ReturnCode checkFileContent(const char *fileName, const char *proper)
{
    FILE *file = fopen(fileName, "rb");
    ASSERT(file != NULL);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(file);
    ASSERT(fseek(file, 0, SEEK_END) == 0);

    long fileSize = ftell(file);
    rewind(file);

    long properSize = strlen(proper);
    ASSERT(fileSize == properSize);

    char fileContent[fileSize];
    ASSERT(fread(fileContent, fileSize, 1, file) == 1);
    fclose(file);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    ASSERT(strncmp(proper, fileContent, properSize) == 0);
    return RET_SUCCESS;
}


static inline ReturnCode checkStringInFile(const char *toRead, FILE *stream)
{
    char buffer[strlen(toRead) + 1];
    ASSERT(fread(buffer, strlen(toRead), 1, stream) == 1);
    buffer[strlen(toRead)] = '\0';
    ASSERT(strcmp(buffer, toRead) == 0);
    return RET_SUCCESS;
}


static inline ReturnCode skipUntilNextLine(FILE *stream)
{
    int read;
    do
    {
        read = fgetc(stream);
        ASSERT(read != EOF);
    } while(read != '\n');
    return RET_SUCCESS;
}


static inline ReturnCode checkFileContentRegex(const char *fileName, const char *proper)
{
    FILE *file = fopen(fileName, "rb");
    ASSERT(file != NULL);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(file);
    ASSERT(fseek(file, 0, SEEK_END) == 0);

    long fileSize = ftell(file);
    rewind(file);

    long properSize = strlen(proper);
    ASSERT(fileSize == properSize);

    char fileContent[fileSize];
    ASSERT(fread(fileContent, fileSize, 1, file) == 1);
    fclose(file);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    int matchSize;
    ASSERT(re_match(proper, fileContent, &matchSize) == 0);
    ASSERT(fileSize == matchSize);
    return RET_SUCCESS;
}

#endif
