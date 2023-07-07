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


inline static bool does29FebruaryExist(unsigned currentYear)
{
    return currentYear % 4 == 0 && (currentYear % 100 != 0 || currentYear % 400 == 0);
}


bool isDateValid(struct DateOfYear toValidate, unsigned year)
{
    if(toValidate.month < 1 || toValidate.month > 12 || toValidate.day < 1)
        return false;
    switch(toValidate.month)
    {
    case 4: case 6: case 9: case 11:
        if(toValidate.day <= 30)
            return true;
        break;
    case 2:
        if(does29FebruaryExist(year))
        {
            if(toValidate.day <= 29)
                return true;
        }
        else
        {
            if(toValidate.day <= 28)
                return true;
        }
        break;
    default:
        if(toValidate.day <= 31)
            return true;
    }
    return false;
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
        if(does29FebruaryExist(now.currentYear))
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


struct YearTimestamp addMinutes(struct YearTimestamp now, unsigned delay)
{
    struct YearTimestamp result = now;
    result.timestamp.time.minute += delay;
    result.timestamp.time.hour += result.timestamp.time.minute / 60;
    result.timestamp.time.minute %= 60;
    while(result.timestamp.time.hour > 23)
    {
        result.timestamp.time.hour -= 24;
        result.timestamp.date = getNextDay(result);
        if(result.timestamp.date.day == 1 && result.timestamp.date.month == 1)
            result.currentYear += 1;
    }
    return result;
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