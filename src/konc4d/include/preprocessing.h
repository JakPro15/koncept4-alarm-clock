#ifndef KONC4D_PREPROCESSING_H
#define KONC4D_PREPROCESSING_H

#include "sizedstring.h"
#include "actions.h"

#include <stdio.h>

struct StringPair
{
    struct SizedString key, value;
};

struct GatheredDefines
{
    unsigned size;
    struct StringPair *defines;
};

ReturnCode gatherDefines(FILE *settingsFile, struct GatheredDefines *toWrite) NO_IGNORE;
ReturnCode fitDefine(struct SizedString settingsLine, struct ActionQueue **actions, struct GatheredDefines defines) NO_IGNORE;
void freeGatheredDefines(struct GatheredDefines defines);

#endif
