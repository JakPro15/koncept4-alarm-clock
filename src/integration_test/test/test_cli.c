#include "test_integration.h"


static ReturnCode basicTest(void)
{
    return RET_SUCCESS;
}


PREPARE_TESTING(cli,
    basicTest
)
