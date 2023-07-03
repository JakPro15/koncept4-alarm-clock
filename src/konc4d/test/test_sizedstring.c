#include <string.h>
#include "test_konc4d.h"
#include "sizedstring.h"


ReturnCode testCreateSizedString(void)
{
    struct SizedString string;
    string.size = (unsigned) -1;
    string.capacity = (unsigned) -1;
    string.data = NULL;
    ASSERT_NOTHROW(createSizedString(&string));
    ASSERT(string.size == 0);
    ASSERT(string.capacity == STARTING_CAPACITY);
    ASSERT(string.data != NULL);
    for(unsigned i = 0; i < string.capacity; i++)
        string.data[i] = 'A';
    return RET_SUCCESS;
}


ReturnCode testAppendToSizedStringNormal(void)
{
    struct SizedString string;
    ASSERT_NOTHROW(createSizedString(&string));
    memset(string.data, '\0', string.capacity);
    ASSERT_NOTHROW(appendToSizedString(&string, 'A'));
    ASSERT(strcmp(string.data, "A") == 0);
    ASSERT(string.size == 1);
    ASSERT_NOTHROW(appendToSizedString(&string, '\t'));
    ASSERT(strcmp(string.data, "A\t") == 0);
    ASSERT(string.size == 2);
    ASSERT_NOTHROW(appendToSizedString(&string, 'C'));
    ASSERT(strcmp(string.data, "A\tC") == 0);
    ASSERT(string.size == 3);
    ASSERT(string.capacity == STARTING_CAPACITY);
    return RET_SUCCESS;
}


ReturnCode testAppendToSizedStringReallocStarting(void)
{
    struct SizedString string;
    ASSERT_NOTHROW(createSizedString(&string));
    memset(string.data, 'A', string.capacity);
    string.size = STARTING_CAPACITY;
    ASSERT_NOTHROW(appendToSizedString(&string, 'B'));
    ASSERT(string.size == STARTING_CAPACITY + 1);
    ASSERT(string.capacity == STARTING_CAPACITY + CAPACITY_INCREMENT);
    for(int i = 0; i < STARTING_CAPACITY; i++)
        ASSERT(string.data[i] == 'A');
    ASSERT(string.data[STARTING_CAPACITY] == 'B');
    return RET_SUCCESS;
}


ReturnCode testAppendToSizedStringReallocLarger(void)
{
    struct SizedString string;
    ASSERT_NOTHROW(createSizedString(&string));
    for(int i = 0; i < STARTING_CAPACITY + CAPACITY_INCREMENT * 3; i++)
        ASSERT_NOTHROW(appendToSizedString(&string, 'A'));
    ASSERT(string.size == STARTING_CAPACITY + CAPACITY_INCREMENT * 3);
    ASSERT(string.capacity == STARTING_CAPACITY + CAPACITY_INCREMENT * 3);
    ASSERT_NOTHROW(appendToSizedString(&string, 'B'));

    ASSERT(string.size == STARTING_CAPACITY + CAPACITY_INCREMENT * 3 + 1);
    ASSERT(string.capacity == STARTING_CAPACITY + CAPACITY_INCREMENT * 4);
    for(int i = 0; i < STARTING_CAPACITY + CAPACITY_INCREMENT * 3; i++)
        ASSERT(string.data[i] == 'A');
    ASSERT(string.data[string.size - 1] == 'B');
    return RET_SUCCESS;
}


PREPARE_TESTING(sized_string,
    testCreateSizedString,
    testAppendToSizedStringNormal,
    testAppendToSizedStringReallocStarting,
    testAppendToSizedStringReallocLarger
)
