#ifndef REGEX_H
#define REGEX_H

#include "error_handling.h"

ReturnCode regexValidate(const char *text, const char *pattern) NO_IGNORE;

#endif
