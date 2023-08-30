#include "test_konc4d.h"
#include "actions.h"


#define ASSERT_EQUAL(first, second) \
    ASSERT(basicCompareTimestamp((first).timestamp, (second).timestamp) == 0); \
    ASSERT((first).type == (second).type); \
    ASSERT((first).args.shutdown.delay == (second).args.shutdown.delay); \
    ASSERT((first).repeatPeriod == (second).repeatPeriod)


static ReturnCode testAddActionRegular(void)
{
    struct ActionQueue *head = NULL;
    struct Action action = {{{2, 1}, {0, 0}}, SHUTDOWN, .args.shutdown = {1}};
    struct Action action2 = {{{2, 1}, {10, 0}}, SHUTDOWN, .args.shutdown = {2}};

    ASSERT_ENSURE(addAction(&head, &action, (struct Timestamp) {{3, 1}, {23, 0}}));
    ASSERT_EQUAL(action, AQ_FIRST(head));
    ASSERT(head->next == NULL);

    ASSERT_ENSURE(addAction(&head, &action2, (struct Timestamp) {{3, 1}, {23, 0}}));
    ASSERT_EQUAL(action, AQ_FIRST(head));
    ASSERT(head->next != NULL);
    ASSERT_EQUAL(action2, AQ_SECOND(head));
    ASSERT(head->next->next == NULL);

    action.timestamp.time.hour = 12;
    ASSERT(AQ_FIRST(head).timestamp.time.hour == 0);

    return RET_SUCCESS;
}


static ReturnCode testAddActionAtEnd(void)
{
    struct ActionQueue second = {NULL, {{{5, 12}, {10, 0}}, SHUTDOWN, .args.shutdown = {1}}};
    struct ActionQueue first = {&second, {{{2, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {2}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{2, 1}, {12, 30}}, SHUTDOWN, .args.shutdown = {3}};
    struct Action toAdd2 = {{{2, 1}, {21, 12}}, SHUTDOWN, .args.shutdown = {-1}};

    ASSERT_ENSURE(addAction(&head, &toAdd, (struct Timestamp) {{14, 2}, {23, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next != NULL);
    ASSERT(second.next->next == NULL);
    ASSERT_EQUAL(second.next->action, toAdd);

    ASSERT_ENSURE(addAction(&head, &toAdd2, (struct Timestamp) {{14, 2}, {23, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next->next != NULL);
    ASSERT(second.next->next->next == NULL);
    ASSERT_EQUAL(second.next->next->action, toAdd2);

    return RET_SUCCESS;
}


static ReturnCode testAddActionInTheMiddle(void)
{
    struct ActionQueue second = {NULL, {{{16, 6}, {5, 0}}, SHUTDOWN, .args.shutdown = {1}}};
    struct ActionQueue first = {&second, {{{15, 6}, {20, 0}}, SHUTDOWN, .args.shutdown = {2}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{16, 6}, {1, 30}}, SHUTDOWN, .args.shutdown = {5}};
    struct Action toAdd2 = {{{16, 6}, {3, 12}}, SHUTDOWN, .args.shutdown = {-1}};

    ASSERT_ENSURE(addAction(&head, &toAdd, (struct Timestamp) {{15, 6}, {10, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next == NULL);
    ASSERT(first.next->next == &second);
    ASSERT_EQUAL(first.next->action, toAdd);

    ASSERT_ENSURE(addAction(&head, &toAdd2, (struct Timestamp) {{15, 6}, {10, 0}}));
    ASSERT(head == &first);
    ASSERT(second.next == NULL);
    ASSERT(first.next->next->next == &second);
    ASSERT_EQUAL(first.next->next->action, toAdd2);

    return RET_SUCCESS;
}


static ReturnCode testAddActionAtHead(void)
{
    struct ActionQueue first = {NULL, {{{2, 1}, {11, 0}}, SHUTDOWN, .args.shutdown = {3}}};
    struct ActionQueue *head = &first;
    struct Action toAdd = {{{2, 1}, {6, 30}}, SHUTDOWN, .args.shutdown = {4}};

    ASSERT_ENSURE(addAction(&head, &toAdd, (struct Timestamp) {{1, 1}, {5, 0}}));
    ASSERT(head->next == &first);
    ASSERT(first.next == NULL);
    ASSERT_EQUAL(AQ_FIRST(head), toAdd);

    return RET_SUCCESS;
}


static ReturnCode testPopAction(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, first = {{{6, 3}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}};
    struct Action second = {{{6, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {5}};
    ASSERT_ENSURE(addAction(&head, &first, (struct Timestamp) {{6, 3}, {5, 0}}));
    ASSERT_ENSURE(addAction(&head, &second, (struct Timestamp) {{6, 3}, {5, 0}}));

    ASSERT_ENSURE(popAction(&head, &toWrite));
    ASSERT_EQUAL(toWrite, first);
    ASSERT_ENSURE(popAction(&head, &toWrite));
    ASSERT_EQUAL(toWrite, second);
    ASSERT(head == NULL);

    return RET_SUCCESS;
}


static ReturnCode testPopActionEmpty(void)
{
    struct ActionQueue *head = NULL;
    ASSERT_THROW(popAction(&head, NULL));
    return RET_SUCCESS;
}


static ReturnCode testPopActionWithRepeat(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, first = {{{6, 3}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}, .repeatPeriod = MINUTES_IN_DAY};
    struct Action second = {{{6, 3}, {12, 0}}, SHUTDOWN, .args.shutdown = {5}, .repeatPeriod = false};
    struct YearTimestamp now = {{{6, 3}, {5, 0}}, 2015};
    ASSERT_ENSURE(addAction(&head, &first, now.timestamp));
    ASSERT_ENSURE(addAction(&head, &second, now.timestamp));

    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, first);
    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, second);

    for(int i = 0; i < 5; i++)
    {
        ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
        ASSERT(toWrite.type == SHUTDOWN);
        ASSERT(basicCompareTimestamp(toWrite.timestamp, (struct Timestamp) {{7 + i, 3}, {11, 0}}) == 0);
        ASSERT(toWrite.args.shutdown.delay == 1);
        ASSERT(toWrite.repeatPeriod == MINUTES_IN_DAY);
    }

    return RET_SUCCESS;
}


static ReturnCode testPopActionWithRepeatMonthly(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, first = {{{6, 9}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}, .repeatPeriod = MONTHLY_REPEAT};
    struct Action second = {{{6, 9}, {12, 0}}, SHUTDOWN, .args.shutdown = {5}, .repeatPeriod = false};
    struct YearTimestamp now = {{{6, 9}, {5, 0}}, 2015};
    ASSERT_ENSURE(addAction(&head, &first, now.timestamp));
    ASSERT_ENSURE(addAction(&head, &second, now.timestamp));

    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, first);
    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, second);

    for(int i = 0; i < 5; i++)
    {
        ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
        ASSERT(toWrite.type == SHUTDOWN);
        ASSERT(basicCompareTimestamp(toWrite.timestamp, (struct Timestamp) {{6, (9 + i) % 12 + 1}, {11, 0}}) == 0);
        ASSERT(toWrite.args.shutdown.delay == 1);
        ASSERT(toWrite.repeatPeriod == MONTHLY_REPEAT);
    }

    return RET_SUCCESS;
}


static ReturnCode testPopActionWithRepeatMonthlyCorrection31(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, action = {{{31, 8}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}, .repeatPeriod = MONTHLY_REPEAT};
    struct YearTimestamp now = {{{6, 8}, {5, 0}}, 2015};
    ASSERT_ENSURE(addAction(&head, &action, now.timestamp));

    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, action);

    for(int i = 0; i < 5; i++)
    {
        ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
        ASSERT(toWrite.type == SHUTDOWN);
        ASSERT(basicCompareTimestamp(toWrite.timestamp, (struct Timestamp) {
            {getMonthLength(toWrite.timestamp.date.month, deduceYear(toWrite.timestamp, now).currentYear), (8 + i) % 12 + 1}, {11, 0}
        }) == 0);
        ASSERT(toWrite.args.shutdown.delay == 1);
        ASSERT(toWrite.repeatPeriod == MONTHLY_REPEAT);
    }

    return RET_SUCCESS;
}


static ReturnCode testPopActionWithRepeatMonthlyCorrection30(void)
{
    struct ActionQueue *head = NULL;
    struct Action toWrite, action = {{{30, 8}, {11, 0}}, SHUTDOWN, .args.shutdown = {1}, .repeatPeriod = MONTHLY_REPEAT};
    struct YearTimestamp now = {{{6, 8}, {5, 0}}, 2015};
    ASSERT_ENSURE(addAction(&head, &action, now.timestamp));

    ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
    ASSERT_EQUAL(toWrite, action);

    for(int i = 0; i < 10; i++)
    {
        ASSERT_ENSURE(popActionWithRepeat(&head, &toWrite, now));
        ASSERT(toWrite.type == SHUTDOWN);
        unsigned monthLength = getMonthLength(toWrite.timestamp.date.month, deduceYear(toWrite.timestamp, now).currentYear);
        ASSERT(basicCompareTimestamp(toWrite.timestamp, (struct Timestamp) {
            {monthLength > 30 ? 30 : monthLength, (8 + i) % 12 + 1}, {11, 0}
        }) == 0);
        ASSERT(toWrite.args.shutdown.delay == 1);
        ASSERT(toWrite.repeatPeriod == MONTHLY_REPEAT);
    }

    return RET_SUCCESS;
}


static ReturnCode testSkipUntilTimestamp(void)
{
    struct YearTimestamp now = {{{4, 7}, {14, 16}}, 2023};
    struct YearTimestamp until = {{{4, 7}, {14, 21}}, 2023};
    struct Action actions[] = {
        {{{4, 7}, {14, 20}}, RESET, {{0}}, false},
        {{{4, 7}, {14, 21}}, RESET, {{0}}, MINUTES_IN_DAY},
        {{{4, 7}, {14, 22}}, RESET, {{0}}, false},
        {{{3, 7}, {14, 15}}, RESET, {{0}}, false},
        {{{5, 7}, {14, 15}}, RESET, {{0}}, MINUTES_IN_DAY}
    };
    struct ActionQueue *head = NULL;
    for(int i = 0; i < 5; i++)
        ASSERT_ENSURE(addAction(&head, &actions[i], now.timestamp));
    ASSERT_ENSURE(skipUntilTimestamp(&head, until.timestamp, now));

    ASSERT(AQ_FIRST(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_FIRST(head).timestamp, actions[2].timestamp) == 0);
    ASSERT(AQ_FIRST(head).repeatPeriod == false);

    ASSERT(AQ_SECOND(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_SECOND(head).timestamp, actions[4].timestamp) == 0);
    ASSERT(AQ_SECOND(head).repeatPeriod == MINUTES_IN_DAY);

    ASSERT(AQ_THIRD(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_THIRD(head).timestamp, (struct Timestamp) {{5, 7}, {14, 21}}) == 0);
    ASSERT(AQ_THIRD(head).repeatPeriod == MINUTES_IN_DAY);

    ASSERT(AQ_FOURTH(head).type == RESET);
    ASSERT(basicCompareTimestamp(AQ_FOURTH(head).timestamp, actions[3].timestamp) == 0);
    ASSERT(AQ_FOURTH(head).repeatPeriod == false);

    ASSERT(head->next->next->next->next == NULL);
    return RET_SUCCESS;
}


PREPARE_TESTING(actions,
    testAddActionRegular,
    testAddActionAtEnd,
    testAddActionInTheMiddle,
    testAddActionAtHead,
    testPopAction,
    testPopActionEmpty,
    testPopActionWithRepeat,
    testPopActionWithRepeatMonthly,
    testPopActionWithRepeatMonthlyCorrection31,
    testPopActionWithRepeatMonthlyCorrection30,
    testSkipUntilTimestamp
)
