#include "testing.h"
#include "stringvector.h"

#include <stdlib.h>
#include <string.h>


ReturnCode testCreateStringVector(void)
{
    struct StringVector vector;
    vector.size = (unsigned) -1;
    vector.capacity = (unsigned) -1;
    vector.data = NULL;
    ASSERT_NOTHROW(createStringVector(&vector));
    ASSERT(vector.size == 0);
    ASSERT(vector.capacity == STARTING_CAPACITY);
    ASSERT(vector.data != NULL);
    for(unsigned i = 0; i < vector.capacity; i++)
        vector.data[i] = NULL;
    free(vector.data);
    return RET_SUCCESS;
}


ReturnCode testAppendToStringVectorNormal(void)
{
    struct StringVector vector;
    ASSERT_NOTHROW(createStringVector(&vector));
    memset(vector.data, 0, vector.capacity);
    ASSERT_NOTHROW(appendToStringVector(&vector, "ABC"));
    ASSERT(strcmp(vector.data[0], "ABC") == 0);
    ASSERT(vector.size == 1);
    ASSERT_NOTHROW(appendToStringVector(&vector, "HEHE xD"));
    ASSERT(strcmp(vector.data[0], "ABC") == 0);
    ASSERT(strcmp(vector.data[1], "HEHE xD") == 0);
    ASSERT(vector.size == 2);
    ASSERT_NOTHROW(appendToStringVector(&vector, "iks de"));
    ASSERT(strcmp(vector.data[0], "ABC") == 0);
    ASSERT(strcmp(vector.data[1], "HEHE xD") == 0);
    ASSERT(strcmp(vector.data[2], "iks de") == 0);
    ASSERT(vector.size == 3);
    ASSERT(vector.capacity == STARTING_CAPACITY);
    free(vector.data);
    return RET_SUCCESS;
}


ReturnCode testAppendToStringVectorReallocStarting(void)
{
    struct StringVector vector;
    ASSERT_NOTHROW(createStringVector(&vector));
    memset(vector.data, 0, vector.capacity);
    vector.size = STARTING_CAPACITY;
    ASSERT_NOTHROW(appendToStringVector(&vector, "BBB"));
    ASSERT(vector.size == STARTING_CAPACITY + 1);
    ASSERT(vector.capacity == STARTING_CAPACITY + CAPACITY_INCREMENT);
    for(int i = 0; i < STARTING_CAPACITY; i++)
        ASSERT(vector.data[i] == 0);
    ASSERT(strcmp(vector.data[STARTING_CAPACITY], "BBB") == 0);
    free(vector.data);
    return RET_SUCCESS;
}


ReturnCode testAppendToStringVectorReallocLarger(void)
{
    struct StringVector vector;
    ASSERT_NOTHROW(createStringVector(&vector));
    for(int i = 0; i < STARTING_CAPACITY + CAPACITY_INCREMENT * 3; i++)
        ASSERT_NOTHROW(appendToStringVector(&vector, "AAA"));
    ASSERT(vector.size == STARTING_CAPACITY + CAPACITY_INCREMENT * 3);
    ASSERT(vector.capacity == STARTING_CAPACITY + CAPACITY_INCREMENT * 3);
    ASSERT_NOTHROW(appendToStringVector(&vector, "ABC"));

    ASSERT(vector.size == STARTING_CAPACITY + CAPACITY_INCREMENT * 3 + 1);
    ASSERT(vector.capacity == STARTING_CAPACITY + CAPACITY_INCREMENT * 4);
    for(int i = 0; i < STARTING_CAPACITY + CAPACITY_INCREMENT * 3; i++)
        ASSERT(strcmp(vector.data[i], "AAA") == 0);
    ASSERT(strcmp(vector.data[vector.size - 1], "ABC") == 0);
    free(vector.data);
    return RET_SUCCESS;
}


PREPARE_TESTING(stringvector,
    testCreateStringVector,
    testAppendToStringVectorNormal,
    testAppendToStringVectorReallocStarting,
    testAppendToStringVectorReallocLarger
)
