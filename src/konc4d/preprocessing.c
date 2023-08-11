#include "preprocessing.h"

#include <stdlib.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
ReturnCode gatherDefines(FILE *settingsFile, struct GatheredDefines *toWrite)
{
    return RET_SUCCESS;
}


ReturnCode fitDefine(struct SizedString settingsLine, struct ActionQueue **actions, struct GatheredDefines defines)
{
    return RET_SUCCESS;
}
#pragma GCC diagnostic pop


void freeGatheredDefines(struct GatheredDefines defines)
{
    free(defines.defines);
}
