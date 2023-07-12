#include "stringvector.h"
#include "logging.h"

#include <stdlib.h>


ReturnCode createStringVector(struct StringVector *vector)
{
    vector->size = 0;
    vector->capacity = STARTING_CAPACITY;
    vector->data = malloc(vector->capacity * sizeof(char*));
    if(vector->data == NULL)
    {
        LOG_LINE(LOG_ERROR, "Memory allocation error");
        return RET_ERROR;
    }
    return RET_SUCCESS;
}


ReturnCode appendToStringVector(struct StringVector *vector, char *string)
{
    if(vector->size >= vector->capacity)
    {
        vector->capacity += CAPACITY_INCREMENT;
        vector->data = realloc(vector->data, vector->capacity * sizeof(char*));
        if(vector->data == NULL)
        {
            LOG_LINE(LOG_ERROR, "Memory allocation error");
            return RET_ERROR;
        }
    }
    vector->data[vector->size++] = string;
    return RET_SUCCESS;
}


void fullyFreeStringVector(struct StringVector *vector)
{
    for(unsigned i = 0; i < vector->size; i++)
        free(vector->data[i]);
    free(vector->data);
}
