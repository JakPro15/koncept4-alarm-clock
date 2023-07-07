#ifndef KONC4D_TIMESTAMPS_H
#define KONC4D_TIMESTAMPS_H

#include <stdbool.h>


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
struct DateOfYear getNextDay(struct YearTimestamp time);
struct YearTimestamp addMinutes(struct YearTimestamp now, unsigned delay);


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
 * Considers times before current time and after midnight to be tomorrow's. */
int basicCompareTimestamp(const struct Timestamp first, const struct Timestamp second);


/* Returns 1 if first is later, -1 if first is earlier, 0 if they're equal.
 * Considers times before current time and after midnight to be tomorrow's. */
int compareTimestamp(const struct Timestamp first, const struct Timestamp second, const struct Timestamp now);

#endif
