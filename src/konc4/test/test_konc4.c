#include "test_konc4.h"
#include "logging.h"


int main(void)
{
    logging_level = LOG_SILENT;
    KONC4_TESTS;
}
