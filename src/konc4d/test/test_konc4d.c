#include "test_konc4d.h"
#include "logging.h"


int main(void)
{
    logging_level = LOG_SILENT;
    KONC4D_TESTS;
}
