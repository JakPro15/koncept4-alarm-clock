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
    freeSizedString(&lineRead);
    return RET_SUCCESS;
}


ReturnCode testGetNextAction(void)
{
    struct BufferedFile file;
    struct SizedString lineBuffer;
    struct Action actionRead;
    struct YearTimestamp now = {{.date = {29, 2}, .time = {12, 30}}, .currentYear = 2012};
    ASSERT_ENSURE(createSizedString(&lineBuffer));
    ASSERT_ENSURE(openBufferedFile(&file, "test/test_get_next_action.txt"));

    ASSERT_ENSURE(getNextAction(&file, &actionRead, &lineBuffer, now));
    ASSERT(actionRead.type == RESET);
    ASSERT(basicCompareTimestamp(actionRead.timestamp, (struct Timestamp) {{12, 9}, {22, 30}}) == 0);
    ASSERT(actionRead.repeated == false);

    ASSERT_ENSURE(getNextAction(&file, &actionRead, &lineBuffer, now));
    ASSERT(actionRead.type == SHUTDOWN);
    ASSERT(basicCompareTimestamp(actionRead.timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(actionRead.args.shutdown.delay == 35);
    ASSERT(actionRead.repeated == false);

    ASSERT_ENSURE(getNextAction(&file, &actionRead, &lineBuffer, now));
    ASSERT(actionRead.type == NOTIFY);
    ASSERT(basicCompareTimestamp(actionRead.timestamp, (struct Timestamp) {{1, 3}, {11, 30}}) == 0);
    ASSERT(strcmp(actionRead.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(actionRead.args.notify.repeats == 5);
    ASSERT(actionRead.repeated == true);

    ASSERT(getNextAction(&file, &actionRead, &lineBuffer, now) == RET_FAILURE);

    ASSERT_ENSURE(closeBufferedFile(&file));
    freeSizedString(&lineBuffer);
    return RET_SUCCESS;
}


ReturnCode testLoadActionsFromFile(void)
{
    struct ActionQueue *results = NULL;
    ASSERT_ENSURE(loadActionsFromFile(&results, "test/test_get_next_action.txt",
                  (struct YearTimestamp) {{{1, 4}, {12, 0}}, 2020}));

    ASSERT(AQ_FIRST(results).type == NOTIFY);
    ASSERT(basicCompareTimestamp(AQ_FIRST(results).timestamp, (struct Timestamp) {{2, 4}, {11, 30}}) == 0);
    ASSERT(strcmp(AQ_FIRST(results).args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(AQ_FIRST(results).args.notify.repeats == 5);
    ASSERT(AQ_FIRST(results).repeated == true);

    ASSERT(AQ_SECOND(results).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_SECOND(results).timestamp, (struct Timestamp) {{12, 9}, {22, 30}}) == 0);
    ASSERT(AQ_SECOND(results).repeated == false);

    ASSERT(AQ_THIRD(results).type == SHUTDOWN);
    ASSERT(basicCompareTimestamp(AQ_THIRD(results).timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(AQ_THIRD(results).args.shutdown.delay == 35);
    ASSERT(AQ_THIRD(results).repeated == false);

    ASSERT(results->next->next->next == NULL);

    return RET_SUCCESS;
}


PREPARE_TESTING(settings_reading,
    testGetCharacter,
    testGetLine,
    testGetNextAction,
    testLoadActionsFromFile
)
