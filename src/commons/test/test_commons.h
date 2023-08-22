#ifndef TEST_KONC4D_H
#define TEST_KONC4D_H

#include "testing.h"


void shared_memory(void);
void timestamps(void);

#define COMMONS_TESTS { \
    shared_memory(); \
    timestamps(); \
}

#endif
