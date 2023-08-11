#ifndef KONC4_INPUT_LOOP_H
#define KONC4_INPUT_LOOP_H

#include "error_handling.h"


enum CallbackReturn
{
    KEEP_SPINNING=0,
    END_SPINNING_SUCCESS,
    END_SPINNING_FAILURE,
    END_SPINNING_ERROR
};


ReturnCode parseInput(unsigned bufferSize, const char *prompt, enum CallbackReturn (*callback)(char*));

#endif
