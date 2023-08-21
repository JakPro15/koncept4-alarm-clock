#ifndef TEST_KONC4D_H
#define TEST_KONC4D_H

#include "testing.h"


void timestamps(void);
void sized_string(void);
void settings_reading(void);
void actions(void);
void preprocessing(void);

#define KONC4D_TESTS { \
    timestamps(); \
    sized_string(); \
    settings_reading(); \
    actions(); \
    preprocessing(); \
}

#endif
