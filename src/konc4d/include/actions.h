#ifndef KONC4D_ACTIONS_H
#define KONC4D_ACTIONS_H

#include "error_handling.h"
#include "passed_action.h"

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
    unsigned repeatPeriod;
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


ReturnCode addAction(struct ActionQueue **head, struct Action *action, struct Timestamp now) NO_IGNORE;
ReturnCode popAction(struct ActionQueue **head, struct Action *toWrite) NO_IGNORE;
ReturnCode parseAction(char *string, struct Action *toWrite, struct YearTimestamp now) NO_IGNORE;
ReturnCode popActionWithRepeat(struct ActionQueue **head, struct Action *toWrite, struct YearTimestamp now) NO_IGNORE;
ReturnCode skipUntilTimestamp(struct ActionQueue **head, struct Timestamp time, struct YearTimestamp now) NO_IGNORE;
void destroyActionQueue(struct ActionQueue **head);

inline struct PassedAction getPassedAction(struct Action *action)
{
    return (struct PassedAction) {action->timestamp, action->type, action->repeatPeriod};
}

#endif
