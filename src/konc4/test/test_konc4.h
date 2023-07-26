#ifndef TEST_KONC4_H
#define TEST_KONC4_H

#include "testing.h"

#define LOOPER_OUT "output\\looper_out.txt"


void input_loop(void);
void command_execution(void);

#define KONC4_TESTS { \
    input_loop(); \
    command_execution(); \
}

#endif
