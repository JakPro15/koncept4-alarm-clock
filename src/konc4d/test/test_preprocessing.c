#include "test_konc4d.h"
#include "preprocessing.h"
#include "settings_reading.h"

#include <stdio.h>
#include <string.h>


static ReturnCode testVerifyDefineName(void)
{
    unsigned size = 0;
    ASSERT_ENSURE(verifyDefineName("name", &size));
    ASSERT(size == 5);
    size = 0;
    ASSERT_ENSURE(verifyDefineName("nameheh2343@5^&", &size));
    ASSERT(size == 16);
    ASSERT_THROW(verifyDefineName("", &size));
    ASSERT_THROW(verifyDefineName("nam e", &size));
    ASSERT_THROW(verifyDefineName("name ", &size));
    ASSERT_THROW(verifyDefineName("n#ame", &size));
    return RET_SUCCESS;
}


#define ASSERT_EQUAL(define, expectedKey, expectedValue) \
    ASSERT(strcmp(define.key, (expectedKey)) == 0); \
    ASSERT(strcmp(define.value, (expectedValue)) == 0);


static ReturnCode testGatherDefines(void)
{
    struct GatheredDefines defines;
    FILE *settingsFile = fopen("test/test_gather_defines.txt", "rb");
#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(settingsFile);
    ASSERT_ENSURE(gatherDefines(settingsFile, &defines));
#undef RETURN_CALLBACK
#define RETURN_CALLBACK fclose(settingsFile); freeGatheredDefines(defines);
    ASSERT(defines.size == 2);
    ASSERT_EQUAL(defines.defines[0], "nr1", "content\r\n");
    ASSERT_EQUAL(defines.defines[1], "nr2", "content line 1\r\ncontent line 2\r\n\r\ncontent line 3\r\nend");
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}
#undef ASSERT_EQUAL


#define ASSERT_EQUAL_SHT(first, second) \
    ASSERT(basicCompareTimestamp((first).timestamp, (second).timestamp) == 0); \
    ASSERT((first).type == (second).type); \
    ASSERT((first).args.shutdown.delay == (second).args.shutdown.delay); \
    ASSERT((first).repeatPeriod == (second).repeatPeriod)

#define ASSERT_EQUAL_RST(first, second) \
    ASSERT(basicCompareTimestamp((first).timestamp, (second).timestamp) == 0); \
    ASSERT((first).type == (second).type); \
    ASSERT((first).repeatPeriod == (second).repeatPeriod)

#define ASSERT_EQUAL_NFY(first, second) \
    ASSERT(basicCompareTimestamp((first).timestamp, (second).timestamp) == 0); \
    ASSERT((first).type == (second).type); \
    ASSERT((first).args.notify.repeats == (second).args.notify.repeats); \
    ASSERT(strcmp((first).args.notify.fileName, (second).args.notify.fileName) == 0); \
    ASSERT((first).repeatPeriod == (second).repeatPeriod)


static ReturnCode testFitDefine(void)
{
    char *names[4] = {"define1", "def2ine", "3define", "4"};
    char *bodies[4] = {"3.02 11:30 shutdown 35", "$0 10:00 $1\r\n$0 11:00 $1\r\n$0 12:00 $1",
                             "12.09 22:$0 reset", "$0:$1 $2 $3 $4 $5"};
    struct StringPair definePairs[4];
    for(int i = 0; i < 4; i++)
    {
        definePairs[i].key = names[i];
        definePairs[i].value = bodies[i];
        definePairs[i].valueSize = strlen(bodies[i]) + 1;
    }
    struct GatheredDefines defines;
    defines.size = 4;
    defines.defines = definePairs;
    struct YearTimestamp now = {{.date = {1, 1}, .time = {0, 0}}, .currentYear = 2010};
    struct AllActions actions = {.queueHead = NULL, .shutdownClock = {.type = SHUTDOWN}};
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&actions.queueHead);

    ASSERT(fitDefine("muu", sizeof("muu"), &actions, defines, now) == RET_FAILURE);
    ASSERT(fitDefine("hehe xd", sizeof("hehe xd"), &actions, defines, now) == RET_FAILURE);
    ASSERT(fitDefine("3define(ab)", sizeof("3define(ab)"), &actions, defines, now) == RET_ERROR);
    ASSERT(fitDefine("//3define(ab)", sizeof("//3define(ab)"), &actions, defines, now) == RET_SUCCESS);

    const char line1[] = "define1";
    ASSERT_ENSURE(fitDefine(line1, sizeof(line1), &actions, defines, now));
    ASSERT_EQUAL_SHT(AQ_FIRST(actions.queueHead), ((struct Action) {{{3, 2}, {11, 30}}, SHUTDOWN, .args.shutdown = {35}}));

    const char line2[] = "def2ine(22.07, reset)";
    ASSERT_ENSURE(fitDefine(line2, sizeof(line2), &actions, defines, now));
    ASSERT_EQUAL_RST(AQ_SECOND(actions.queueHead), ((struct Action) {{{22, 7}, {10, 00}}, RESET, {{0}}, false}));
    ASSERT_EQUAL_RST(AQ_THIRD(actions.queueHead), ((struct Action) {{{22, 7}, {11, 00}}, RESET, {{0}}, false}));
    ASSERT_EQUAL_RST(AQ_FOURTH(actions.queueHead), ((struct Action) {{{22, 7}, {12, 00}}, RESET, {{0}}, false}));

    destroyActionQueue(&actions.queueHead); actions.queueHead = NULL;
    const char line3[] = "3define(16)";
    ASSERT_ENSURE(fitDefine(line3, sizeof(line3), &actions, defines, now));
    ASSERT_EQUAL_RST(AQ_FIRST(actions.queueHead), ((struct Action) {{{12, 9}, {22, 16}}, RESET, {{0}}, false}));

    destroyActionQueue(&actions.queueHead); actions.queueHead = NULL;
    const char line4[] = "4(11, 11, notify, hehe.wav, 5)", line5[] = "4(1.03 11, 12, reset)";
    ASSERT_ENSURE(fitDefine(line4, sizeof(line4), &actions, defines, now));
    ASSERT_ENSURE(fitDefine(line5, sizeof(line5), &actions, defines, now));
    ASSERT_EQUAL_NFY(AQ_FIRST(actions.queueHead), ((struct Action) {{{1, 1}, {11, 11}}, NOTIFY,
                     {.notify = {5, "hehe.wav"}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_RST(AQ_SECOND(actions.queueHead), ((struct Action) {{{1, 3}, {11, 12}}, RESET, {{0}}, false}));

    destroyActionQueue(&actions.queueHead);
#undef RETURN_CALLBACK
#define RETURN_CALLBACK
    return RET_SUCCESS;
}


static ReturnCode testLoadActionsFromFileWithPreprocessing(void)
{
    struct AllActions results = {.queueHead = NULL, .shutdownClock = {.type = SHUTDOWN}};
#undef RETURN_CALLBACK
#define RETURN_CALLBACK destroyActionQueue(&results.queueHead);
    ASSERT_ENSURE(loadActionsFromFile(&results, "test/test_preprocessed.txt",
                  (struct YearTimestamp) {{{1, 1}, {0, 0}}, 2020}));

    ASSERT_EQUAL_SHT(AQ_FIRST(results.queueHead), ((struct Action) {{{1, 1}, {0, 20}}, SHUTDOWN,
                     {.shutdown = {DEFAULT_SHUTDOWN_DELAY}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_SHT(AQ_SECOND(results.queueHead), ((struct Action) {{{1, 1}, {0, 20}}, SHUTDOWN,
                     {.shutdown = {12}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_SHT(AQ_THIRD(results.queueHead), ((struct Action) {{{1, 1}, {0, 20}}, SHUTDOWN,
                     {.shutdown = {10}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_SHT(AQ_FOURTH(results.queueHead), ((struct Action) {{{1, 1}, {0, 20}}, SHUTDOWN,
                     {.shutdown = {9}}, MINUTES_IN_DAY}));

    ASSERT_EQUAL_NFY(AQ_FIFTH(results.queueHead), ((struct Action) {{{1, 1}, {0, 40}}, NOTIFY,
                     {.notify = {DEFAULT_NOTIFY_SOUND_REPEATS, DEFAULT_NOTIFY_SOUND}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_NFY(AQ_SIXTH(results.queueHead), ((struct Action) {{{1, 1}, {0, 40}}, NOTIFY,
                     {.notify = {DEFAULT_NOTIFY_SOUND_REPEATS, DEFAULT_NOTIFY_SOUND}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_NFY(AQ_SEVENTH(results.queueHead), ((struct Action) {{{1, 1}, {0, 40}}, NOTIFY,
                     {.notify = {1, "bruh.mp4"}}, MINUTES_IN_DAY}));
    ASSERT_EQUAL_NFY(AQ_EIGHTH(results.queueHead), ((struct Action) {{{1, 1}, {0, 40}}, NOTIFY,
                     {.notify = {2, "bruh.wav"}}, MINUTES_IN_DAY}));
    return RET_SUCCESS;
}


PREPARE_TESTING(preprocessing,
    testVerifyDefineName,
    testGatherDefines,
    testFitDefine,
    testLoadActionsFromFileWithPreprocessing
)
