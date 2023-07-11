#ifndef TEST_KONC4D_H
#define TEST_KONC4D_H

#include "testing.h"


void shared_memory(void);

#define COMMONS_TESTS { \
    shared_memory(); \
}

#endif
