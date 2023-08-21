#include "logging.h"
#include "sizedstring.h"

#include <stdlib.h>
#include <ctype.h>


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


ReturnCode increaseSizedStringCapacity(struct SizedString *string)
{
    string->capacity += CAPACITY_INCREMENT;
    string->data = realloc(string->data, string->capacity);
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
        ENSURE(increaseSizedStringCapacity(string));
    string->data[string->size++] = character;
    return RET_SUCCESS;
}


void freeSizedString(struct SizedString string)
{
    free(string.data);
}


void skipWhitespace(char **string)
{
    while(isspace(**string))
        ++(*string);
}
