#include "testing.h"


ReturnCode test1(void)
{
    return RET_SUCCESS;
}


PREPARE_TESTING(shared_memory,
    test1
)
