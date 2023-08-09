#ifndef KONC4D_SIZEDSTRING_H
#define KONC4D_SIZEDSTRING_H

#include <stdbool.h>
#include "error_handling.h"

#define CAPACITY_INCREMENT 128
#define STARTING_CAPACITY CAPACITY_INCREMENT * 2


struct SizedString
{
    unsigned size, capacity;
    char *data;
};


ReturnCode createSizedString(struct SizedString *string) NO_IGNORE;
ReturnCode increaseSizedStringCapacity(struct SizedString *string) NO_IGNORE;
ReturnCode appendToSizedString(struct SizedString *string, char character) NO_IGNORE;
void freeSizedString(struct SizedString string);


#endif
