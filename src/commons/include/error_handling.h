#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

typedef enum ReturnCodeEnum
{
    RET_SUCCESS=0,
    RET_FAILURE,
    RET_ERROR
} ReturnCode;

#define ENSURE(expr) if((expr) != RET_SUCCESS) return RET_ERROR
#define RETHROW(expr) if((expr) == RET_ERROR) return RET_ERROR
#define ENSURE_EXIT(expr, callback) if((expr) != RET_SUCCESS) { callback; return RET_ERROR; }
#define TRY_END(expr) if((expr) == RET_SUCCESS) return RET_SUCCESS

#endif
