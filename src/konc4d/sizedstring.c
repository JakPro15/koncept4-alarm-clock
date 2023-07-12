#include <stdlib.h>
#include "logging.h"
#include "sizedstring.h"


ReturnCode createSizedString(struct SizedString *string)
{
    string->size = 0;
    string->capacity = STARTING_CAPACITY;
    string->data = malloc(string->capacity * sizeof(char));
    if(string->data == NULL)
    {
        LOG_LINE(LOG_ERROR, "Memory allocation error");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode appendToSizedString(struct SizedString *string, char character)
{
    if(string->size >= string->capacity)
    {
        string->capacity += CAPACITY_INCREMENT;
        string->data = realloc(string->data, string->capacity * sizeof(char));
        if(string->data == NULL)
        {
            LOG_LINE(LOG_ERROR, "Memory allocation error");
            return RET_ERROR;
        }
    }
    string->data[string->size++] = character;
    return RET_SUCCESS;
}


void freeSizedString(struct SizedString string)
{
    free(string.data);
}
