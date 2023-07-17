#ifndef KONC4D_SETTINGS_READING_H
#define KONC4D_SETTINGS_READING_H

#include <windows.h>
#include "actions.h"
#include "sizedstring.h"

#define SETTINGS_FILE_NAME "asset\\settings.txt"


struct BufferedFile
{
    HANDLE handle;
    char buffer[256];
    unsigned buffIndex, buffEnd;
};


ReturnCode openBufferedFile(struct BufferedFile *toWrite, char *fileName) NO_IGNORE;
ReturnCode closeBufferedFile(struct BufferedFile *file);
ReturnCode getCharacter(struct BufferedFile *file, char *toWrite) NO_IGNORE;
ReturnCode getLine(struct BufferedFile *file, struct SizedString *toWrite) NO_IGNORE;
ReturnCode getNextAction(struct BufferedFile *settingsFile, struct Action *toWrite, struct SizedString *buffer, struct YearTimestamp now) NO_IGNORE;
ReturnCode loadActionsFromFile(struct ActionQueue **toWrite, char *fileName, struct YearTimestamp now) NO_IGNORE;

inline ReturnCode loadActions(struct ActionQueue **toWrite)
{
    return loadActionsFromFile(toWrite, SETTINGS_FILE_NAME, getCurrentTimestamp());
}

#endif
