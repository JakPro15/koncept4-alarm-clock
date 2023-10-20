#include "test_integration.h"
#include "konc4_locations.h"

#include <stdio.h>


static ReturnCode initialShutdown(void)
{
    FILE *konc4Stream = popen(KONC4_LOCATION" > output/konc4_output.txt", "w");
    fwrite("stop", sizeof("stop"), 1, konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    return RET_FAILURE;
}


PREPARE_TESTING(cli,
    initialShutdown
)
