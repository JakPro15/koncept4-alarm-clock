#include "error_handling.h"

#define CAPACITY_INCREMENT 4
#define STARTING_CAPACITY CAPACITY_INCREMENT * 2

struct StringVector
{
    char **data;
    unsigned size, capacity;
};


ReturnCode createStringVector(struct StringVector *vector);
ReturnCode appendToStringVector(struct StringVector *vector, char *string);
void fullyFreeStringVector(struct StringVector *vector);
