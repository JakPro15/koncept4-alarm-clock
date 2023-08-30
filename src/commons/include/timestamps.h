#ifndef KONC4D_TIMESTAMPS_H
#define KONC4D_TIMESTAMPS_H

#include <stdbool.h>

#define MINUTES_IN_HOUR 60
#define MINUTES_IN_DAY 24 * 60
#define MINUTES_IN_WEEK 7 * 24 * 60


struct DateOfYear
{
    unsigned day, month;
};


struct TimeOfDay
{
    unsigned hour, minute;
};


struct Timestamp
{
    struct DateOfYear date;
    struct TimeOfDay time;
};


struct YearTimestamp
{
    struct Timestamp timestamp;
    unsigned currentYear;
};


struct YearTimestamp getCurrentTimestamp(void);
bool isDateValid(struct DateOfYear toValidate, unsigned year);
unsigned getMonthLength(unsigned month, unsigned year);
struct DateOfYear getNextDay(struct YearTimestamp time);
struct YearTimestamp addMinutes(struct YearTimestamp now, unsigned delay);

/* Returns difference in minutes: second - first */
unsigned difference(struct YearTimestamp earlier, struct YearTimestamp later);
struct YearTimestamp deduceYear(struct Timestamp toDeduce, struct YearTimestamp now);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal,
 * without respect of the current time */
int basicCompareTime(const struct TimeOfDay first, const struct TimeOfDay second);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal.
 * Considers times before current time and after midnight to be tomorrow's. */
int compareTime(const struct TimeOfDay first, const struct TimeOfDay second, const struct TimeOfDay now);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal,
 * without respect of the current time */
int basicCompareDate(const struct DateOfYear first, const struct DateOfYear second);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal.
 * without respect of the current time */
int basicCompareTimestamp(const struct Timestamp first, const struct Timestamp second);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal.
 * Considers times before current time and after midnight to be tomorrow's. */
int compareTimestamp(const struct Timestamp first, const struct Timestamp second, const struct Timestamp now);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal. */
int compareYearTimestamp(const struct YearTimestamp first, const struct YearTimestamp second);

#endif
