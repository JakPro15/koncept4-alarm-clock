#include "test_integration.h"
#include "konc4_locations.h"
#include "testing_file_check.h"

#include <stdio.h>
#include <stdlib.h>


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
    ASSERT_ENSURE(checkFileContentRegex(OUTPUT_FILE,
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
    ASSERT_ENSURE(checkFileContentRegex(OUTPUT_FILE,
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
    ASSERT_ENSURE(checkFileContentRegex(OUTPUT_FILE,
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
    ASSERT_ENSURE(checkFileContentRegex(OUTPUT_FILE,
        "konc4> Actions:\r\n"
        "( \\d\\) \\{\\d{2}\\.\\d{2} \\d{2}:\\d{2}, type: (  notify|shutdown|   reset), "
        "(repeated with period: \\d+ minutes|not repeated)\\}\r\n){5}"
        "\r\n"
        "(Shutdowns will also be made in the following periods:\r\n"
        "(between \\d{2}:\\d{2} and \\d{2}:\\d{2}\r\n)+|No further shutdowns will be made.\r\n)"
        "konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode testSkip(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("skip 15; show 5\n", konc4Stream);
    pclose(konc4Stream);
    ASSERT_ENSURE(checkFileContentRegex(OUTPUT_FILE,
        "konc4> skip command executed successfully.\r\n"
        "Actions:\r\n"
        "( \\d\\) \\{\\d{2}\\.\\d{2} \\d{2}:\\d{2}, type: (  notify|shutdown|   reset), "
        "(repeated with period: \\d+ minutes|not repeated)\\}\r\n){5}"
        "\r\n"
        "(Shutdowns will also be made in the following periods:\r\n"
        "(between \\d{2}:\\d{2} and \\d{2}:\\d{2}\r\n)+|No further shutdowns will be made\\.\r\n)"
        "No actions will be made for the next 15 minutes though.\r\n"
        "konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode testEmptyCommands(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("\n;;;\n", konc4Stream);
    pclose(konc4Stream);
    ASSERT_ENSURE(checkFileContent(OUTPUT_FILE,
        "konc4> konc4> konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode testInvalidCommands(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("abc\nhehe; xd\n", konc4Stream);
    pclose(konc4Stream);
    ASSERT_ENSURE(checkFileContent(OUTPUT_FILE,
        "konc4> Unrecognized command\r\n"
        "konc4> Unrecognized command\r\n"
        "Unrecognized command\r\n"
        "konc4> "
    ));
    return RET_SUCCESS;
}


static ReturnCode teardown(void)
{
    FILE *konc4Stream = popen(KONC4_COMMAND, "w");
    write("stop\n", konc4Stream);
    ASSERT(pclose(konc4Stream) == 0);
    ASSERT(system("del konc4log.txt") == 0);
    ASSERT(system("del "KONC4LOG_LOCATION) == 0);
    return RET_FAILURE;
}


PREPARE_TESTING(cli,
    initialShutdown,
    testStartingAndStopping,
    testStoppingWhileOff,
    testStartingWhileOn,
    testShow,
    testSkip,
    testEmptyCommands,
    testInvalidCommands,
    teardown
)
