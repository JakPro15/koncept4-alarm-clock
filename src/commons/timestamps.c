#include <windows.h>

#include "timestamps.h"


struct YearTimestamp getCurrentTimestamp(void)
{
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    return (struct YearTimestamp) {
        {.date = (struct DateOfYear) {.day = systemTime.wDay, .month = systemTime.wMonth},
         .time = (struct TimeOfDay) {.hour = systemTime.wHour, .minute = systemTime.wMinute}},
        .currentYear = systemTime.wYear
    };
}


static bool isLeapYear(unsigned currentYear)
{
    return currentYear % 4 == 0 && (currentYear % 100 != 0 || currentYear % 400 == 0);
}


bool isTimeValid(struct TimeOfDay toValidate)
{
    return toValidate.hour <= 23 && toValidate.minute <= 59;
}


bool isDateValid(struct DateOfYear toValidate, unsigned year)
{
    return toValidate.day > 0 && toValidate.month > 0 && toValidate.month <= 12 &&
           getMonthLength(toValidate.month, year) >= toValidate.day;
}


struct DateOfYear getNextDay(struct YearTimestamp now)
{
    struct DateOfYear current = now.timestamp.date;
    switch(current.month)
    {
    case 4: case 6: case 9: case 11:
        if(current.day == 30)
            return (struct DateOfYear) {.day = 1, .month = current.month + 1};
        break;
    case 2:
        if(isLeapYear(now.currentYear))
        {
            if(current.day == 29)
                return (struct DateOfYear) {.day = 1, .month = 3};
        }
        else if(current.day == 28)
            return (struct DateOfYear) {.day = 1, .month = 3};
        break;
    default:
        if(current.day == 31)
            return (struct DateOfYear) {.day = 1, .month = current.month % 12 + 1};
    }
    return (struct DateOfYear) {.day = current.day + 1, .month = current.month};
}


static const unsigned monthLengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
unsigned getMonthLength(unsigned month, unsigned year)
{
    if(month == 2 && isLeapYear(year))
        return 29;
    else
        return monthLengths[month - 1];
}


static unsigned getYearLength(unsigned year)
{
    return isLeapYear(year) ? 366 : 365;
}


static unsigned getDayOfYear(struct DateOfYear date, unsigned year)
{
    unsigned day = 0;
    for(unsigned i = 1; i < date.month; i++)
        day += getMonthLength(i, year);
    day += date.day;
    return day;
}


static struct DateOfYear dayOfYearToDate(unsigned day, unsigned year)
{
    int month = 1;
    while((int) day - (int) getMonthLength(month, year) > 0)
        day -= getMonthLength(month++, year);
    return (struct DateOfYear) {day, month};
}


struct YearTimestamp addMinutes(struct YearTimestamp now, unsigned delay)
{
    struct YearTimestamp result = now;
    result.timestamp.time.minute += delay;

    result.timestamp.time.hour += result.timestamp.time.minute / 60;
    result.timestamp.time.minute %= 60;

    unsigned day = getDayOfYear(now.timestamp.date, now.currentYear);
    day += result.timestamp.time.hour / 24;
    result.timestamp.time.hour %= 24;

    while(day > getYearLength(result.currentYear))
        day -= getYearLength(result.currentYear++);

    result.timestamp.date = dayOfYearToDate(day, result.currentYear);
    return result;
}


void incrementTime(struct TimeOfDay *time)
{
    if(++time->minute >= 60)
    {
        time->minute = 0;
        ++time->hour;
    }
}


struct TimeOfDay decrementedTime(struct TimeOfDay time)
{
    if(time.minute == 0)
    {
        if(time.hour == 0)
            time.hour = 23;
        else
            --time.hour;
        time.minute = 59;
    }
    else
        --time.minute;
    return time;
}


unsigned difference(struct YearTimestamp earlier, struct YearTimestamp later)
{
    int diff = 0;
    while(later.currentYear > earlier.currentYear)
        diff += getYearLength(--later.currentYear) * MINUTES_IN_DAY;

    unsigned earlierDayOfYear = getDayOfYear(earlier.timestamp.date, earlier.currentYear);
    unsigned laterDayOfYear = getDayOfYear(later.timestamp.date, later.currentYear);
    diff += (laterDayOfYear - earlierDayOfYear) * MINUTES_IN_DAY;

    diff += (later.timestamp.time.hour - earlier.timestamp.time.hour) * MINUTES_IN_HOUR;
    diff += later.timestamp.time.minute - earlier.timestamp.time.minute;
    return (unsigned) diff;
}


struct YearTimestamp deduceYear(struct Timestamp toDeduce, struct YearTimestamp now)
{
    if(basicCompareTimestamp(toDeduce, now.timestamp) <= 0)
        return (struct YearTimestamp) {toDeduce, now.currentYear + 1};
    else
        return (struct YearTimestamp) {toDeduce, now.currentYear};
}


int basicCompareTime(const struct TimeOfDay first, const struct TimeOfDay second)
{
    if(first.hour > second.hour || (first.hour == second.hour && first.minute > second.minute))
        return 1;
    else if(first.hour == second.hour && first.minute == second.minute)
        return 0;
    else
        return -1;
}


int compareTime(const struct TimeOfDay first, const struct TimeOfDay second, const struct TimeOfDay now)
{
    if(basicCompareTime(first, now) < 0)
    {
        if(basicCompareTime(now, second) < 0)
            return 1;
        else
            return basicCompareTime(first, second);
    }
    else
    {
        if(basicCompareTime(second, now) < 0)
            return -1;
        else
            return basicCompareTime(first, second);
    }
}


int basicCompareDate(const struct DateOfYear first, const struct DateOfYear second)
{
    if(first.month > second.month || (first.month == second.month && first.day > second.day))
        return 1;
    else if(first.month == second.month && first.day == second.day)
        return 0;
    else
        return -1;
}


int basicCompareTimestamp(const struct Timestamp first, const struct Timestamp second)
{
    if(first.date.month > second.date.month)
        return 1;
    else if(first.date.month == second.date.month)
    {
        if(first.date.day > second.date.day)
            return 1;
        else if(first.date.day == second.date.day)
            return basicCompareTime(first.time, second.time);
    }
    return -1;
}


int compareTimestamp(const struct Timestamp first, const struct Timestamp second, const struct Timestamp now)
{
    if(basicCompareTimestamp(first, now) < 0)
    {
        if(basicCompareTimestamp(now, second) < 0)
            return 1;
        else
            return basicCompareTimestamp(first, second);
    }
    else
    {
        if(basicCompareTimestamp(second, now) < 0)
            return -1;
        else
            return basicCompareTimestamp(first, second);
    }
}


int compareYearTimestamp(const struct YearTimestamp first, const struct YearTimestamp second)
{
    if(first.currentYear != second.currentYear)
        return first.currentYear < second.currentYear ? -1 : 1;
    else
        return basicCompareTimestamp(first.timestamp, second.timestamp);
}
