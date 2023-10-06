#include "action_clock.h"
#include <stdio.h>


static unsigned timeToIndex(struct TimeOfDay time)
{
    return time.hour * 60 + time.minute;
}


static void setBitsUntil(uint32_t *toSet, unsigned bitUntil, bool value)
{
    if(value)
        (*toSet) |= ~((uint32_t) 0) << (31 - bitUntil);
    else
    {
        if(bitUntil == 31)
            (*toSet) = 0;
        else
            (*toSet) &= ~((uint32_t) 0) >> (bitUntil + 1);
    }
}


static void setBitsFrom(uint32_t *toSet, unsigned bitFrom, bool value)
{
    if(value)
        (*toSet) |= ~((uint32_t) 0) >> bitFrom;
    else
    {
        if(bitFrom == 0)
            (*toSet) = 0;
        else
            (*toSet) &= ~((uint32_t) 0) << (31 - bitFrom + 1);
    }
    return;
}


void setActionClock(struct ActionClock *toWrite, struct TimeOfDay since, struct TimeOfDay until, bool value)
{
    unsigned start = timeToIndex(since), end = timeToIndex(until);
    if(start > end)
    {
        for(unsigned i = 0; i < end / 32; i++)
            toWrite->data[i] = value ? ~((uint32_t) 0) : 0;
        setBitsUntil(&toWrite->data[end / 32], end % 32, value);

        for(unsigned i = start / 32 + 1; i < 45; i++)
            toWrite->data[i] = value ? ~((uint32_t) 0) : 0;
        setBitsFrom(&toWrite->data[start / 32], start % 32, value);
    }
    else
    {
        if(start / 32 == end / 32)
        {
            uint32_t toSet = (~((uint32_t) 0) >> (start % 32)) & (~((uint32_t) 0) << (31 - (end % 32)));
            if(value)
                toWrite->data[start / 32] |= toSet;
            else
                toWrite->data[start / 32] &= ~toSet;
        }
        else
        {
            for(unsigned i = start / 32 + 1; i < end / 32; i++)
                toWrite->data[i] = value ? ~((uint32_t) 0) : 0;
            setBitsFrom(&toWrite->data[start / 32], start % 32, value);
            setBitsUntil(&toWrite->data[end / 32], end % 32, value);
        }
    }
}


bool checkActionAtTime(struct ActionClock *clock, struct TimeOfDay time)
{
    unsigned index = timeToIndex(time);
    return (clock->data[index / 32] >> (31 - (index % 32))) & 1;
}


bool checkActionsInPeriod(struct ActionClock *clock, struct TimeOfDay from, struct TimeOfDay until, bool value)
{
    for(struct TimeOfDay t = from; basicCompareTime(t, until) <= 0; incrementTime(&t))
        if(checkActionAtTime(clock, t) != value)
            return false;
    return true;
}
