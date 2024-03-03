#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "system/timer/timer.h"

#include "bindings.h"

static int duk_convertTimeToSeconds(duk_context *ctx)
{
	const char* timeString = (const char*)duk_get_string(ctx, 0);
	duk_push_number(ctx, (double)convertTimeToSeconds(timeString));
	
	return 1;
}

static int duk_timerGetBeatsPerMinute(duk_context *ctx)
{
	duk_push_number(ctx, (double)timerGetBeatsPerMinute());

	return 1;
}

static int duk_timerSetBeatsPerMinute(duk_context *ctx)
{
	double bpm = (double)duk_get_number(ctx, 0);
	timerSetBeatsPerMinute(bpm);

	return 0;
}

static int duk_timerGetBeatInSeconds(duk_context *ctx)
{
	duk_push_number(ctx, (double)timerGetBeatInSeconds());

	return 1;
}

static int duk_timerGetCurrentBeat(duk_context *ctx)
{
	duk_push_number(ctx, (double)timerGetCurrentBeat());

	return 1;
}

static int duk_timerGetTime(duk_context *ctx)
{
	duk_push_number(ctx, (double)timerGetTime());

	return 1;
}

static int duk_timerInit(duk_context *ctx)
{
	double newEndTime = (double)duk_get_number(ctx, 0);
	timerInit(newEndTime);

	return 0;
}

static int duk_timerSleep(duk_context *ctx)
{
	int millis = (int)duk_get_int(ctx, 0);
	timerSleep(millis);

	return 0;
}

static int duk_timerGetFpsCorrection(duk_context *ctx)
{
	duk_push_number(ctx, (double)timerGetFpsCorrection());

	return 1;
}

static int duk_timerSetTargetFps(duk_context *ctx)
{
	double targetFps = (double)duk_get_number(ctx, 0);
	timerSetTargetFps(targetFps);

	return 0;
}

static int duk_timerGetTargetFps(duk_context *ctx)
{
	duk_push_number(ctx, (double)timerGetTargetFps());

	return 1;
}

void bindJsTimerFunctions(duk_context *ctx)
{
	bindCFunctionToJs(convertTimeToSeconds, 1);
	bindCFunctionToJs(timerGetBeatsPerMinute, 0);
	bindCFunctionToJs(timerSetBeatsPerMinute, 1);
	bindCFunctionToJs(timerGetBeatInSeconds, 0);
	bindCFunctionToJs(timerGetCurrentBeat, 0);
	bindCFunctionToJs(timerGetTime, 0);
	bindCFunctionToJs(timerInit, 1);
	bindCFunctionToJs(timerSleep, 1);
	bindCFunctionToJs(timerGetFpsCorrection, 0);
	bindCFunctionToJs(timerSetTargetFps, 1);
	bindCFunctionToJs(timerGetTargetFps, 0);
}
