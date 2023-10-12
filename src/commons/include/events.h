#include <windows.h>
#include <stdbool.h>

#include "error_handling.h"

#define EVENT_TO_KONC4D "konc4d_event_write"
#define EVENT_FROM_KONC4D "konc4d_event_read"
#define EVENT_COMMAND_CONFIRM "konc4d_event_confirm"


ReturnCode createEventObject(HANDLE *toWrite, const char *name) NO_IGNORE;
ReturnCode openEventObject(HANDLE *toWrite, const char *name) NO_IGNORE;
ReturnCode pingEventObject(HANDLE event) NO_IGNORE;
ReturnCode resetEventObject(HANDLE event) NO_IGNORE;
ReturnCode waitOnEventObject(HANDLE event, unsigned timeoutMs) NO_IGNORE;
