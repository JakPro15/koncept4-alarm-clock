#include <string.h>
#include <stdlib.h>
#include "test_konc4d.h"
#include "actions.h"


#define ASSERT_EQUAL(first, second) \
    ASSERT(basicCompareTimestamp((first).timestamp, (second).timestamp) == 0); \
    ASSERT((first).type == (second).type); \
    ASSERT((first).args.shutdown.delay == (second).args.shutdown.delay); \
    ASSERT((first).repeatPeriod == (second).repeatPeriod)


ReturnCode testAddActionRegular(void)
{
    struct ActionQueue *head = NULL;
    struct Action action = {{{2, 1}, {0, 0}}, SHUTDOWN, .args.shutdown = {1}};
    struct Action action2 = {{{2, 1}, {10, 0}}, SHUTDOWN, .args.shutdown = {2}};

    ASSERT_ENSURE(addAction(&head, &action, (struct Timestamp) {{3, 1}, {23, 0}}));
    ASSERT_EQUAL(action, AQ_FIRST(head));
    ASSERT(head->next == NULL);

    ASSERT_ENSURE(addAction(&head, &action2, (struct Timestamp) {{3, 1}, {23, 0}}));
    ASSERT_EQUAL(action, AQ_FIRST(head));
    ASSERT(head->next != NULL);
    ASSERT_EQUAL(action2, AQ_SECOND(head));
    ASSERT(head->next->next == NULL);

    action.timestamp.time.hour = 12;
    ASSERT(AQ_FIRST(head).timestamp.time.hour == 0);

    return RET_SUCCESS;
}


ReturnCode testAddActionAtEnd(void)
{
    struct ActionQueue second = {NULL, {{{5, 12}, {10, 0}}, SHUTDOWN, .args.shutdown = {1}}};
    struct ActionQueue first = {&second, {{{2, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {2}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{2, 1}, {12, 30}}, SHUTDOWN, .args.shutdown = {3}};
    struct Action toAdd2 = {{{2, 1}, {21, 12}}, SHUTDOWN, .args.shutdown = {-1}};

    ASSERT_ENSURE(addAction(&head, &toAdd, (struct Timestamp) {{14, 2}, {23, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next != NULL);
    ASSERT(second.next->next == NULL);
    ASSERT_EQUAL(second.next->action, toAdd);

    ASSERT_ENSURE(addAction(&head, &toAdd2, (struct Timestamp) {{14, 2}, {23, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next->next != NULL);
    ASSERT(second.next->next->next == NULL);
    ASSERT_EQUAL(second.next->next->action, toAdd2);

    return RET_SUCCESS;
}


ReturnCode testAddActionInTheMiddle(void)
{
    struct ActionQueue second = {NULL, {{{16, 6}, {5, 0}}, SHUTDOWN, .args.shutdown = {1}}};
    struct ActionQueue first = {&second, {{{15, 6}, {20, 0}}, SHUTDOWN, .args.shutdown = {2}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{16, 6}, {1, 30}}, SHUTDOWN, .args.shutdown = {5}};
    struct Action toAdd2 = {{{16, 6}, {3, 12}}, SHUTDOWN, .args.shutdown = {-1}};

    ASSERT_ENSURE(addAction(&head, &toAdd, (struct Timestamp) {{15, 6}, {10, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next == NULL);
    ASSERT(first.next->next == &second);
    ASSERT_EQUAL(first.next->action, toAdd);

    ASSERT_ENSURE(addAction(&head, &toAdd2, (struct Timestamp) {{15, 6}, {10, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next == NULL);
    ASSERT(first.next->next->next == &second);
    ASSERT_EQUAL(first.next->next->action, toAdd2);

    return RET_SUCCESS;
}


ReturnCode testAddActionAtHead(void)
{
    struct ActionQueue first = {NULL, {{{2, 1}, {11, 0}}, SHUTDOWN, .args.shutdown = {3}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{2, 1}, {6, 30}}, SHUTDOWN, .args.shutdown = {4}};

    ASSERT_ENSURE(addAction(&head, &toAdd, (struct Timestamp) {{1, 1}, {5, 0}}));
    ASSERT(head->next == &first);
    ASSERT(first.next == NULL);
    ASSERT_EQUAL(AQ_FIRST(head), toAdd);

    return RET_SUCCESS;
}


ReturnCode testPopAction(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, first = {{{6, 3}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}};
    struct Action second = {{{6, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {5}};
    ASSERT_ENSURE(addAction(&head, &first, (struct Timestamp) {{6, 3}, {5, 0}}));
    ASSERT_ENSURE(addAction(&head, &second, (struct Timestamp) {{6, 3}, {5, 0}}));

    ASSERT_ENSURE(popAction(&head, &toWrite));
    ASSERT_EQUAL(toWrite, first);
    ASSERT_ENSURE(popAction(&head, &toWrite));
    ASSERT_EQUAL(toWrite, second);
    ASSERT(head == NULL);

    return RET_SUCCESS;
}


ReturnCode testPopActionEmpty(void)
{
    struct ActionQueue *head = NULL;
    ASSERT_THROW(popAction(&head, NULL));
    return RET_SUCCESS;
}


ReturnCode testPopActionWithRepeat(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, first = {{{6, 3}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}, .repeatPeriod = MINUTES_IN_DAY};
    struct Action second = {{{6, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {5}, .repeatPeriod = false};
    struct YearTimestamp now = {{{6, 3}, {5, 0}}, 2015};
    ASSERT_ENSURE(addAction(&head, &first, now.timestamp));
    ASSERT_ENSURE(addAction(&head, &second, now.timestamp));

    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, first);
    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, second);

    for(int i = 0; i < 5; i++)
    {
        ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
        ASSERT(toWrite.type == SHUTDOWN);
        ASSERT(basicCompareTimestamp(toWrite.timestamp, (struct Timestamp) {{7 + i, 3}, {11, 0}}) == 0);
        ASSERT(toWrite.args.shutdown.delay == 1);
        ASSERT(toWrite.repeatPeriod == MINUTES_IN_DAY);
    }

    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateReset(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("22:30 reset", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 1234}));
    ASSERT(parsed.type == RESET);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{21, 1}, {22, 30}}) == 0);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateShutdown(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("  11:30 \tshutdown  35  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == SHUTDOWN);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{22, 1}, {11, 30}}) == 0);
    ASSERT(parsed.args.shutdown.delay == 35);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotify(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("  11:30 \tnotify   hehexd.wav  35  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 3}, {11, 30}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 35);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotifyNoNumber(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("  11:30 \tnotify   hehexd.wav   \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 3}, {11, 30}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 1);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotifyNoFileName(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("  11:30 \tnotify    \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 3}, {11, 30}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotifySpacesFileName(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("  11:30 \tnotify    \v \"Hehe Xd.wav\" \t", &parsed,
                 (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 3}, {11, 30}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "Hehe Xd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 1);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateReset(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("12.09 22:30 reset", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 1234}));
    ASSERT(parsed.type == RESET);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{12, 9}, {22, 30}}) == 0);
    ASSERT(parsed.repeatPeriod == false);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateShutdown(void)
{
    struct Action parsed;
    ASSERT_THROW(parseAction("    30.02  11:30 \tshutdown  35  \v \t", &parsed,
                 (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2020}));
    ASSERT(parseAction("    29.02  11:30 \tshutdown  35  \v \t", &parsed,
           (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}) == RET_FAILURE);
    ASSERT_ENSURE(parseAction("    29.02  11:30 \tshutdown  35  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2012}));
    ASSERT(parsed.type == SHUTDOWN);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(parsed.args.shutdown.delay == 35);
    ASSERT(parsed.repeatPeriod == false);
    return RET_SUCCESS;
}


ReturnCode testParseAction29February(void)
{
    struct Action parsed1, parsed2;
    ASSERT(parseAction("29.02 11:30 reset", &parsed1, (struct YearTimestamp) {{{1, 1}, {11, 30}}, 2021}) == RET_FAILURE);
    ASSERT(parseAction("29.02 11:30 reset", &parsed1, (struct YearTimestamp) {{{1, 1}, {11, 30}}, 2022}) == RET_FAILURE);
    ASSERT(parseAction("29.02 11:30 reset", &parsed1, (struct YearTimestamp) {{{1, 1}, {11, 30}}, 2023}) == RET_FAILURE);
    ASSERT(parseAction("29.02 11:30 reset", &parsed1, (struct YearTimestamp) {{{1, 1}, {11, 30}}, 2024}) == RET_SUCCESS);
    ASSERT(parsed1.type == RESET);
    ASSERT(basicCompareTimestamp(parsed1.timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(parsed1.repeatPeriod == false);

    ASSERT(parseAction("29.02 11:30 reset", &parsed2, (struct YearTimestamp) {{{1, 12}, {11, 30}}, 2020}) == RET_FAILURE);
    ASSERT(parseAction("29.02 11:30 reset", &parsed2, (struct YearTimestamp) {{{1, 12}, {11, 30}}, 2021}) == RET_FAILURE);
    ASSERT(parseAction("29.02 11:30 reset", &parsed2, (struct YearTimestamp) {{{1, 12}, {11, 30}}, 2022}) == RET_FAILURE);
    ASSERT(parseAction("29.02 11:30 reset", &parsed2, (struct YearTimestamp) {{{1, 12}, {11, 30}}, 2023}) == RET_SUCCESS);
    ASSERT(parsed2.type == RESET);
    ASSERT(basicCompareTimestamp(parsed2.timestamp, (struct Timestamp) {{29, 2}, {11, 30}}) == 0);
    ASSERT(parsed2.repeatPeriod == false);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateNotify(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("   1.1  15:00 \tnotify   hehexd.wav  35  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 1}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 35);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateNotifyNoNumber(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("   1.1  15:00 \tnotify   hehexd.wav  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 1}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 1);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateNotifyNoFileName(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("   1.1  15:00 \tnotify  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 1}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    return RET_SUCCESS;
}


ReturnCode testParseActionNotifyTooLongFileName(void)
{
    struct Action parsed;
    ASSERT_THROW(parseAction("   1.1  15:00 \tnotify   hehexdlooooooooooooooooooooooooooooooooooooooooooong.wav  35  \v \t", &parsed,
                 (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    return RET_SUCCESS;
}


ReturnCode testSkipUntilTimestamp(void)
{
    struct YearTimestamp now = {{{4, 7}, {14, 16}}, 2023};
    struct YearTimestamp until = {{{4, 7}, {14, 21}}, 2023};
    struct Action actions[] = {
        {{{4, 7}, {14, 20}}, RESET, {{0}}, false},
        {{{4, 7}, {14, 21}}, RESET, {{0}}, MINUTES_IN_DAY},
        {{{4, 7}, {14, 22}}, RESET, {{0}}, false},
        {{{3, 7}, {14, 15}}, RESET, {{0}}, false},
        {{{5, 7}, {14, 15}}, RESET, {{0}}, MINUTES_IN_DAY}
    };
    struct ActionQueue *head = NULL;
    for(int i = 0; i < 5; i++)
        ASSERT_ENSURE(addAction(&head, &actions[i], now.timestamp));
    ASSERT_ENSURE(skipUntilTimestamp(&head, until.timestamp, now));

    ASSERT(AQ_FIRST(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_FIRST(head).timestamp, actions[2].timestamp) == 0);
    ASSERT(AQ_FIRST(head).repeatPeriod == false);

    ASSERT(AQ_SECOND(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_SECOND(head).timestamp, actions[4].timestamp) == 0);
    ASSERT(AQ_SECOND(head).repeatPeriod == MINUTES_IN_DAY);

    ASSERT(AQ_THIRD(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_THIRD(head).timestamp, (struct Timestamp) {{5, 7}, {14, 21}}) == 0);
    ASSERT(AQ_THIRD(head).repeatPeriod == MINUTES_IN_DAY);

    ASSERT(AQ_FOURTH(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_FOURTH(head).timestamp, actions[3].timestamp) == 0);
    ASSERT(AQ_FOURTH(head).repeatPeriod == false);

    ASSERT(head->next->next->next->next == NULL);
    return RET_SUCCESS;
}


PREPARE_TESTING(actions,
    testAddActionRegular,
    testAddActionAtEnd,
    testAddActionInTheMiddle,
    testAddActionAtHead,
    testPopAction,
    testPopActionEmpty,
    testPopActionWithRepeat,
    testParseActionNoDateReset,
    testParseActionNoDateShutdown,
    testParseActionNoDateNotify,
    testParseActionNoDateNotifyNoNumber,
    testParseActionNoDateNotifyNoFileName,
    testParseActionNoDateNotifySpacesFileName,
    testParseActionWithDateReset,
    testParseActionWithDateShutdown,
    testParseAction29February,
    testParseActionWithDateNotify,
    testParseActionWithDateNotifyNoNumber,
    testParseActionWithDateNotifyNoFileName,
    testParseActionNotifyTooLongFileName,
    testSkipUntilTimestamp
)
