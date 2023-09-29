#include "test_konc4d.h"
#include "settings_reading.h"


static ReturnCode testGetLine(void)
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


static ReturnCode testLoadActionsFromFile(void)
{
    struct AllActions results = {.queueHead = NULL};
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&results.queueHead);
    ASSERT_ENSURE(loadActionsFromFile(&results, "test/test_settings_reading.txt",
                  (struct YearTimestamp) {{{1, 4}, {12, 0}}, 2020}));

    ASSERT(AQ_FIRST(results.queueHead).type == NOTIFY);
    ASSERT(basicCompareTimestamp(AQ_FIRST(results.queueHead).timestamp, (struct Timestamp) {{2, 4}, {11, 30}}) == 0);
    ASSERT(strcmp(AQ_FIRST(results.queueHead).args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(AQ_FIRST(results.queueHead).args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(AQ_FIRST(results.queueHead).repeatPeriod == MINUTES_IN_DAY);

    ASSERT(AQ_SECOND(results.queueHead).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_SECOND(results.queueHead).timestamp, (struct Timestamp) {{12, 4}, {22, 30}}) == 0);
    ASSERT(AQ_SECOND(results.queueHead).repeatPeriod == MONTHLY_REPEAT);

    ASSERT(results.queueHead->next->next == NULL);
    destroyActionQueue(&results.queueHead);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


static ReturnCode testLoadActionsFromFileWithFeb29(void)
{
    struct AllActions results = {.queueHead = NULL};
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&results.queueHead);
    ASSERT_ENSURE(loadActionsFromFile(&results, "test/test_settings_reading.txt",
                  (struct YearTimestamp) {{{1, 4}, {12, 0}}, 2019}));

    ASSERT(AQ_FIRST(results.queueHead).type == NOTIFY);
    ASSERT(basicCompareTimestamp(AQ_FIRST(results.queueHead).timestamp, (struct Timestamp) {{2, 4}, {11, 30}}) == 0);
    ASSERT(strcmp(AQ_FIRST(results.queueHead).args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(AQ_FIRST(results.queueHead).args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(AQ_FIRST(results.queueHead).repeatPeriod == MINUTES_IN_DAY);

    ASSERT(AQ_SECOND(results.queueHead).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_SECOND(results.queueHead).timestamp, (struct Timestamp) {{12, 4}, {22, 30}}) == 0);
    ASSERT(AQ_SECOND(results.queueHead).repeatPeriod == MONTHLY_REPEAT);

    ASSERT(AQ_THIRD(results.queueHead).type == SHUTDOWN);
    ASSERT(basicCompareTimestamp(AQ_THIRD(results.queueHead).timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(AQ_THIRD(results.queueHead).args.shutdown.delay == 35);
    ASSERT(AQ_THIRD(results.queueHead).repeatPeriod == false);

    ASSERT(results.queueHead->next->next->next == NULL);
    destroyActionQueue(&results.queueHead);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


static ReturnCode testLoadActionsFromFile29FebLast(void)
{
    struct AllActions results = {.queueHead = NULL};
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&results.queueHead);
    ASSERT_ENSURE(loadActionsFromFile(&results, "test/test_load_actions_from_file.txt",
                  (struct YearTimestamp) {{{1, 4}, {12, 0}}, 2020}));

    ASSERT(results.queueHead == NULL);
    destroyActionQueue(&results.queueHead);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


static ReturnCode testSkipPreprocessingDirectives(void)
{
    FILE *file = fopen("test/test_skip_preprocessing_directives.txt", "r");
    ASSERT(file != NULL);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(file);

    ASSERT_ENSURE(skipPreprocessingDirectives(file));
    char data[10];
    ASSERT(fgets(data, 10, file) != NULL);
    ASSERT(strcmp(data, "bean") == 0);

#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


PREPARE_TESTING(settings_reading,
    testGetLine,
    testLoadActionsFromFile,
    testLoadActionsFromFileWithFeb29,
    testLoadActionsFromFile29FebLast,
    testSkipPreprocessingDirectives
)
