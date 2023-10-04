#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#ifndef TESTING_H
typedef enum
{
    RET_SUCCESS=0,
    RET_FAILURE,
    RET_ERROR
} ReturnCode;
#endif

#define SAFE(x) do { x } while(0)
#define NO_IGNORE                            __attribute__((warn_unused_result))
#define ENSURE(expr)                         SAFE(if((expr) != RET_SUCCESS) return RET_ERROR;)
#define RETHROW(expr)                        SAFE(if((expr) == RET_ERROR) return RET_ERROR;)
#define RETURN_FAIL(expr)                    SAFE(ReturnCode result = (expr); if(result != RET_SUCCESS) return result;)
#define ENSURE_CALLBACK(expr, callback)      SAFE(if((expr) != RET_SUCCESS) { callback; return RET_ERROR; })
#define RETHROW_CALLBACK(expr, callback)     SAFE(if((expr) == RET_ERROR) { callback; return RET_ERROR; })
#define RETURN_FAIL_CALLBACK(expr, callback) SAFE(ReturnCode result = (expr); if(result != RET_SUCCESS) { callback; return result; })
#define TRY_END(expr)                        SAFE(if((expr) == RET_SUCCESS) return RET_SUCCESS;)
#define TRY_END_RETHROW(expr)                SAFE(ReturnCode result = (expr); if(result != RET_FAILURE) return result;)

#endif
