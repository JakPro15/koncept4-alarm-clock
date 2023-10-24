#include <regex>

extern "C" {
#include "regex.h"
#include "logging.h"
}

ReturnCode regexValidate(const char *text, const char *pattern)
{
    try
    {
        std::regex regex(pattern, std::regex::ECMAScript);
        if(std::regex_match(text, regex))
            return RET_SUCCESS;
        else
            return RET_FAILURE;
    }
    catch(const std::exception& e)
    {
        LOG_LINE(LOG_ERROR, "Exception thrown by C++ regex library, with message: %s", e.what());
        return RET_ERROR;
    }
}
