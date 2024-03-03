#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "system/player/player.h"

#include "bindings.h"

static int duk_getSceneName(duk_context *ctx)
{
	duk_push_string(ctx, getSceneName());
	
	return 1;
}

static int duk_getSceneStartTime(duk_context *ctx)
{
	duk_push_number(ctx, (double)getSceneStartTime());
	
	return 1;
}

static int duk_getSceneEndTime(duk_context *ctx)
{
	duk_push_number(ctx, (double)getSceneEndTime());
	
	return 1;
}

static int duk_getSceneTimeFromStart(duk_context *ctx)
{
	duk_push_number(ctx, (double)getSceneTimeFromStart());
	
	return 1;
}

static int duk_getSceneProgressPercent(duk_context *ctx)
{
	duk_push_number(ctx, (double)getSceneProgressPercent());
	
	return 1;
}


static int duk_setResourceCount(duk_context *ctx)
{
	int resource_count = (int)duk_get_int(ctx, 0);

	setResourceCount(resource_count);
	return 0;
}

static int duk_notifyResourceLoaded(duk_context *ctx)
{
	notifyResourceLoaded();
	return 0;
}

static int duk_addPlayerEffect(duk_context *ctx) {
	const char *name = duk_get_string(ctx, 0);
	const char *reference = duk_get_string(ctx, 1);

	//debugPrintf("name:'%s', reference:'%s'", name, reference);
	addPlayerEffect(name, reference, NULL, NULL, NULL);

	return 0;  // no return value
}

static int duk_addPlayerScene(duk_context *ctx) {
	const char *parentName = duk_get_string(ctx, 0);
	const char *name = duk_get_string(ctx, 1);
	const char *effectName = duk_get_string(ctx, 2);
	const char *startTime = duk_get_string(ctx, 3);
	const char *durationTime = duk_get_string(ctx, 4);

	//debugPrintf("name:'%s', effectName:'%s'", name, effectName);
	addPlayerScene(getPlayerScene(parentName), name, effectName, startTime, durationTime);

	return 0;  // no return value
}


int duk_setPlayerAutoClear(duk_context *ctx)
{
	int autoClear = (int)duk_get_boolean(ctx, 0);

	setPlayerAutoClear(autoClear);

	return 0;
}

void bindJsPlayerFunctions(duk_context *ctx)
{
	bindCFunctionToJs(setPlayerAutoClear, 1);

	bindCFunctionToJs(getSceneName, 1);
	bindCFunctionToJs(getSceneStartTime, 1);
	bindCFunctionToJs(getSceneEndTime, 1);
	bindCFunctionToJs(getSceneTimeFromStart, 0);
	bindCFunctionToJs(getSceneProgressPercent, 0);

	bindCFunctionToJs(setResourceCount, 1);
	bindCFunctionToJs(notifyResourceLoaded, 0);

	bindCFunctionToJs(addPlayerEffect, 2);
	bindCFunctionToJs(addPlayerScene, 5);
}
