#include <windows.h>
#include <stdbool.h>

#include "error_handling.h"

#define EVENT_NOTIFY_KONC4D "konc4d_event_write"
#define EVENT_COMMAND_CONFIRM "konc4d_event_confirm"
#define EVENT_COMMAND_ERROR "konc4d_event_error"
#define EVENT_KONC4D_SHUTDOWN "konc4d_event_shutdown"
#define EVENT_KONC4D_STARTUP "konc4d_event_startup"


ReturnCode createEventObject(HANDLE *toWrite, const char *name) NO_IGNORE;
ReturnCode openEventObject(HANDLE *toWrite, const char *name) NO_IGNORE;
ReturnCode pingEventObject(HANDLE event) NO_IGNORE;
ReturnCode resetEventObject(HANDLE event) NO_IGNORE;
ReturnCode waitOnEventObject(HANDLE event, unsigned timeoutMs) NO_IGNORE;
ReturnCode waitOnEventObjects(HANDLE *events, unsigned count, unsigned timeoutMs, unsigned *pingedIndex) NO_IGNORE;
ReturnCode sendNotification(const char *eventName);
