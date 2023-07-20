#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#ifndef TESTING_H
typedef enum _ReturnCodeEnum
{
    RET_SUCCESS=0,
    RET_FAILURE,
    RET_ERROR
} ReturnCode;
#endif

#define NO_IGNORE                        __attribute__((warn_unused_result))
#define ENSURE(expr)                     do { if((expr) != RET_SUCCESS) return RET_ERROR; } while(0)
#define RETHROW(expr)                    do { if((expr) == RET_ERROR) return RET_ERROR; } while(0)
#define RETURN_FAIL(expr)                do { ReturnCode expr_result = (expr); if(expr_result != RET_SUCCESS) return expr_result; } while(0)
#define ENSURE_CALLBACK(expr, callback)  do { if((expr) != RET_SUCCESS) { callback; return RET_ERROR; } } while(0)
#define RETHROW_CALLBACK(expr, callback) do { if((expr) == RET_ERROR) { callback; return RET_ERROR; } } while(0)
#define TRY_END(expr)                    do { if((expr) == RET_SUCCESS) return RET_SUCCESS; } while(0)

#endif
