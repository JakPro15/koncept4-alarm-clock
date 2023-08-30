#include "test_konc4d.h"
#include "actions.h"

#include <string.h>


static ReturnCode testNoDateReset(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("22:30 reset", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 1234}));
    ASSERT(parsed.type == RESET);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{21, 1}, {22, 30}}) == 0);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


static ReturnCode testNoDateShutdown(void)
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


static ReturnCode testNoDateNotify(void)
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


static ReturnCode testNoDateNotifyNoNumber(void)
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


static ReturnCode testNoDateNotifyNoFileName(void)
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


static ReturnCode testNoDateNotifySpacesFileName(void)
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


static ReturnCode testWithDateReset(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("12.09 22:30 reset", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 1234}));
    ASSERT(parsed.type == RESET);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{12, 9}, {22, 30}}) == 0);
    ASSERT(parsed.repeatPeriod == false);
    return RET_SUCCESS;
}


static ReturnCode testWithDateShutdown(void)
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


static ReturnCode test29February(void)
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


static ReturnCode testWithDateNotify(void)
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


static ReturnCode testWithDateNotifyNoNumber(void)
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


static ReturnCode testWithDateNotifyNoFileName(void)
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


static ReturnCode testNotifyTooLongFileName(void)
{
    struct Action parsed;
    ASSERT_THROW(parseAction("   1.1  15:00 \tnotify   hehexdlooooooooooooooooooooooooooooooooooooooooooong.wav  35  \v \t", &parsed,
                 (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    return RET_SUCCESS;
}


static ReturnCode testRepeatSpecifierDaily(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("\t daily   1.1  15:00 \tnotify  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{21, 1}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_DAY);
    return RET_SUCCESS;
}


static ReturnCode testRepeatSpecifierWeekly(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("weekly   1.1  15:00 \tnotify  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{22, 1}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeatPeriod == MINUTES_IN_WEEK);
    return RET_SUCCESS;
}


static ReturnCode testRepeatSpecifierMonthly(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("\t monthly   1.1  15:00 \tnotify  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{1, 2}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeatPeriod == MONTHLY_REPEAT);
    return RET_SUCCESS;
}


static ReturnCode testRepeatSpecifierPeriod(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("period 144   1.1  15:00 \tnotify  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{25, 1}, {15, 0}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeatPeriod == 144 * MINUTES_IN_HOUR);
    return RET_SUCCESS;
}


static ReturnCode testRepeatSpecifierPeriodFraction(void)
{
    struct Action parsed;
    ASSERT_ENSURE(parseAction("\t period 1.5   1.1  15:00 \tnotify  \v \t", &parsed,
                  (struct YearTimestamp) {{.date = {21, 1}, .time = {12, 30}}, .currentYear = 2010}));
    ASSERT(parsed.type == NOTIFY);
    ASSERT(basicCompareTimestamp(parsed.timestamp, (struct Timestamp) {{21, 1}, {13, 30}}) == 0);
    ASSERT(strcmp(parsed.args.notify.fileName, DEFAULT_NOTIFY_SOUND) == 0);
    ASSERT(parsed.args.notify.repeats == DEFAULT_NOTIFY_SOUND_REPEATS);
    ASSERT(parsed.repeatPeriod == 1.5 * MINUTES_IN_HOUR);
    return RET_SUCCESS;
}


PREPARE_TESTING(parse_action,
    testNoDateReset,
    testNoDateShutdown,
    testNoDateNotify,
    testNoDateNotifyNoNumber,
    testNoDateNotifyNoFileName,
    testNoDateNotifySpacesFileName,
    testWithDateReset,
    testWithDateShutdown,
    test29February,
    testWithDateNotify,
    testWithDateNotifyNoNumber,
    testWithDateNotifyNoFileName,
    testNotifyTooLongFileName,
    testRepeatSpecifierDaily,
    testRepeatSpecifierWeekly,
    testRepeatSpecifierMonthly,
    testRepeatSpecifierPeriod,
    testRepeatSpecifierPeriodFraction
)