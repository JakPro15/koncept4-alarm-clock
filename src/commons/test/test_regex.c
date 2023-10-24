#include "testing.h"
#include "regex.h"


static ReturnCode testSingleLineBasic(void)
{
    ASSERT_ENSURE(regexValidate("hehe xd", "hehe xd"));
    ASSERT(regexValidate("hehe xd", "hehexd") == RET_FAILURE);
    ASSERT(regexValidate("hehexd", "hehe xd") == RET_FAILURE);
    return RET_SUCCESS;
}


static ReturnCode testSingleLineRegex(void)
{
    ASSERT_ENSURE(regexValidate("hhe xd", "he?he\\s+x[bcd]"));
    ASSERT_ENSURE(regexValidate("hehe   xb", "he?he\\s+x[bcd]"));
    ASSERT(regexValidate("hehexd", "he?he\\s+x[bcd]") == RET_FAILURE);
    return RET_SUCCESS;
}


static ReturnCode testMultilineBasic(void)
{
    ASSERT_ENSURE(regexValidate("hehe xd\r\nhehe", "hehe xd\r\nhehe"));
    ASSERT(regexValidate("hehe xd\r\nhehe\r\n", "hehe xd\r\nhehe") == RET_FAILURE);
    ASSERT(regexValidate("hehexd\r\nhehe", "hehe xd\r\nhehe") == RET_FAILURE);
    return RET_SUCCESS;
}


static ReturnCode testMultilineRegex(void)
{
    ASSERT_ENSURE(regexValidate("hhe \r\nxd", "he?he\\s+\r\nx[bcd]"));
    ASSERT_ENSURE(regexValidate("hehe   \r\nxb", "he?he\\s+\r\nx[bcd]"));
    ASSERT(regexValidate("hehe\r\nxd", "he?he\\s+\r\nx[bcd]") == RET_FAILURE);
    return RET_SUCCESS;
}


PREPARE_TESTING(regex,
    testSingleLineBasic,
    testSingleLineRegex,
    testMultilineBasic,
    testMultilineRegex
)