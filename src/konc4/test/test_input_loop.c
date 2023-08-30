#include "testing.h"
#include "error_handling.h"
#include "string.h"
#include "test_konc4.h"
#include "testing_file_check.h"

#include <stdlib.h>


ReturnCode checkLooperOutput(const char *proper)
{
    ASSERT(checkFileContent(LOOPER_OUT, proper) == RET_SUCCESS);
    return RET_SUCCESS;
}


static ReturnCode testEndSpinningSuccess(void)
{
    char message[] = "hehe xd\nabcde\nhehe xd xd xd\nsuccess\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_SUCCESS);
    ASSERT_ENSURE(checkLooperOutput("prompt> hehe xd\r\nprompt> abcde\r\nprompt> hehe xd xd xd\r\nprompt> "));
    return RET_SUCCESS;
}


static ReturnCode testEndSpinningFailure(void)
{
    char message[] = "abcdefgh\nfailure\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_FAILURE);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefgh\r\nprompt> "));
    return RET_SUCCESS;
}


static ReturnCode testEndSpinningError(void)
{
    char message[] = "abcdefgh\nfailure \nerror\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_ERROR);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefgh\r\nprompt> failure \r\nprompt> "));
    return RET_SUCCESS;
}


static ReturnCode testInputEqualBufferSize(void)
{
    char message[] = "abcdefghijklmnoprst\nsuccess\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_SUCCESS);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefghijklmnoprstprompt> "));
    return RET_SUCCESS;
}


static ReturnCode testInputOverBufferSize(void)
{
    char message[] = "abcdefghijklmnoprstuvw\nsuccess\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_SUCCESS);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefghijklmnoprstprompt> "));
    return RET_SUCCESS;
}


static ReturnCode testInputSuddenlyEnding(void)
{
    char message[] = "abcde\nhehe\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_FAILURE);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcde\r\nprompt> hehe\r\nprompt> "));
    return RET_SUCCESS;
}


PREPARE_TESTING(input_loop,
    testEndSpinningSuccess,
    testEndSpinningFailure,
    testEndSpinningError,
    testInputEqualBufferSize,
    testInputOverBufferSize,
    testInputSuddenlyEnding
)
