#include "command_execution.h"
#include "action_printing.h"
#include "logging.h"
#include "shared_memory.h"
#include "input_loop.h"
#include "konc4d_ipc.h"
#include "action_receiving.h"
#include "timestamps.h"
#include "events.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>


ReturnCode executeStart(void)
{
    HANDLE startupEvent;
    ENSURE(createEventObject(&startupEvent, EVENT_KONC4D_STARTUP));
    RETURN_FAIL(startKonc4d());
    ReturnCode done;
    RETHROW(done = waitOnEventObject(startupEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d did not start up - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 start command timed out; no startup detected");
        return RET_FAILURE;
    }
    puts("start command executed successfully.");
    LOG_LINE(LOG_INFO, "konc4 start command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeStop(void)
{
    if(isKonc4dOn() != RET_SUCCESS)
    {
        puts("konc4d is off.");
        return RET_FAILURE;
    }
    HANDLE shutdownEvent;
    ENSURE(createEventObject(&shutdownEvent, EVENT_KONC4D_SHUTDOWN));
    RETURN_FAIL(fullSendMessage("STOP"));
    ReturnCode done;
    RETHROW(done = waitOnEventObject(shutdownEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d did not shut down - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 stop command timed out; no shutdown detected");
        return RET_FAILURE;
    }
    puts("stop command executed successfully.");
    LOG_LINE(LOG_INFO, "konc4 stop command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeReset(void)
{
    if(isKonc4dOn() != RET_SUCCESS)
    {
        puts("konc4d is off.");
        return RET_FAILURE;
    }
    HANDLE shutdownEvent, startupEvent;
    ENSURE(createEventObject(&shutdownEvent, EVENT_KONC4D_SHUTDOWN));
    ENSURE(createEventObject(&startupEvent, EVENT_KONC4D_STARTUP));
    RETURN_FAIL(fullSendMessage("RESET"));
    ReturnCode done;
    RETHROW(done = waitOnEventObject(shutdownEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d did not shut down - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 reset command timed out; no shutdown detected");
        return RET_FAILURE;
    }
    RETHROW(done = waitOnEventObject(startupEvent, EVENT_TIMEOUT));
    if(done == RET_FAILURE)
    {
        puts("konc4d shut down, but did not start up again - see logs for more information.");
        LOG_LINE(LOG_WARNING, "konc4 reset command failed in konc4d; no startup detected after shutdown");
        return RET_FAILURE;
    }
    puts("reset command executed successfully.");
    LOG_LINE(LOG_INFO, "konc4 reset command executed successfully");
    return RET_SUCCESS;
}


ReturnCode executeSkip(unsigned minutesToSkip)
{
    if(minutesToSkip == 0)
    {
        printf("Skip command expects a positive integer argument.\n");
        LOG_LINE(LOG_WARNING, "skip command received invalid argument: %d", minutesToSkip);
        return RET_FAILURE;
    }
    if(minutesToSkip > 7200)
    {
        printf("Skipping more than five days at once is not supported.\n");
        LOG_LINE(LOG_WARNING, "skip command received invalid argument: %d", minutesToSkip);
        return RET_FAILURE;
    }
    HANDLE events[2];
    ENSURE(createEventObject(&events[0], EVENT_COMMAND_CONFIRM));
    ENSURE(createEventObject(&events[1], EVENT_COMMAND_ERROR));
    RETURN_FAIL(fullSendMessageWithArgument("SKIP", minutesToSkip));
    return checkKonc4dResponse(events, "skip", EVENT_TIMEOUT);
}


ReturnCode executeShow(struct ShowArgument argument)
{
    struct ReceivedActions receivedActions;
    RETURN_FAIL(obtainActions(&receivedActions));
    printAllActions(argument, &receivedActions);
    free(receivedActions.actionVector);

    LOG_LINE(LOG_INFO, "konc4 show command executed successfully");
    return RET_SUCCESS;
}
