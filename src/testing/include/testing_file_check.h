#ifndef TESTING_FILE_CHECK_H
#define TESTING_FILE_CHECK_H

#include "testing.h"

#include <stdio.h>
#include <string.h>


static inline ReturnCode checkFileContent(const char *fileName, const char *proper)
{
    FILE *looperOutFile = fopen(fileName, "rb");
    ASSERT(looperOutFile != NULL);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(looperOutFile);
    ASSERT(fseek(looperOutFile, 0, SEEK_END) == 0);

    long looperOutSize = ftell(looperOutFile);
    rewind(looperOutFile);

    long properSize = strlen(proper);
    ASSERT(looperOutSize == properSize);

    char looperOut[looperOutSize];
    ASSERT(fread(looperOut, looperOutSize, 1, looperOutFile) == 1);
    fclose(looperOutFile);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    ASSERT(strncmp(proper, looperOut, properSize) == 0);
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

#endif
