#ifndef TESTING_FILE_CHECK_H
#define TESTING_FILE_CHECK_H

#include "error_handling.h"

#include <stdio.h>
#include <string.h>


static inline ReturnCode checkFileContent(const char *fileName, const char *proper)
{
    FILE *looperOutFile = fopen(fileName, "rb");
    if(looperOutFile == NULL)
        return RET_ERROR;
    if(fseek(looperOutFile, 0, SEEK_END) != 0)
    {
        fclose(looperOutFile);
        return RET_ERROR;
    }
    long looperOutSize = ftell(looperOutFile);
    rewind(looperOutFile);

    long properSize = strlen(proper);
    if(looperOutSize != properSize)
    {
        fclose(looperOutFile);
        return RET_ERROR;
    }

    char looperOut[looperOutSize];
    if(fread(looperOut, looperOutSize, 1, looperOutFile) != 1)
    {
        fclose(looperOutFile);
        return RET_ERROR;
    }
    fclose(looperOutFile);

    if(strncmp(proper, looperOut, properSize) == 0)
        return RET_SUCCESS;
    else
        return RET_ERROR;
}

#endif
