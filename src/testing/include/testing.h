#ifndef TESTING_H
#define TESTING_H

#include <stdarg.h>
#include <stdio.h>

#ifndef ERROR_HANDLING_H
typedef enum _ReturnCodeEnum
{
    RET_SUCCESS=0,
    RET_FAILURE,
    RET_ERROR
} ReturnCode;

#define SAFE(x) do { x } while(0)
#endif

#define COLOR_RESET "\x1B[0m"
#define COLOR_RED "\x1B[31m"
#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_BLUE "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN "\x1B[36m"
#define COLOR_WHITE "\x1B[37m"


void doTesting(char *fileName, unsigned numberOfFunctions, ...);


#define NUMARGS(...)  (sizeof((ReturnCode (*[])(void)){__VA_ARGS__}))/(sizeof(ReturnCode (*)(void)))
#define PREPARE_TESTING(function_name, ...)  void function_name(void) { \
    doTesting(__FILE__, NUMARGS(__VA_ARGS__), __VA_ARGS__); \
}

#define RETURN_CALLBACK
#define ASSERT(expression) SAFE(if(!(expression)) { \
    printf(COLOR_RED "%s(%s:%d) Assertion failed\n" COLOR_RESET, __func__, __FILE__, __LINE__); \
    RETURN_CALLBACK \
    return RET_ERROR; \
})
#define ASSERT_MESSAGE(expression, message) SAFE(if(!(expression)) { \
    printf(COLOR_RED "%s(%s:%d) Assertion failed: %s\n" COLOR_RESET, __func__, __FILE__, __LINE__, (message)); \
    RETURN_CALLBACK \
    return RET_ERROR; \
})
#define ASSERT_ENSURE(expression) SAFE(if((expression) != RET_SUCCESS) { \
    printf(COLOR_RED "%s(%s:%d) Function %s failed\n" COLOR_RESET, __func__, __FILE__, __LINE__, #expression); \
    RETURN_CALLBACK \
    return RET_ERROR; \
})
#define ASSERT_NOTHROW(expression) SAFE(if((expression) == RET_ERROR) { \
    printf(COLOR_RED "%s(%s:%d) Function %s threw error\n" COLOR_RESET, __func__, __FILE__, __LINE__, #expression); \
    RETURN_CALLBACK \
    return RET_ERROR; \
})
#define ASSERT_THROW(expression) SAFE(if((expression) != RET_ERROR) { \
    printf(COLOR_RED "%s(%s:%d) Function %s did not throw error\n" COLOR_RESET, __func__, __FILE__, __LINE__, #expression); \
    RETURN_CALLBACK \
    return RET_ERROR; \
})

#endif
