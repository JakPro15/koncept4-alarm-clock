#include "test_commons.h"
#include "timestamps.h"

#define TOD(h, m) (struct TimeOfDay) {(h), (m)}
#define DOY(d, m) (struct DateOfYear) {(d), (m)}
#define TSP(d, mon, h, m) (struct Timestamp) {{(d), (mon)}, {(h), (m)}}

#define TOD_EQUAL(time1, h, m) do { \
    struct TimeOfDay firstTime = (time1); \
    ASSERT(firstTime.hour == (unsigned) (h)); \
    ASSERT(firstTime.minute == (unsigned) (m)); \
} while(0)

#define YDATE(d, m, y) (struct YearTimestamp) {{{(d), (m)}, {0, 0}}, (y)}
#define DATE_EQUAL(date1, d, m) do { \
    struct DateOfYear firstDate = (date1); \
    ASSERT(firstDate.day == (unsigned) (d)); \
    ASSERT(firstDate.month == (unsigned) (m)); \
} while(0)

#define YTS(d, mon, h, m, y) (struct YearTimestamp) {{{(d), (mon)}, {(h), (m)}}, (y)}
#define YTS_EQUAL(yeartimestamp1, d, mon, h, m, y) do { \
    struct YearTimestamp yts = (yeartimestamp1); \
    ASSERT(yts.timestamp.date.day == (unsigned) (d)); \
    ASSERT(yts.timestamp.date.month == (unsigned) (mon)); \
    ASSERT(yts.timestamp.time.hour == (unsigned) (h)); \
    ASSERT(yts.timestamp.time.minute == (unsigned) (m)); \
    ASSERT(yts.currentYear == (unsigned) (y)); \
} while(0)


static ReturnCode testBasicCompareTime(void)
{
    ASSERT(basicCompareTime(TOD(0, 0), TOD(23, 59)) == -1);
    ASSERT(basicCompareTime(TOD(23, 59), TOD(0, 0)) == 1);
    ASSERT(basicCompareTime(TOD(0, 0), TOD(0, 0)) == 0);

    ASSERT(basicCompareTime(TOD(12, 30), TOD(12, 30)) == 0);
    ASSERT(basicCompareTime(TOD(12, 30), TOD(12, 31)) == -1);
    ASSERT(basicCompareTime(TOD(12, 30), TOD(12, 29)) == 1);
    return RET_SUCCESS;
}


static ReturnCode testCompareTimeRegular(void)
{
    ASSERT(compareTime(TOD(12, 30), TOD(13, 30), TOD(7, 0)) == -1);
    ASSERT(compareTime(TOD(12, 30), TOD(12, 30), TOD(7, 0)) == 0);
    ASSERT(compareTime(TOD(12, 30), TOD(11, 30), TOD(7, 0)) == 1);

    ASSERT(compareTime(TOD(12, 30), TOD(13, 30), TOD(15, 0)) == -1);
    ASSERT(compareTime(TOD(12, 30), TOD(12, 30), TOD(15, 0)) == 0);
    ASSERT(compareTime(TOD(12, 30), TOD(11, 30), TOD(15, 0)) == 1);
    return RET_SUCCESS;
}


static ReturnCode testCompareTimeCurrentBetween(void)
{
    ASSERT(compareTime(TOD(12, 30), TOD(13, 30), TOD(13, 0)) == 1);
    ASSERT(compareTime(TOD(13, 30), TOD(12, 30), TOD(13, 0)) == -1);

    ASSERT(compareTime(TOD(23, 59), TOD(0, 0), TOD(23, 58)) == -1);
    ASSERT(compareTime(TOD(0, 0), TOD(23, 59), TOD(23, 58)) == 1);

    ASSERT(compareTime(TOD(23, 59), TOD(0, 0), TOD(0, 1)) == -1);
    ASSERT(compareTime(TOD(0, 0), TOD(23, 59), TOD(0, 1)) == 1);
    return RET_SUCCESS;
}


static ReturnCode testBasicCompareDate(void)
{
    ASSERT(basicCompareDate(DOY(1, 1), DOY(12, 1)) == -1);
    ASSERT(basicCompareDate(DOY(12, 12), DOY(31, 1)) == 1);
    ASSERT(basicCompareDate(DOY(3, 3), DOY(3, 3)) == 0);

    ASSERT(basicCompareDate(DOY(7, 10), DOY(7, 10)) == 0);
    ASSERT(basicCompareDate(DOY(7, 9), DOY(8, 9)) == -1);
    ASSERT(basicCompareDate(DOY(7, 8), DOY(6, 8)) == 1);
    return RET_SUCCESS;
}


static ReturnCode testBasicCompareTimestamp(void)
{
    ASSERT(basicCompareTimestamp(TSP(1, 1, 12, 30), TSP(1, 1, 12, 40)) == -1);
    ASSERT(basicCompareTimestamp(TSP(14, 2, 23, 59), TSP(14, 2, 0, 0)) == 1);
    ASSERT(basicCompareTimestamp(TSP(31, 12, 0, 0), TSP(31, 12, 0, 0)) == 0);

    ASSERT(basicCompareTimestamp(TSP(22, 3, 12, 30), TSP(22, 3, 12, 30)) == 0);
    ASSERT(basicCompareTimestamp(TSP(30, 5, 12, 30), TSP(30, 5, 12, 31)) == -1);
    ASSERT(basicCompareTimestamp(TSP(30, 6, 12, 30), TSP(30, 6, 12, 29)) == 1);

    ASSERT(basicCompareTimestamp(TSP(1, 1, 12, 30), TSP(1, 2, 12, 40)) == -1);
    ASSERT(basicCompareTimestamp(TSP(14, 2, 23, 59), TSP(18, 1, 0, 0)) == 1);

    ASSERT(basicCompareTimestamp(TSP(31, 12, 12, 30), TSP(1, 1, 12, 31)) == 1);
    ASSERT(basicCompareTimestamp(TSP(30, 5, 12, 30), TSP(30, 6, 12, 29)) == -1);
    return RET_SUCCESS;
}


static ReturnCode testCompareTimestampRegular(void)
{
    ASSERT(compareTimestamp(TSP(1, 1, 12, 30), TSP(1, 1, 12, 40), TSP(5, 5, 0, 0)) == -1);
    ASSERT(compareTimestamp(TSP(14, 2, 23, 59), TSP(14, 2, 0, 0), TSP(5, 5, 0, 0)) == 1);
    ASSERT(compareTimestamp(TSP(31, 12, 0, 0), TSP(31, 12, 0, 0), TSP(5, 5, 0, 0)) == 0);

    ASSERT(compareTimestamp(TSP(22, 3, 12, 30), TSP(22, 3, 12, 30), TSP(5, 5, 0, 0)) == 0);
    ASSERT(compareTimestamp(TSP(30, 5, 12, 30), TSP(30, 5, 12, 31), TSP(5, 5, 0, 0)) == -1);
    ASSERT(compareTimestamp(TSP(30, 6, 12, 30), TSP(30, 6, 12, 29), TSP(5, 5, 0, 0)) == 1);

    ASSERT(compareTimestamp(TSP(1, 1, 12, 30), TSP(1, 2, 12, 40), TSP(5, 5, 0, 0)) == -1);
    ASSERT(compareTimestamp(TSP(14, 2, 23, 59), TSP(18, 1, 0, 0), TSP(5, 5, 0, 0)) == 1);

    ASSERT(compareTimestamp(TSP(31, 12, 12, 30), TSP(1, 1, 12, 31), TSP(1, 1, 0, 0)) == 1);
    ASSERT(compareTimestamp(TSP(30, 5, 12, 30), TSP(30, 6, 12, 29), TSP(5, 5, 0, 0)) == -1);
    return RET_SUCCESS;
}


static ReturnCode testCompareTimestampCurrentBetween(void)
{
    ASSERT(compareTimestamp(TSP(1, 1, 12, 30), TSP(1, 1, 12, 40), TSP(1, 1, 12, 35)) == 1);
    ASSERT(compareTimestamp(TSP(14, 2, 23, 59), TSP(14, 2, 0, 0), TSP(14, 2, 22, 0)) == -1);
    ASSERT(compareTimestamp(TSP(31, 12, 0, 0), TSP(31, 12, 0, 0), TSP(31, 12, 0, 0)) == 0);

    ASSERT(compareTimestamp(TSP(22, 3, 12, 30), TSP(22, 3, 12, 30), TSP(22, 3, 12, 30)) == 0);
    ASSERT(compareTimestamp(TSP(30, 5, 12, 30), TSP(30, 5, 12, 32), TSP(30, 5, 12, 31)) == 1);
    ASSERT(compareTimestamp(TSP(30, 6, 12, 30), TSP(30, 6, 12, 28), TSP(30, 6, 12, 29)) == -1);

    ASSERT(compareTimestamp(TSP(1, 1, 12, 30), TSP(1, 2, 12, 40), TSP(1, 2, 12, 39)) == 1);
    ASSERT(compareTimestamp(TSP(14, 2, 23, 59), TSP(18, 1, 0, 0), TSP(1, 2, 12, 39)) == -1);

    ASSERT(compareTimestamp(TSP(31, 12, 12, 30), TSP(1, 1, 12, 31), TSP(1, 1, 23, 0)) == -1);
    ASSERT(compareTimestamp(TSP(30, 5, 12, 30), TSP(30, 6, 12, 29), TSP(1, 6, 0, 0)) == 1);
    return RET_SUCCESS;
}


static ReturnCode testCompareYearTimestamp(void)
{
    ASSERT(compareYearTimestamp(YTS(1, 1, 12, 30, 2022), YTS(1, 1, 12, 40, 2021)) == 1);
    ASSERT(compareYearTimestamp(YTS(14, 2, 23, 59, 2021), YTS(14, 2, 0, 0, 2023)) == -1);
    ASSERT(compareYearTimestamp(YTS(31, 12, 0, 0, 2023), YTS(31, 12, 0, 0, 2023)) == 0);

    ASSERT(compareYearTimestamp(YTS(22, 3, 12, 30, 2022), YTS(22, 3, 12, 30, 2021)) == 1);
    ASSERT(compareYearTimestamp(YTS(30, 5, 12, 30, 2019), YTS(30, 5, 12, 32, 2021)) == -1);
    ASSERT(compareYearTimestamp(YTS(30, 6, 12, 30, 2020), YTS(30, 6, 12, 28, 2021)) == -1);
    return RET_SUCCESS;
}


static ReturnCode testIsTimeValid(void)
{
    ASSERT(isTimeValid((struct TimeOfDay) {23, 59}) == true);
    ASSERT(isTimeValid((struct TimeOfDay) {0, 0}) == true);
    ASSERT(isTimeValid((struct TimeOfDay) {12, 0}) == true);
    ASSERT(isTimeValid((struct TimeOfDay) {24, 0}) == false);
    ASSERT(isTimeValid((struct TimeOfDay) {4, 60}) == false);
    ASSERT(isTimeValid((struct TimeOfDay) {25, 71}) == false);
    return RET_SUCCESS;
}


static ReturnCode testIsDateValid(void)
{
    ASSERT(isDateValid((struct DateOfYear) {0, 3}, 2020) == false);
    ASSERT(isDateValid((struct DateOfYear) {0, 12}, 2020) == false);
    ASSERT(isDateValid((struct DateOfYear) {6, 0}, 2020) == false);
    ASSERT(isDateValid((struct DateOfYear) {12, 13}, 2020) == false);
    ASSERT(isDateValid((struct DateOfYear) {1, 1}, 2020) == true);
    ASSERT(isDateValid((struct DateOfYear) {8, 10}, 2019) == true);
    unsigned endings[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for(int i = 0; i < 12; i++)
    {
        ASSERT(isDateValid((struct DateOfYear) {endings[i], i + 1}, 2018) == true);
        ASSERT(isDateValid((struct DateOfYear) {endings[i] + 1, i + 1}, 2018) == false);
    }
    ASSERT(isDateValid((struct DateOfYear) {29, 2}, 2020) == true);
    ASSERT(isDateValid((struct DateOfYear) {30, 2}, 2020) == false);
    return RET_SUCCESS;
}


static ReturnCode testGetNextDay(void)
{
    DATE_EQUAL(getNextDay(YDATE(1, 1, 2020)), 2, 1);
    DATE_EQUAL(getNextDay(YDATE(8, 10, 2019)), 9, 10);
    unsigned endings[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for(int i = 0; i < 12; i++)
    {
        DATE_EQUAL(getNextDay(YDATE(endings[i] - 1, i + 1, 2018)), endings[i], i + 1);
        DATE_EQUAL(getNextDay(YDATE(endings[i], i + 1, 2018)), 1, (i + 1) % 12 + 1);
    }
    DATE_EQUAL(getNextDay(YDATE(28, 2, 2020)), 29, 2);
    DATE_EQUAL(getNextDay(YDATE(29, 2, 2020)), 1, 3);
    return RET_SUCCESS;
}


static ReturnCode testAddMinutes(void)
{
    YTS_EQUAL(addMinutes(YTS(1, 1, 12, 0, 2020), 5), 1, 1, 12, 5, 2020);
    YTS_EQUAL(addMinutes(YTS(8, 8, 4, 49, 2020), 11), 8, 8, 5, 0, 2020);
    YTS_EQUAL(addMinutes(YTS(20, 11, 21, 12, 2020), 4), 20, 11, 21, 16, 2020);
    YTS_EQUAL(addMinutes(YTS(30, 12, 23, 50, 2020), 15), 31, 12, 0, 5, 2020);

    YTS_EQUAL(addMinutes(YTS(28, 2, 3, 59, 2021), 50), 28, 2, 4, 49, 2021);
    YTS_EQUAL(addMinutes(YTS(28, 2, 23, 59, 2021), 50), 1, 3, 0, 49, 2021);
    YTS_EQUAL(addMinutes(YTS(28, 2, 23, 59, 2024), 50), 29, 2, 0, 49, 2024);
    YTS_EQUAL(addMinutes(YTS(28, 2, 23, 59, 2024), 50 + MINUTES_IN_DAY), 1, 3, 0, 49, 2024);

    YTS_EQUAL(addMinutes(YTS(31, 12, 12, 0, 2020), 13 * 60), 1, 1, 1, 0, 2021);
    YTS_EQUAL(addMinutes(YTS(2, 1, 12, 0, 2020), 3 * 365 * MINUTES_IN_DAY), 1, 1, 12, 0, 2023);
    return RET_SUCCESS;
}


static ReturnCode testIncrementTime(void)
{
    struct TimeOfDay time = {0, 0};
    incrementTime(&time);
    TOD_EQUAL(time, 0, 1);
    incrementTime(&time);
    TOD_EQUAL(time, 0, 2);
    time = TOD(0, 59);
    incrementTime(&time);
    TOD_EQUAL(time, 1, 0);
    time = TOD(23, 59);
    incrementTime(&time);
    TOD_EQUAL(time, 24, 0);
    incrementTime(&time);
    TOD_EQUAL(time, 24, 1);
    return RET_SUCCESS;
}


static ReturnCode testDecrementedTime(void)
{
    struct TimeOfDay time = {23, 59};
    time = decrementedTime(time);
    TOD_EQUAL(time, 23, 58);
    time = decrementedTime(time);
    TOD_EQUAL(time, 23, 57);
    time = TOD(23, 0);
    time = decrementedTime(time);
    TOD_EQUAL(time, 22, 59);
    time = TOD(0, 0);
    time = decrementedTime(time);
    TOD_EQUAL(time, 23, 59);
    return RET_SUCCESS;
}


static ReturnCode testDifference(void)
{
    struct YearTimestamp timestamp = YTS(15, 10, 12, 0, 2023);
    ASSERT(difference(YTS(15, 10, 12, 0, 2022), timestamp) == 525600);
    ASSERT(difference(YTS(2, 9, 12, 0, 2023), timestamp) == 61920);
    ASSERT(difference(YTS(14, 10, 11, 15, 2023), timestamp) == 1485);
    ASSERT(difference(YTS(15, 10, 10, 0, 2023), timestamp) == 120);
    ASSERT(difference(YTS(15, 10, 11, 15, 2023), timestamp) == 45);
    ASSERT(difference(YTS(15, 10, 12, 0, 2023), timestamp) == 0);
    ASSERT(difference(timestamp, YTS(15, 10, 12, 0, 2023)) == 0);
    ASSERT(difference(timestamp, YTS(15, 10, 12, 45, 2023)) == 45);
    ASSERT(difference(timestamp, YTS(15, 10, 14, 0, 2023)) == 120);
    ASSERT(difference(timestamp, YTS(16, 10, 12, 45, 2023)) == 1485);
    ASSERT(difference(timestamp, YTS(28, 11, 12, 0, 2023)) == 63360);
    ASSERT(difference(timestamp, YTS(15, 10, 12, 0, 2024)) == 527040);
    return RET_SUCCESS;
}


static ReturnCode testDeduceYear(void)
{
    YTS_EQUAL(deduceYear(TSP(23, 10, 11, 30), YTS(23, 10, 11, 0, 2023)), 23, 10, 11, 30, 2023);
    YTS_EQUAL(deduceYear(TSP(23, 10, 10, 30), YTS(23, 10, 11, 0, 2023)), 23, 10, 10, 30, 2024);
    YTS_EQUAL(deduceYear(TSP(1, 1, 0, 0), YTS(23, 10, 11, 0, 2020)), 1, 1, 0, 0, 2021);
    YTS_EQUAL(deduceYear(TSP(31, 12, 23, 59), YTS(23, 10, 11, 0, 2020)), 31, 12, 23, 59, 2020);
    // can return an invalid date
    YTS_EQUAL(deduceYear(TSP(29, 2, 10, 0), YTS(23, 10, 11, 0, 2020)), 29, 2, 10, 0, 2021);
    YTS_EQUAL(deduceYear(TSP(29, 2, 10, 0), YTS(23, 1, 11, 0, 2020)), 29, 2, 10, 0, 2020);
    return RET_SUCCESS;
}


static ReturnCode testDeduceTimestamp(void)
{
    YTS_EQUAL(deduceTimestamp(TOD(11, 30), YTS(23, 10, 11, 0, 2023)), 23, 10, 11, 30, 2023);
    YTS_EQUAL(deduceTimestamp(TOD(10, 30), YTS(23, 10, 11, 0, 2023)), 24, 10, 10, 30, 2023);
    YTS_EQUAL(deduceTimestamp(TOD(10, 30), YTS(31, 10, 11, 0, 2023)), 1, 11, 10, 30, 2023);
    YTS_EQUAL(deduceTimestamp(TOD(10, 30), YTS(31, 12, 11, 0, 2023)), 1, 1, 10, 30, 2024);

    YTS_EQUAL(deduceTimestamp(TOD(11, 30), YTS(29, 2, 11, 0, 2024)), 29, 2, 11, 30, 2024);
    YTS_EQUAL(deduceTimestamp(TOD(10, 30), YTS(28, 2, 11, 0, 2023)), 1, 3, 10, 30, 2023);
    YTS_EQUAL(deduceTimestamp(TOD(10, 30), YTS(28, 2, 11, 0, 2024)), 29, 2, 10, 30, 2024);
    return RET_SUCCESS;
}


PREPARE_TESTING(timestamps,
    testBasicCompareTime,
    testCompareTimeRegular,
    testCompareTimeCurrentBetween,
    testBasicCompareDate,
    testBasicCompareTimestamp,
    testCompareTimestampRegular,
    testCompareTimestampCurrentBetween,
    testCompareYearTimestamp,
    testIsTimeValid,
    testIsDateValid,
    testGetNextDay,
    testAddMinutes,
    testIncrementTime,
    testDecrementedTime,
    testDifference,
    testDeduceYear,
    testDeduceTimestamp
)
