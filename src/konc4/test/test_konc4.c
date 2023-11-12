#include "test_konc4.h"
#include "logging.h"

#include <stdlib.h>


int main(void)
{
    KONC4_TESTS
    system("del "LOOPER_OUT);
}
