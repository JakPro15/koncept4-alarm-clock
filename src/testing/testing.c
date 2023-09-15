#include "testing.h"

enum LOGGING_LEVEL
{
    LOG_TRACE=0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SILENT
};
extern enum LOGGING_LEVEL logging_level;


void doTesting(char *fileName, unsigned numberOfFunctions, ...)
{
    logging_level = LOG_SILENT;
    printf("Testing of file %s\n", fileName);
    va_list functions;
    va_start(functions, numberOfFunctions);
    unsigned succeeded = 0, run = 0;
    for(unsigned i = 0; i < numberOfFunctions; i++)
    {
        ReturnCode (*testFunction)(void) = va_arg(functions, ReturnCode (*)(void));
        ReturnCode result = testFunction();
        if(result == RET_SUCCESS)
            ++succeeded;
        if(result != RET_FAILURE)
            ++run;
    }
    va_end(functions);
    printf("finished, ");
    printf(succeeded == run ? COLOR_GREEN : COLOR_RED);
    printf("%d/%d tests succeeded.\n" COLOR_RESET, succeeded, run);
}
