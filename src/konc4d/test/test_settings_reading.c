#include "test_konc4d.h"
#include "settings_reading.h"


ReturnCode testGetCharacter(void)
{
    struct BufferedFile file;
    char charRead;
    ASSERT_ENSURE(openBufferedFile(&file, "test/test_get_character.txt"));
    ASSERT_ENSURE(getCharacter(&file, &charRead));
    ASSERT(charRead == 'h');
    ASSERT_ENSURE(getCharacter(&file, &charRead));
    ASSERT(charRead == ' ');
    ASSERT_ENSURE(getCharacter(&file, &charRead));
    ASSERT(charRead == 'e');
    ASSERT_ENSURE(getCharacter(&file, &charRead));
    ASSERT(charRead == '\r');
    ASSERT_ENSURE(getCharacter(&file, &charRead));
    ASSERT(charRead == '\n');
    ASSERT(getCharacter(&file, &charRead) == RET_FAILURE);
    ASSERT_ENSURE(closeBufferedFile(&file));
    return RET_SUCCESS;
}


ReturnCode testGetLine(void)
{
    struct BufferedFile file;
    struct SizedString lineRead;
    ASSERT_ENSURE(createSizedString(&lineRead));
    ASSERT_ENSURE(openBufferedFile(&file, "test/test_get_line.txt"));
    ASSERT_ENSURE(getLine(&file, &lineRead));
    ASSERT(strcmp(lineRead.data, "hehe xd") == 0);
    lineRead.size = 0;
    ASSERT_ENSURE(getLine(&file, &lineRead));
    ASSERT(strcmp(lineRead.data, "abcd") == 0);
    lineRead.size = 0;
    ASSERT_ENSURE(getLine(&file, &lineRead));
    ASSERT(strcmp(lineRead.data, "") == 0);
    lineRead.size = 0;
    ASSERT_ENSURE(getLine(&file, &lineRead));
    ASSERT(strcmp(lineRead.data, "last line") == 0);
    ASSERT(getLine(&file, &lineRead) == RET_FAILURE);
    ASSERT_ENSURE(closeBufferedFile(&file));
    return RET_SUCCESS;
}


PREPARE_TESTING(settings_reading,
    testGetCharacter,
    testGetLine
)
