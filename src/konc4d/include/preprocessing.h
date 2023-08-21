#ifndef KONC4D_PREPROCESSING_H
#define KONC4D_PREPROCESSING_H

#include "sizedstring.h"
#include "actions.h"

#include <stdio.h>

struct StringPair
{
    char *key, *value;
    unsigned valueSize;
};

struct GatheredDefines
{
    unsigned size;
    struct StringPair *defines;
};

ReturnCode verifyDefineName(const char *name, unsigned *size) NO_IGNORE;
ReturnCode gatherDefines(FILE *settingsFile, struct GatheredDefines *toWrite) NO_IGNORE;
ReturnCode fitDefine(const char *settingsLine, unsigned size, struct ActionQueue **actions,
                     struct GatheredDefines defines, struct YearTimestamp now) NO_IGNORE;
void freeGatheredDefines(struct GatheredDefines defines);

#endif
