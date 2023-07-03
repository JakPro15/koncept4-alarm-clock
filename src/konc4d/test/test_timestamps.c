#include "test_konc4d.h"
#include "timestamps.h"

#define TOD(h, m) (struct TimeOfDay) {(h), (m)}
#define TSP(d, mon, h, m) (struct Timestamp) {{(d), (mon)}, {(h), (m)}}


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


PREPARE_TESTING(timestamps,
    testBasicCompareTime,
    testCompareTimeRegular,
    testCompareTimeCurrentBetween,
    testBasicCompareTimestamp,
    testCompareTimestampRegular,
    testCompareTimestampCurrentBetween
)
