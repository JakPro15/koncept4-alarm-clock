#ifndef KONC4D_ACTIONS_H
#define KONC4D_ACTIONS_H

#include "error_handling.h"
#include "timestamps.h"

#define ASSET_DIRECTORY "asset\\"
#define DEFAULT_NOTIFY_SOUND "asset\\ring.wav"
#define DEFAULT_NOTIFY_SOUND_REPEATS 5
#define MAX_NOTIFY_FILE_NAME_SIZE 30


enum ActionType
{
    SHUTDOWN,
    NOTIFY,
    RESET
};


struct Action
{
    struct Timestamp timestamp;
    enum ActionType type;
    union
    {
        struct {unsigned repeats; char fileName[MAX_NOTIFY_FILE_NAME_SIZE + 1];} notify;
        struct {unsigned delay;} shutdown;
    } args;
    bool repeated;
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


ReturnCode addAction(struct ActionQueue **head, struct Action *action, struct Timestamp now);
ReturnCode popAction(struct ActionQueue **head, struct Action *toWrite);
ReturnCode parseAction(char *string, struct Action *toWrite, struct YearTimestamp now);
ReturnCode popActionWithRepeat(struct ActionQueue **head, struct Action *toWrite, struct YearTimestamp now);
ReturnCode skipUntilTimestamp(struct ActionQueue **head, struct Timestamp time, struct YearTimestamp now);

#endif
