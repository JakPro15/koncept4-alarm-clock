#include "testing.h"


void doTesting(char *fileName, unsigned numberOfFunctions, ...)
{
    printf("Testing of file %s\n", fileName);
    va_list functions;
    va_start(functions, numberOfFunctions);
    unsigned succeeded = 0;
    for(unsigned i = 0; i < numberOfFunctions; i++)
    {
        ReturnCode (*testFunction)(void) = va_arg(functions, ReturnCode (*)(void));
        if(testFunction() == RET_SUCCESS)
            ++succeeded;
    }
    va_end(functions);
    printf("finished, ");
    printf(succeeded == numberOfFunctions ? COLOR_GREEN : COLOR_RED);
    printf("%d/%d tests succeeded.\n" COLOR_RESET, succeeded, numberOfFunctions);
}
