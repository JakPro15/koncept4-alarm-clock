#ifndef TEST_KONC4D_H
#define TEST_KONC4D_H

#include "testing.h"


void shared_memory(void);
void timestamps(void);
void action_clock(void);
void regex(void);

#define COMMONS_TESTS { \
    shared_memory(); \
    timestamps(); \
    action_clock(); \
    regex(); \
}

#endif
