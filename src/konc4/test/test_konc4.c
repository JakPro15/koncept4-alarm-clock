#include "test_konc4.h"
#include "logging.h"

#include <stdlib.h>


int main(void)
{
    logging_level = LOG_SILENT;
    KONC4_TESTS;
    system("powershell.exe rm " LOOPER_OUT);
}
