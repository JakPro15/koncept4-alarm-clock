#ifndef TEST_KONC4D_H
#define TEST_KONC4D_H

#include "testing.h"


void sized_string(void);
void settings_reading(void);
void actions(void);
void parse_action(void);
void preprocessing(void);
void action_clock(void);

#define KONC4D_TESTS { \
    sized_string(); \
    settings_reading(); \
    actions(); \
    parse_action(); \
    preprocessing(); \
    action_clock(); \
}

#endif
