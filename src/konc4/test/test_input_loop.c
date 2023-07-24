#include "testing.h"
#include "error_handling.h"
#include "string.h"
#include "test_konc4.h"

#include <stdlib.h>


ReturnCode checkLooperOutput(char *proper)
{
    FILE *looperOutFile = fopen(LOOPER_OUT, "rb");
    ASSERT_CALLBACK(fseek(looperOutFile, 0, SEEK_END) == 0, fclose(looperOutFile));
    long looperOutSize = ftell(looperOutFile);
    rewind(looperOutFile);

    long properSize = strlen(proper);
    if(looperOutSize != properSize)
    {
        fclose(looperOutFile);
        return RET_ERROR;
    }

    char looperOut[looperOutSize];
    ASSERT_CALLBACK(fread(looperOut, looperOutSize, 1, looperOutFile) == 1, fclose(looperOutFile));
    fclose(looperOutFile);

    if(strncmp(proper, looperOut, properSize) == 0)
        return RET_SUCCESS;
    else
        return RET_ERROR;
}


ReturnCode testEndSpinningSuccess(void)
{
    char message[] = "hehe xd\nabcde\nhehe xd xd xd\nsuccess\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_SUCCESS);
    ASSERT_ENSURE(checkLooperOutput("prompt> hehe xd\r\nprompt> abcde\r\nprompt> hehe xd xd xd\r\nprompt> "));
    return RET_SUCCESS;
}


ReturnCode testEndSpinningFailure(void)
{
    char message[] = "abcdefgh\nfailure\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_FAILURE);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefgh\r\nprompt> "));
    return RET_SUCCESS;
}


ReturnCode testEndSpinningError(void)
{
    char message[] = "abcdefgh\nfailure \nerror\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_ERROR);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefgh\r\nprompt> failure \r\nprompt> "));
    return RET_SUCCESS;
}


ReturnCode testInputEqualBufferSize(void)
{
    char message[] = "abcdefghijklmnoprst\nsuccess\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_SUCCESS);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefghijklmnoprstprompt> "));
    return RET_SUCCESS;
}


ReturnCode testInputOverBufferSize(void)
{
    char message[] = "abcdefghijklmnoprstuvw\nsuccess\n";
    FILE *inputLooper = popen("output\\input_looper.exe > " LOOPER_OUT, "w");
    ASSERT(fwrite(message, sizeof(message), 1, inputLooper) == 1);

    ASSERT(pclose(inputLooper) == RET_SUCCESS);
    ASSERT_ENSURE(checkLooperOutput("prompt> abcdefghijklmnoprstprompt> "));
    return RET_SUCCESS;
}


ReturnCode testInputSuddenlyEnding(void)
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
