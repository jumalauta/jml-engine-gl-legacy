#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "graphicsIncludes.h"
#include "system/math/general/general.h"
#include "system/io/io.h"
#include "system/thread/thread.h"

#include "system/javascript/javascript.h"
#include "system/ui/input/input.h"

#include "bindings.h"

static int duk_readFile(duk_context *ctx)
{
	const char *filename = duk_get_string(ctx, 0);

	unsigned int fileSize = 0;
	char *data = ioReadFileToBuffer(filename, &fileSize);
	if (data)
	{
		//debugPrintf("'%s': '%s'",filename,(const char*)data);
		duk_push_string(ctx, (const char*)data);
		free(data);

		return 1; // file content
	}

	return 0; // could not read file
}

static int duk_evalFile(duk_context *ctx)
{
	int i;
	int n = duk_get_top(ctx);

	for (i = 0; i < n; i++)
	{
		jsEvalFile(duk_get_string(ctx, i));
	}

	return 0;  // no return value
}

static int duk_interpolateLinear(duk_context *ctx)
{
	double p = (double)duk_get_number(ctx, 0);
	double a = (double)duk_get_number(ctx, 1);
	double b = (double)duk_get_number(ctx, 2);

	duk_push_number(ctx, (double)interpolateLinear(p, a, b));

	return 1;
}

static int duk_interpolateSmootherStep(duk_context *ctx)
{
	double p = (double)duk_get_number(ctx, 0);
	double a = (double)duk_get_number(ctx, 1);
	double b = (double)duk_get_number(ctx, 2);

	duk_push_number(ctx, (double)interpolateSmootherStep(p, a, b));

	return 1;
}

static int duk_interpolateSmoothStep(duk_context *ctx)
{
	double p = (double)duk_get_number(ctx, 0);
	double a = (double)duk_get_number(ctx, 1);
	double b = (double)duk_get_number(ctx, 2);

	duk_push_number(ctx, (double)interpolateSmoothStep(p, a, b));

	return 1;
}

static int duk_interpolate(duk_context *ctx)
{
	double p = getClamp((double)duk_get_number(ctx, 0), 0.0, 1.0);
	double a = (double)duk_get_number(ctx, 1);
	double b = (double)duk_get_number(ctx, 2);
	int type = (int)duk_get_int(ctx, 3);
	
	double value = 0.0;
	switch(type)
	{
		case 2:
			value = interpolateSmootherStep(p, a, b);
			break;

		case 1:
			value = interpolateSmoothStep(p, a, b);
			break;

		case 0:
		default:
			value = interpolateLinear(p, a, b);
			break;
	}
	duk_push_number(ctx, value);

	return 1;
}

static int duk_random(duk_context *ctx)
{
	if (duk_get_top(ctx) > 0)
	{
		srand((unsigned int)duk_get_uint(ctx, 0));
	}

	duk_push_number(ctx, (double)((rand()%10000)/10000.0));
	return 1;
}

static int duk_screenPrint(duk_context *ctx)
{
#ifndef NDEBUG
	int i;
	int n = duk_get_top(ctx); 

	for (i = 0; i < n; i++)
	{
		screenPrintf("%s",duk_get_string(ctx, i));
	}
#endif

	return 0;  // no return value
}

static int duk_debugPrint(duk_context *ctx)
{
	int i;
	int n = duk_get_top(ctx); 

	for (i = 0; i < n; i++)
	{
		debugPrintf("%s",duk_get_string(ctx, i));
	}

	return 0;  // no return value
}

static int duk_debugWarningPrint(duk_context *ctx)
{
	int i;
	int n = duk_get_top(ctx); 

	for (i = 0; i < n; i++)
	{
		debugWarningPrintf("%s",duk_get_string(ctx, i));
	}

	return 0;  // no return value
}

static int duk_debugErrorPrint(duk_context *ctx)
{
	int i;
	int n = duk_get_top(ctx); 

	for (i = 0; i < n; i++)
	{
		debugErrorPrintf("%s",duk_get_string(ctx, i));
	}

	return 0;  // no return value
}

static int duk_threadWaitAsyncCalls(duk_context *ctx)
{
	threadWaitAsyncCalls();

	return 0;
}

int duk_jsSetUseInput(duk_context *ctx)
{
	int useInput = (int)duk_get_int(ctx, 0);

	jsSetUseInput(useInput);

	return 0;
}

int duk_jsIsUseInput(duk_context *ctx)
{
	duk_push_int(ctx, jsIsUseInput());

	return 1;
}

void bindJsMiscellaneousFunctions(duk_context *ctx)
{
	bindCFunctionToJs(screenPrint, DUK_VARARGS);
	bindCFunctionToJs(debugPrint, DUK_VARARGS);
	bindCFunctionToJs(debugWarningPrint, DUK_VARARGS);
	bindCFunctionToJs(debugErrorPrint, DUK_VARARGS);
	bindCFunctionToJs(readFile, 1);
	bindCFunctionToJs(evalFile, 1);

	bindCFunctionToJs(jsSetUseInput, 1);
	bindCFunctionToJs(jsIsUseInput, 0);

	bindCFunctionToJs(interpolateLinear, 3);
	bindCFunctionToJs(interpolateSmoothStep, 3);
	bindCFunctionToJs(interpolateSmootherStep, 3);
	bindCFunctionToJs(interpolate, DUK_VARARGS);
	
	bindCFunctionToJs(random, DUK_VARARGS);
	
	bindCFunctionToJs(threadWaitAsyncCalls, 0);
}
