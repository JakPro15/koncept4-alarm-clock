#include "input_loop.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


ReturnCode parseInput(unsigned bufferSize, const char *prompt, enum CallbackReturn (*callback)(char*))
{
    char answer[bufferSize], passed[bufferSize];
    bool tooLong = false;
    printf("%s", prompt);
    while(fgets(answer, bufferSize, stdin) != NULL)
    {
        if(tooLong)
        {
            if(answer[strlen(answer) - 1] == '\n')
            {
                tooLong = false;
                printf("%s", prompt);
            }
        }
        else
        {
            strcpy(passed, answer);
            enum CallbackReturn returned = callback(passed);
            if(returned != KEEP_SPINNING)
                return (ReturnCode) (returned - 1);
            if(answer[strlen(answer) - 1] == '\n')
                printf("%s", prompt);
            else
                tooLong = true;
        }
    }
    return RET_FAILURE;
}
