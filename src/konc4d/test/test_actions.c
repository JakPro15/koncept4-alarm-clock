#include <string.h>
#include "test_konc4d.h"
#include "actions.h"


#define ASSERT_EQUAL(first, second) \
    ASSERT((first).timestamp.date.day == (second).timestamp.date.day); \
    ASSERT((first).timestamp.date.month == (second).timestamp.date.month); \
    ASSERT((first).timestamp.time.hour == (second).timestamp.time.hour); \
    ASSERT((first).timestamp.time.minute == (second).timestamp.time.minute); \
    ASSERT((first).type == (second).type); \
    ASSERT((first).args.shutdown.delay == (second).args.shutdown.delay)


ReturnCode testAddActionRegular(void)
{
    struct ActionQueue *head = NULL;
    struct Action action = {{{2, 1}, {0, 0}}, SHUTDOWN, .args.shutdown = {1}};
    struct Action action2 = {{{2, 1}, {10, 0}}, SHUTDOWN, .args.shutdown = {2}};

    addAction(&head, &action, (struct Timestamp) {{3, 1}, {23, 0}});
    ASSERT_EQUAL(action, head->action);
    ASSERT(head->next == NULL);

    addAction(&head, &action2, (struct Timestamp) {{3, 1}, {23, 0}});
    ASSERT_EQUAL(action, head->action);
    ASSERT(head->next != NULL);
    ASSERT_EQUAL(action2, head->next->action);
    ASSERT(head->next->next == NULL);

    action.timestamp.time.hour = 12;
    ASSERT(head->action.timestamp.time.hour == 0);

    return RET_SUCCESS;
}


ReturnCode testAddActionAtEnd(void)
{
    struct ActionQueue second = {NULL, {{{5, 12}, {10, 0}}, SHUTDOWN, .args.shutdown = {1}}};
    struct ActionQueue first = {&second, {{{2, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {2}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{2, 1}, {12, 30}}, SHUTDOWN, .args.shutdown = {3}};
    struct Action toAdd2 = {{{2, 1}, {21, 12}}, SHUTDOWN, .args.shutdown = {-1}};

    addAction(&head, &toAdd, (struct Timestamp) {{14, 2}, {23, 0}});
    ASSERT(head == &first);
    ASSERT(second.next != NULL);
    ASSERT(second.next->next == NULL);
    ASSERT_EQUAL(second.next->action, toAdd);

    addAction(&head, &toAdd2, (struct Timestamp) {{14, 2}, {23, 0}});
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

    addAction(&head, &toAdd, (struct Timestamp) {{15, 6}, {10, 0}});
    ASSERT(head == &first);
    ASSERT(second.next == NULL);
    ASSERT(first.next->next == &second);
    ASSERT_EQUAL(first.next->action, toAdd);

    addAction(&head, &toAdd2, (struct Timestamp) {{15, 6}, {10, 0}});
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

    addAction(&head, &toAdd, (struct Timestamp) {{1, 1}, {5, 0}});
    ASSERT(head->next == &first);
    ASSERT(first.next == NULL);
    ASSERT_EQUAL(head->action, toAdd);

    return RET_SUCCESS;
}


ReturnCode testPopAction(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, first = {{{6, 3}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}};
    struct Action second = {{{6, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {5}};
    addAction(&head, &first, (struct Timestamp) {{6, 3}, {5, 0}});
    addAction(&head, &second, (struct Timestamp) {{6, 3}, {5, 0}});

    popAction(&head, &toWrite);
    ASSERT_EQUAL(toWrite, first);
    popAction(&head, &toWrite);
    ASSERT_EQUAL(toWrite, second);
    ASSERT(head == NULL);

    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateReset(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("22:30 reset", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 1234}));
    ASSERT(parsed.type == RESET);
    ASSERT(parsed.timestamp.date.day == 21);
    ASSERT(parsed.timestamp.date.month == 1);
    ASSERT(parsed.timestamp.time.hour == 22);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(parsed.repeated == true);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateShutdown(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("  11:30 \tshutdown  35  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == SHUTDOWN);
    ASSERT(parsed.timestamp.date.day == 22);
    ASSERT(parsed.timestamp.date.month == 1);
    ASSERT(parsed.timestamp.time.hour == 11);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(parsed.args.shutdown.delay == 35);
    ASSERT(parsed.repeated == true);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotify(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("  11:30 \tnotify   hehexd.wav  35  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 3);
    ASSERT(parsed.timestamp.time.hour == 11);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 35);
    ASSERT(parsed.repeated == true);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotifyNoNumber(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("  11:30 \tnotify   hehexd.wav   \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 3);
    ASSERT(parsed.timestamp.time.hour == 11);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 1);
    ASSERT(parsed.repeated == true);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotifyNoFileName(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("  11:30 \tnotify    \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 3);
    ASSERT(parsed.timestamp.time.hour == 11);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeated == true);
    return RET_SUCCESS;
}


ReturnCode testParseActionNoDateNotifySpacesFileName(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("  11:30 \tnotify    \v \"Hehe Xd.wav\" \t", &parsed,
                   (struct YearTimestamp) {{.date = {28, 2}, .time = {12, 30}}, .currentYear = 2011}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 3);
    ASSERT(parsed.timestamp.time.hour == 11);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(strcmp(parsed.args.notify.fileName, "Hehe Xd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 1);
    ASSERT(parsed.repeated == true);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateReset(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("12.09 22:30 reset", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 1234}));
    ASSERT(parsed.type == RESET);
    ASSERT(parsed.timestamp.date.day == 12);
    ASSERT(parsed.timestamp.date.month == 9);
    ASSERT(parsed.timestamp.time.hour == 22);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(parsed.repeated == false);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateShutdown(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("    29.02  11:30 \tshutdown  35  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == SHUTDOWN);
    ASSERT(parsed.timestamp.date.day == 29);
    ASSERT(parsed.timestamp.date.month == 2);
    ASSERT(parsed.timestamp.time.hour == 11);
    ASSERT(parsed.timestamp.time.minute == 30);
    ASSERT(parsed.args.shutdown.delay == 35);
    ASSERT(parsed.repeated == false);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateNotify(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("   1.1  15:00 \tnotify   hehexd.wav  35  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 1);
    ASSERT(parsed.timestamp.time.hour == 15);
    ASSERT(parsed.timestamp.time.minute == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 35);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateNotifyNoNumber(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("   1.1  15:00 \tnotify   hehexd.wav  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 1);
    ASSERT(parsed.timestamp.time.hour == 15);
    ASSERT(parsed.timestamp.time.minute == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, "hehexd.wav") == 0);
    ASSERT(parsed.args.notify.repeats == 1);
    return RET_SUCCESS;
}


ReturnCode testParseActionWithDateNotifyNoFileName(void)
{
    struct Action parsed;
    ASSERT_NOTHROW(parseAction("   1.1  15:00 \tnotify  \v \t", &parsed,
                   (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(parsed.timestamp.date.day == 1);
    ASSERT(parsed.timestamp.date.month == 1);
    ASSERT(parsed.timestamp.time.hour == 15);
    ASSERT(parsed.timestamp.time.minute == 0);
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


PREPARE_TESTING(actions,
    testAddActionRegular,
    testAddActionAtEnd,
    testAddActionInTheMiddle,
    testAddActionAtHead,
    testPopAction,
    testParseActionNoDateReset,
    testParseActionNoDateShutdown,
    testParseActionNoDateNotify,
    testParseActionNoDateNotifyNoNumber,
    testParseActionNoDateNotifyNoFileName,
    testParseActionNoDateNotifySpacesFileName,
    testParseActionWithDateReset,
    testParseActionWithDateShutdown,
    testParseActionWithDateNotify,
    testParseActionWithDateNotifyNoNumber,
    testParseActionWithDateNotifyNoFileName,
    testParseActionNotifyTooLongFileName
)
