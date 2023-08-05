#ifndef LOGGING_H
#define LOGGING_H

#include "error_handling.h"


enum LOGGING_LEVEL
{
    LOG_TRACE=0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SILENT
};

extern enum LOGGING_LEVEL logging_level;
ReturnCode logLine(enum LOGGING_LEVEL level, char *format, ...) __attribute__((format(printf, 2, 3)));

#define LOG_LINE(level, format, ...) \
    logLine(level, "%s(%s:%d) " format, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

#endif
