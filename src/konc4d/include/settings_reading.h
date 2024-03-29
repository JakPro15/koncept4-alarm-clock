#ifndef KONC4D_SETTINGS_READING_H
#define KONC4D_SETTINGS_READING_H

#include "actions.h"
#include "sizedstring.h"

#include <windows.h>
#include <stdio.h>

#define SETTINGS_FILE_NAME "asset\\settings.txt"


ReturnCode getLine(FILE *file, struct SizedString *toWrite) NO_IGNORE;
ReturnCode skipPreprocessingDirectives(FILE *settingsFile) NO_IGNORE;
ReturnCode loadActionsFromFile(struct AllActions *toWrite, char *fileName, struct YearTimestamp now) NO_IGNORE;

inline ReturnCode loadActions(struct AllActions *toWrite)
{
    return loadActionsFromFile(toWrite, SETTINGS_FILE_NAME, getCurrentTimestamp());
}

#endif
