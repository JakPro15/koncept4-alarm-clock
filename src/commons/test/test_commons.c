#include "test_commons.h"
#include "logging.h"


int main(void)
{
    logging_level = LOG_SILENT;
    COMMONS_TESTS;
}
