#include "test_konc4d.h"
#include "settings_reading.h"


ReturnCode testGetLine(void)
{
    FILE *file = fopen("test/test_get_line.txt", "rb");
    ASSERT(file != NULL);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(file);

    struct SizedString lineRead;
    ASSERT_ENSURE(createSizedString(&lineRead));

    ASSERT_ENSURE(getLine(file, &lineRead));
    ASSERT(strcmp(lineRead.data, "hehe xd\r\n") == 0);
    ASSERT(lineRead.size == 10);

    ASSERT_ENSURE(getLine(file, &lineRead));
    ASSERT(strcmp(lineRead.data, "abcd\r\n") == 0);
    ASSERT(lineRead.size == 7);

    ASSERT_ENSURE(getLine(file, &lineRead));
    ASSERT(strcmp(lineRead.data, "\r\n") == 0);
    ASSERT(lineRead.size == 3);

    ASSERT_ENSURE(getLine(file, &lineRead));
    ASSERT(strcmp(lineRead.data, "last line") == 0);
    ASSERT(lineRead.size == 10);

    ASSERT(getLine(file, &lineRead) == RET_FAILURE);
    fclose(file);
    freeSizedString(lineRead);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


ReturnCode testGetNextAction(void)
{
    FILE *file = fopen("test/test_get_next_action.txt", "rb");
    ASSERT(file != NULL);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(file);

    struct SizedString lineBuffer;
    struct Action actionRead;
    struct YearTimestamp now = {{.date = {1, 3}, .time = {12, 30}}, .currentYear = 2011};
    ASSERT_ENSURE(createSizedString(&lineBuffer));

    ASSERT_ENSURE(getNextAction(file, &actionRead, &lineBuffer, now));
    ASSERT(actionRead.type == RESET);
    ASSERT(basicCompareTimestamp(actionRead.timestamp, (struct Timestamp) {{12, 9}, {22, 30}}) == 0);
    ASSERT(actionRead.repeated == false);

    ASSERT_ENSURE(getNextAction(file, &actionRead, &lineBuffer, now));
    ASSERT(actionRead.type == SHUTDOWN);
    ASSERT(basicCompareTimestamp(actionRead.timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(actionRead.args.shutdown.delay == 35);
    ASSERT(actionRead.repeated == false);

    ASSERT_ENSURE(getNextAction(file, &actionRead, &lineBuffer, now));
    ASSERT(actionRead.type == NOTIFY);
    ASSERT(basicCompareTimestamp(actionRead.timestamp, (struct Timestamp) {{2, 3}, {11, 30}}) == 0);
    ASSERT(strcmp(actionRead.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(actionRead.args.notify.repeats == 5);
    ASSERT(actionRead.repeated == true);

    ASSERT(getNextAction(file, &actionRead, &lineBuffer, now) == RET_FAILURE);

    fclose(file);
    freeSizedString(lineBuffer);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


ReturnCode testLoadActionsFromFile(void)
{
    struct ActionQueue *results = NULL;
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&results);
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

    ASSERT(results->next->next == NULL);
    destroyActionQueue(&results);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


ReturnCode testLoadActionsFromFile29FebLast(void)
{
    struct ActionQueue *results = NULL;
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&results);
    ASSERT_ENSURE(loadActionsFromFile(&results, "test/test_load_actions_from_file.txt",
                  (struct YearTimestamp) {{{1, 4}, {12, 0}}, 2020}));

    ASSERT(results == NULL);
    destroyActionQueue(&results);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


PREPARE_TESTING(settings_reading,
    testGetLine,
    testGetNextAction,
    testLoadActionsFromFile,
    testLoadActionsFromFile29FebLast
)
