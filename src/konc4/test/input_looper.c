#include "input_loop.h"
#include "logging.h"

#include <stdio.h>
#include <string.h>


const enum LOGGING_LEVEL logging_level = LOG_SILENT;


enum CallbackReturn handler(char *lineRead)
{
    if(strcmp(lineRead, "success\n") == 0)
        return END_SPINNING_SUCCESS;
    if(strcmp(lineRead, "failure\n") == 0)
        return END_SPINNING_FAILURE;
    if(strcmp(lineRead, "error\n") == 0)
        return END_SPINNING_ERROR;
    printf("%s", lineRead);
    return KEEP_SPINNING;
}


int main()
{
    return parseInput(20, "prompt> ", handler);
}
