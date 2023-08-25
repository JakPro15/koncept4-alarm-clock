#include "test_commons.h"
#include "timestamps.h"

#define TOD(h, m) (struct TimeOfDay) {(h), (m)}
#define DOY(d, m) (struct DateOfYear) {(d), (m)}
#define TSP(d, mon, h, m) (struct Timestamp) {{(d), (mon)}, {(h), (m)}}

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


ReturnCode testBasicCompareTime(void)
{
    ASSERT(basicCompareTime(TOD(0, 0), TOD(23, 59)) == -1);
    ASSERT(basicCompareTime(TOD(23, 59), TOD(0, 0)) == 1);
    ASSERT(basicCompareTime(TOD(0, 0), TOD(0, 0)) == 0);

    ASSERT(basicCompareTime(TOD(12, 30), TOD(12, 30)) == 0);
    ASSERT(basicCompareTime(TOD(12, 30), TOD(12, 31)) == -1);
    ASSERT(basicCompareTime(TOD(12, 30), TOD(12, 29)) == 1);
    return RET_SUCCESS;
}


ReturnCode testCompareTimeRegular(void)
{
    ASSERT(compareTime(TOD(12, 30), TOD(13, 30), TOD(7, 0)) == -1);
    ASSERT(compareTime(TOD(12, 30), TOD(12, 30), TOD(7, 0)) == 0);
    ASSERT(compareTime(TOD(12, 30), TOD(11, 30), TOD(7, 0)) == 1);

    ASSERT(compareTime(TOD(12, 30), TOD(13, 30), TOD(15, 0)) == -1);
    ASSERT(compareTime(TOD(12, 30), TOD(12, 30), TOD(15, 0)) == 0);
    ASSERT(compareTime(TOD(12, 30), TOD(11, 30), TOD(15, 0)) == 1);
    return RET_SUCCESS;
}


ReturnCode testCompareTimeCurrentBetween(void)
{
    ASSERT(compareTime(TOD(12, 30), TOD(13, 30), TOD(13, 0)) == 1);
    ASSERT(compareTime(TOD(13, 30), TOD(12, 30), TOD(13, 0)) == -1);

    ASSERT(compareTime(TOD(23, 59), TOD(0, 0), TOD(23, 58)) == -1);
    ASSERT(compareTime(TOD(0, 0), TOD(23, 59), TOD(23, 58)) == 1);

    ASSERT(compareTime(TOD(23, 59), TOD(0, 0), TOD(0, 1)) == -1);
    ASSERT(compareTime(TOD(0, 0), TOD(23, 59), TOD(0, 1)) == 1);
    return RET_SUCCESS;
}


ReturnCode testBasicCompareDate(void)
{
    ASSERT(basicCompareDate(DOY(1, 1), DOY(12, 1)) == -1);
    ASSERT(basicCompareDate(DOY(12, 12), DOY(31, 1)) == 1);
    ASSERT(basicCompareDate(DOY(3, 3), DOY(3, 3)) == 0);

    ASSERT(basicCompareDate(DOY(7, 10), DOY(7, 10)) == 0);
    ASSERT(basicCompareDate(DOY(7, 9), DOY(8, 9)) == -1);
    ASSERT(basicCompareDate(DOY(7, 8), DOY(6, 8)) == 1);
    return RET_SUCCESS;
}


ReturnCode testBasicCompareTimestamp(void)
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


ReturnCode testCompareTimestampRegular(void)
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


ReturnCode testCompareTimestampCurrentBetween(void)
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


ReturnCode testIsDateValid(void)
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


ReturnCode testGetNextDay(void)
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


ReturnCode testAddMinutes(void)
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


PREPARE_TESTING(timestamps,
    testBasicCompareTime,
    testCompareTimeRegular,
    testCompareTimeCurrentBetween,
    testBasicCompareDate,
    testBasicCompareTimestamp,
    testCompareTimestampRegular,
    testCompareTimestampCurrentBetween,
    testIsDateValid,
    testGetNextDay,
    testAddMinutes
)
