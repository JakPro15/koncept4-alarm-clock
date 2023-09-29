#ifndef KONC4D_ACTIONS_H
#define KONC4D_ACTIONS_H

#include "error_handling.h"
#include "passed_action.h"
#include "action_clock.h"

#define ASSET_DIRECTORY "asset\\"
#define DEFAULT_NOTIFY_SOUND "ring.wav"
#define DEFAULT_NOTIFY_SOUND_REPEATS 5
#define DEFAULT_SHUTDOWN_DELAY 30
#define MAX_NOTIFY_FILE_NAME_SIZE 30


struct Action
{
    struct Timestamp timestamp;
    enum ActionType type;
    union
    {
        struct {unsigned repeats; char fileName[MAX_NOTIFY_FILE_NAME_SIZE + 1];} notify;
        struct {unsigned delay;} shutdown;
    } args;
    int repeatPeriod;
};


ReturnCode doAction(struct Action *action);


struct ActionQueue
{
    struct ActionQueue *next;
    struct Action action;
};

#define AQ_FIRST(head) (head)->action
#define AQ_SECOND(head) (head)->next->action
#define AQ_THIRD(head) (head)->next->next->action
#define AQ_FOURTH(head) (head)->next->next->next->action
#define AQ_FIFTH(head) (head)->next->next->next->next->action
#define AQ_SIXTH(head) (head)->next->next->next->next->next->action
#define AQ_SEVENTH(head) (head)->next->next->next->next->next->next->action
#define AQ_EIGHTH(head) (head)->next->next->next->next->next->next->next->action


struct AllActions
{
    struct ActionQueue *queueHead;
    struct ActionClock shutdownClock;
};


ReturnCode addAction(struct ActionQueue **head, struct Action *action, struct Timestamp now) NO_IGNORE;
ReturnCode popAction(struct ActionQueue **head, struct Action *toWrite) NO_IGNORE;
ReturnCode getActionTimesEveryClause(char **string, unsigned *times, unsigned *every) NO_IGNORE;
ReturnCode parseAction(char *string, struct Action *toWrite, struct YearTimestamp now) NO_IGNORE;

/* Action line format:
 * [times_every_specifier] [[repeat_specifier] date] minute type [arguments]
 * times_every_specifier := N times every M
 *      action will be repeated N times every M minutes, beginning from the given minute
 * repeat_specifier := (daily|weekly|monthly|period N)
 *      period given in hours as floating-point number
 *      default: if date is given, no repeat
 *               if date not given, daily
 * date := DD.MM
 * minute := hh:mm
 * type := (shutdown|notify|reset)
 * arguments :=
 *      if type == shutdown: delay
 *          delay given in seconds, default DEFAULT_SHUTDOWN_DELAY
 *      if type == reset:
 *          no arguments permitted
 *      if type == notify: [filename [repeats]]
 *          filename specifies a file in ASSET_DIRECTORY, default is DEFAULT_NOTIFY_SOUND with DEFAULT_NOTIFY_SOUND_REPEATS
 *              filename must not be longer than MAX_NOTIFY_FILE_NAME_SIZE
 *          repeats is an integer, default is 1 if filename given, DEFAULT_NOTIFY_SOUND_REPEATS if not
 */
ReturnCode parseActionLine(char *string, struct AllActions *toWrite, struct YearTimestamp now) NO_IGNORE;

ReturnCode popActionWithRepeat(struct ActionQueue **head, struct Action *toWrite, struct YearTimestamp now) NO_IGNORE;
ReturnCode skipUntilTimestamp(struct ActionQueue **head, struct Timestamp time, struct YearTimestamp now) NO_IGNORE;
void destroyActionQueue(struct ActionQueue **head);
bool actionsEqual(const struct Action *first, const struct Action *second) NO_IGNORE;

inline struct PassedAction getPassedAction(struct Action *action)
{
    return (struct PassedAction) {action->timestamp, action->type, action->repeatPeriod};
}

#endif
