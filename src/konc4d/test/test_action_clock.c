#include "action_clock.h"
#include "testing.h"

#define TOD(h, m) (struct TimeOfDay){h, m}

static ReturnCode testAll0s(void)
{
    struct ActionClock clock = {0};
    ASSERT(checkActionsInPeriod(&clock, TOD(0, 0), TOD(23, 59), 0));
    return RET_SUCCESS;
}

static ReturnCode testSettingInMiddle(void)
{
    struct ActionClock clock = {0};
    setActionClock(&clock, (struct TimeOfDay){4, 30}, (struct TimeOfDay){15, 15}, 1);
    ASSERT(checkActionsInPeriod(&clock, TOD(0, 0), TOD(4, 29), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(4, 30), TOD(15, 15), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(15, 16), TOD(23, 59), 0));

    setActionClock(&clock, (struct TimeOfDay){5, 0}, (struct TimeOfDay){15, 0}, 0);
    ASSERT(checkActionsInPeriod(&clock, TOD(0, 0), TOD(4, 29), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(4, 30), TOD(4, 59), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(5, 0), TOD(15, 0), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(15, 1), TOD(15, 15), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(15, 16), TOD(23, 59), 0));
    return RET_SUCCESS;
}

static ReturnCode testSettingOverMidnight(void)
{
    struct ActionClock clock = {0};
    setActionClock(&clock, (struct TimeOfDay){15, 15}, (struct TimeOfDay){4, 30}, 1);
    ASSERT(checkActionsInPeriod(&clock, TOD(0, 0), TOD(4, 30), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(4, 31), TOD(15, 14), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(15, 15), TOD(23, 59), 1));

    setActionClock(&clock, (struct TimeOfDay){16, 0}, (struct TimeOfDay){4, 0}, 0);
    ASSERT(checkActionsInPeriod(&clock, TOD(0, 0), TOD(4, 0), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(4, 1), TOD(4, 30), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(4, 31), TOD(15, 14), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(15, 15), TOD(15, 59), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(16, 0), TOD(23, 59), 0));
    return RET_SUCCESS;
}

static ReturnCode testSettingAtEdges(void)
{
    struct ActionClock clock = {0};
    setActionClock(&clock, (struct TimeOfDay){15, 15}, (struct TimeOfDay){4, 30}, 1);
    setActionClock(&clock, (struct TimeOfDay){15, 0}, (struct TimeOfDay){16, 0}, 0);
    setActionClock(&clock, (struct TimeOfDay){4, 0}, (struct TimeOfDay){5, 0}, 1);
    ASSERT(checkActionsInPeriod(&clock, TOD(0, 0), TOD(5, 0), 1));
    ASSERT(checkActionsInPeriod(&clock, TOD(5, 1), TOD(16, 0), 0));
    ASSERT(checkActionsInPeriod(&clock, TOD(16, 1), TOD(23, 59), 1));
    return RET_SUCCESS;
}

PREPARE_TESTING(action_clock,
    testAll0s,
    testSettingInMiddle,
    testSettingOverMidnight,
    testSettingAtEdges
)
