#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "actions.h"
#include "logging.h"
#include "sizedstring.h"


static ReturnCode playSoundFile(char *fileName, unsigned repeats)
{
    LOG_LINE(LOG_INFO, "Notifying with sound file %s and %d repeats.", fileName, repeats);
    for(unsigned i = 0; i < repeats; i++)
    {
        if(!PlaySound(fileName, NULL, SND_FILENAME | SND_NODEFAULT))
        {
            LOG_LINE(LOG_ERROR, "Failed to play notification sound: %s.", fileName);
            return RET_ERROR;
        }
    }
    return RET_SUCCESS;
}


static ReturnCode actionShutdown(unsigned delay)
{
    LOG_LINE(LOG_INFO, "Shutting down the machine.");
    char command[] = "C:\\WINDOWS\\System32\\shutdown /s /t %d";
    char commandBuffer[sizeof(command) + 26];
    sprintf(commandBuffer, command, delay);
    if(system(commandBuffer) == 0)
        return RET_SUCCESS;
    else
    {
        LOG_LINE(LOG_ERROR, "Failed to shut down the machine.");
        return RET_ERROR;
    }
}


static ReturnCode actionNotify(int repeats, char *actionArgument)
{
    if(actionArgument[0] != '\0')
    {
        unsigned argumentLength = strlen(actionArgument);
        char fileName[argumentLength + sizeof(ASSET_DIRECTORY)];
        strcpy(fileName, ASSET_DIRECTORY);
        strcat(fileName, actionArgument);
        ENSURE(playSoundFile(fileName, repeats));
    }
    else
    {
        ENSURE(playSoundFile(DEFAULT_NOTIFY_SOUND, DEFAULT_NOTIFY_SOUND_REPEATS));
    }
    return RET_SUCCESS;
}


ReturnCode doAction(struct Action *action)
{
    switch(action->type)
    {
    case SHUTDOWN:
        ENSURE(actionShutdown(action->args.shutdown.delay));
        break;
    case NOTIFY:
        ENSURE(actionNotify(action->args.notify.repeats, action->args.notify.fileName));
        break;
    case RESET:
        LOG_LINE(LOG_INFO, "Resetting konc4d");
        return RET_FAILURE;
    default:
        LOG_LINE(LOG_WARNING, "Unknown action: %d", action->type);
    }
    return RET_SUCCESS;
}


static ReturnCode createNewNode(struct Action *action, struct ActionQueue **toWrite)
{
    *toWrite = malloc(sizeof(struct ActionQueue));
    if(*toWrite == NULL)
    {
        LOG_LINE(LOG_ERROR, "Failed to allocate memory for a new ActionQueue node");
        return RET_ERROR;
    }
    AQ_FIRST(*toWrite) = *action;
    return RET_SUCCESS;
}


static ReturnCode checkActionAtHead(struct ActionQueue **head, struct ActionQueue *newNode, struct Timestamp now)
{
    if(!(*head))
    {
        *head = newNode;
        newNode->next = NULL;
        return RET_SUCCESS;
    }

    if(compareTimestamp(newNode->action.timestamp, AQ_FIRST(*head).timestamp, now) < 0)
    {
        newNode->next = *head;
        *head = newNode;
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}


static struct ActionQueue* findPrevious(struct ActionQueue **head, struct Action *action, struct Timestamp now)
{
    struct ActionQueue *previous = *head, *current = (*head)->next;
    while(current != NULL)
    {
        if(compareTimestamp(action->timestamp, current->action.timestamp, now) < 0)
            return previous;
        previous = current;
        current = current->next;
    }
    return previous;
}


ReturnCode addAction(struct ActionQueue **head, struct Action *action, struct Timestamp now)
{
    struct ActionQueue *newNode;
    ENSURE(createNewNode(action, &newNode));
    TRY_END(checkActionAtHead(head, newNode, now));

    struct ActionQueue *previous = findPrevious(head, action, now);
    newNode->next = previous->next;
    previous->next = newNode;

    return RET_SUCCESS;
}


ReturnCode popAction(struct ActionQueue **head, struct Action *toWrite)
{
    if(*head == NULL)
    {
        LOG_LINE(LOG_ERROR, "Attempted access to empty ActionQueue");
        return RET_ERROR;
    }
    struct ActionQueue *oldHead = *head;
    if(toWrite != NULL)
        *toWrite = AQ_FIRST(*head);
    *head = (*head)->next;
    free(oldHead);
    return RET_SUCCESS;
}


static ReturnCode getActionDate(char **string, struct Action *toWrite, struct YearTimestamp now)
{
    unsigned day, month;
    int size;
    int itemsRead = sscanf(*string, "%u.%u%n", &day, &month, &size);
    if(itemsRead < 2)
    {
        toWrite->timestamp.date = (struct DateOfYear) {0, 0};
        return RET_FAILURE;
    }
    *string += size;
    skipWhitespace(string);
    LOG_LINE(LOG_DEBUG, "Determined action date to be %02u.%02u", day, month);

    unsigned actionYear = now.currentYear;
    if(basicCompareDate(now.timestamp.date, (struct DateOfYear) {day, month}) >= 0)
        actionYear += 1;

    if(!isDateValid((struct DateOfYear) {day, month}, actionYear))
    {
        if(day != 29 || month != 2)
        {
            LOG_LINE(LOG_ERROR, "Invalid action date: %u.%u", day, month);
            return RET_ERROR;
        }
        LOG_LINE(LOG_WARNING, "Skipping action, because 29.02 does not exist this year");
        toWrite->timestamp.date = (struct DateOfYear) {day, month};
        return RET_FAILURE;
    }

    toWrite->timestamp.date.day = day;
    toWrite->timestamp.date.month = month;
    return RET_SUCCESS;
}


static ReturnCode getActionTime(char **string, struct Action *toWrite)
{
    unsigned hour, minute;
    int size;
    int itemsRead = sscanf(*string, "%u:%u%n", &hour, &minute, &size);
    if(itemsRead < 2 || hour > 23 || minute > 59)
    {
        LOG_LINE(LOG_ERROR, "Failed to parse time in settings line: %s", *string);
        return RET_ERROR;
    }
    *string += size;
    skipWhitespace(string);
    LOG_LINE(LOG_DEBUG, "Determined action time to be %02u:%02u", hour, minute);

    toWrite->timestamp.time.hour = hour;
    toWrite->timestamp.time.minute = minute;
    return RET_SUCCESS;
}


void writeCurrentDate(struct Action *toWrite, struct YearTimestamp time)
{
    if(basicCompareTime(toWrite->timestamp.time, time.timestamp.time) <= 0)
        toWrite->timestamp.date = getNextDay(time);
    else
        toWrite->timestamp.date = time.timestamp.date;
}


#define checkActionType(checkedType)                                  \
    if(memicmp(#checkedType, *string, sizeof(#checkedType) - 1) == 0) \
    {                                                                 \
        toWrite->type = (checkedType);                                \
        *string += sizeof(#checkedType) - 1;                          \
        skipWhitespace(string);                                       \
        return RET_SUCCESS;                                           \
    }


static ReturnCode getActionType(char **string, struct Action *toWrite)
{
    checkActionType(SHUTDOWN);
    checkActionType(NOTIFY);
    checkActionType(RESET);
    LOG_LINE(LOG_ERROR, "Unknown action type to parse: %s", *string);
    return RET_ERROR;
}


static ReturnCode getShutdownActionArgs(char **string, struct Action *toWrite)
{
    unsigned delay;
    int size;
    int itemsRead = sscanf(*string, "%u%n", &delay, &size);
    if(itemsRead < 1)
        delay = DEFAULT_SHUTDOWN_DELAY;
    else
        *string += size;
    LOG_LINE(LOG_DEBUG, "Determined shutdown action delay to be %u seconds", delay);

    toWrite->args.shutdown.delay = delay;
    return RET_SUCCESS;
}


static ReturnCode getFileName(char **string, struct SizedString *toWrite)
{
    bool endAtQuote = false, properlyEnded = false, escaped = false;
    if(**string == '\"')
    {
        endAtQuote = true;
        ++(*string);
    }

    while(**string != '\0')
    {
        char analyzed = *((*string)++);
        if(escaped)
        {
            ENSURE(appendToSizedString(toWrite, analyzed));
            escaped = false;
            continue;
        }
        if(analyzed == '\\')
        {
            escaped = true;
            continue;
        }
        if(endAtQuote)
        {
            if(analyzed == '\"')
            {
                properlyEnded = true;
                break;
            }
        }
        else if(isspace(analyzed))
            break;

        ENSURE(appendToSizedString(toWrite, analyzed));
    }

    if(endAtQuote && !properlyEnded)
    {
        LOG_LINE(LOG_ERROR, "File name started with \" and not terminated: %s", toWrite->data);
        return RET_ERROR;
    }
    if(toWrite->size > MAX_NOTIFY_FILE_NAME_SIZE)
    {
        LOG_LINE(LOG_ERROR, "File name too long: %s", toWrite->data);
        return RET_ERROR;
    }
    ENSURE(appendToSizedString(toWrite, '\0'));
    return RET_SUCCESS;
}


static ReturnCode getNotifyActionArgs(char **string, struct Action *toWrite)
{
    struct SizedString fileName;
    ENSURE(createSizedString(&fileName));
    ENSURE_CALLBACK(getFileName(string, &fileName), freeSizedString(fileName));
    if(fileName.data[0] == '\0')
    {
        strcpy(toWrite->args.notify.fileName, DEFAULT_NOTIFY_SOUND);
        toWrite->args.notify.repeats = DEFAULT_NOTIFY_SOUND_REPEATS;
        return RET_SUCCESS;
    }
    strcpy(toWrite->args.notify.fileName, fileName.data);
    freeSizedString(fileName);

    unsigned repeats;
    int size;
    int itemsRead = sscanf(*string, "%u%n", &repeats, &size);
    if(itemsRead < 1)
        repeats = 1;
    else
        *string += size;
    LOG_LINE(LOG_DEBUG, "Determined notify action file to be %s with %u repeats", toWrite->args.notify.fileName, repeats);

    toWrite->args.notify.repeats = repeats;
    return RET_SUCCESS;
}


static ReturnCode getActionArguments(char **string, struct Action *toWrite)
{
    switch(toWrite->type)
    {
    case SHUTDOWN:
        ENSURE(getShutdownActionArgs(string, toWrite));
        break;
    case NOTIFY:
        ENSURE(getNotifyActionArgs(string, toWrite));
        break;
    case RESET:
        ; // do nothing
    }
    skipWhitespace(string);
    return RET_SUCCESS;
}


// [date] minute shutdown delay
// [date] minute notify filename repeats
// [date] minute reset
ReturnCode parseAction(char *string, struct Action *toWrite, struct YearTimestamp now)
{
    LOG_LINE(LOG_DEBUG, "Parsing action from string %s", string);
    char *currentString = string;
    skipWhitespace(&currentString);

    ReturnCode dateParsing;
    RETHROW(dateParsing = getActionDate(&currentString, toWrite, now));
    ENSURE(getActionTime(&currentString, toWrite));
    if(dateParsing == RET_FAILURE)
    {
        if(toWrite->timestamp.date.day == 29 && toWrite->timestamp.date.month == 2)
            return RET_FAILURE;
        toWrite->repeated = true;
        writeCurrentDate(toWrite, now);
    }
    else
        toWrite->repeated = false;
    ENSURE(getActionType(&currentString, toWrite));
    ENSURE(getActionArguments(&currentString, toWrite));
    if(*currentString != '\0')
        LOG_LINE(LOG_WARNING, "Settings line contains extra tokens: %s", string);
    return RET_SUCCESS;
}


ReturnCode popActionWithRepeat(struct ActionQueue **head, struct Action *toWrite, struct YearTimestamp now)
{
    struct Action popped;
    ENSURE(popAction(head, &popped));
    if(toWrite != NULL)
        *toWrite = popped;
    if(popped.repeated)
    {
        int actionYear = now.currentYear;
        if(basicCompareTimestamp(now.timestamp, popped.timestamp) >= 0)
            actionYear += 1;
        popped.timestamp.date = getNextDay((struct YearTimestamp) {popped.timestamp, actionYear});
        ENSURE(addAction(head, &popped, now.timestamp));
    }
    return RET_SUCCESS;
}


ReturnCode skipUntilTimestamp(struct ActionQueue **head, struct Timestamp time, struct YearTimestamp now)
{
    while(compareTimestamp(AQ_FIRST(*head).timestamp, time, now.timestamp) <= 0)
        ENSURE(popActionWithRepeat(head, NULL, now));
    return RET_SUCCESS;
}


void destroyActionQueue(struct ActionQueue **head)
{
    while(*head != NULL)
        if(popAction(head, NULL) != RET_SUCCESS)
            LOG_LINE(LOG_ERROR, "popAction failed - should be impossible to reach");
}
