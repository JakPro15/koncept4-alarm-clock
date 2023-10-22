#include "test_integration.h"
#include "konc4_locations.h"
#include "testing_file_check.h"

#include <stdio.h>


#define write(message, stream) fwrite(message, strlen(message), 1, stream)
#define OUTPUT_FILE "output/konc4_output.txt"
#define KONC4_COMMAND KONC4_LOCATION" > "OUTPUT_FILE

static ReturnCode initialShutdown(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("stop\n", konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    return RET_FAILURE;
}


static ReturnCode testStartingAndStopping(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("start\nstop\n", konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    ASSERT_ENSURE(checkFileContent(OUTPUT_FILE,
        "konc4> start command executed successfully.\r\n"
        "konc4> stop command executed successfully.\r\n"
        "konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode testStoppingWhileOff(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("stop\n", konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    ASSERT_ENSURE(checkFileContent(OUTPUT_FILE,
        "konc4> konc4d is off.\r\n"
        "konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode testStartingWhileOn(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("start; start\n", konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    ASSERT_ENSURE(checkFileContent(OUTPUT_FILE,
        "konc4> start command executed successfully.\r\n"
        "konc4d is already on.\r\n"
        "konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode testShow(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("show 5\n", konc4Stream);
    pclose(konc4Stream);

    FILE *outputFile = fopen(OUTPUT_FILE, "rb");
    ASSERT(outputFile != NULL);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(outputFile);
    ASSERT_ENSURE(checkStringInFile("konc4> Actions:\r\n", outputFile));

#define CHECK_LINE(i) SAFE( \
    ASSERT_ENSURE(checkStringInFile(" "#i") {", outputFile)); \
    ASSERT_ENSURE(skipUntilNextLine(outputFile)); \
)
    CHECK_LINE(1);
    CHECK_LINE(2);
    CHECK_LINE(3);
    CHECK_LINE(4);
    CHECK_LINE(5);
#undef CHECK_LINE
    ASSERT_ENSURE(skipUntilNextLine(outputFile));
    ASSERT_ENSURE(checkStringInFile("Shutdowns will also be made in the following periods:\r\n", outputFile));

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    fclose(outputFile);
    return RET_SUCCESS;
}


static ReturnCode teardownShutdown(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("stop\n", konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    return RET_FAILURE;
}


PREPARE_TESTING(cli,
    initialShutdown,
    testStartingAndStopping,
    testStoppingWhileOff,
    testStartingWhileOn,
    testShow,
    teardownShutdown
)
