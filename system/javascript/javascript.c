/**
 * @page JSAPI JavaScript Public API
 * JavaScript Public API refers that a function can be called from the JavaScript as well.
 * Please refer to the JavaScript engine documentation for more information and examples.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "javascript.h"
#include "system/timer/timer.h"
#include "system/debug/debug.h"
#include <duktape.h>
#include <duktape_opengl.h>

#include "system/javascript/bindings/bindings.h"

#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/graphics/shader/shader.h"
#include "system/graphics/font/font.h"
#include "system/graphics/image/image.h"
#include "system/graphics/object/object3d.h"
#include "system/graphics/object/basic3dshapes.h"
#include "system/graphics/object/lighting.h"
#include "system/graphics/particle/particle.h"
#include "system/datatypes/memory.h"
#include "system/player/player.h"
#include "system/ui/window/window.h"
#include "system/ui/window/menu.h"
#include "system/ui/input/input.h"
#include "system/audio/sound.h"
#include "system/io/io.h"
#include "system/rocket/synceditor.h"
#include "system/math/splines/spline.h"


static duk_context *ctx = NULL;

static void duk_jsDefineGlobalFunctions()
{
	duk_push_global_object(ctx);
	
	bindJsMiscellaneousFunctions(ctx);
	bindJsOpenGlFunctions(ctx);
	bindJsAntTweakBarFunctions(ctx);
	bindJsAudioFunctions(ctx);
	bindJsGraphicsFunctions(ctx);
	bindJsUiFunctions(ctx);
	bindJsPlayerFunctions(ctx);
	bindJsTimerFunctions(ctx);
	bindJsSyncEditorFunctions(ctx);

	bindJsCustomFunctions(ctx);

	duk_pop(ctx);
	
	
	duk_gl_push_opengl_bindings(ctx);
}

static int stackTraceCalled = 0;

void* jsGetDuktapeContext()
{
	return (void*)ctx;
}

static duk_idx_t jsSdlEventToObject(duk_context *ctx, SDL_Event* event)
{
	duk_idx_t obj = duk_push_object(ctx);

	duk_push_uint(ctx, event->type);
	duk_put_prop_string(ctx, obj, "type");

	duk_push_number(ctx, (double)timerGetTime());
	duk_put_prop_string(ctx, obj, "time");

	if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
	{
		duk_idx_t keyboard_obj = duk_push_object(ctx);
		duk_push_uint(ctx, event->key.state);
		duk_put_prop_string(ctx, keyboard_obj, "state");

		duk_push_uint(ctx, event->key.keysym.sym);
		duk_put_prop_string(ctx, keyboard_obj, "symbol");

		duk_put_prop_string(ctx, obj, "keyboard");
	}

	if (event->type == SDL_MOUSEMOTION || event->type == SDL_MOUSEBUTTONUP || event->type == SDL_MOUSEBUTTONDOWN)
	{
		int mouseX = 0;
		int mouseY = 0;
		inputGetMouseState(&mouseX, &mouseY);

		duk_idx_t mouse_obj = duk_push_object(ctx);
		duk_push_int(ctx, mouseX);
		duk_put_prop_string(ctx, mouse_obj, "x");
		duk_push_int(ctx, mouseY);
		duk_put_prop_string(ctx, mouse_obj, "y");
		duk_put_prop_string(ctx, obj, "mouse");		
	}

	return obj;
}

void jsAddInputEvent(void* eventPtr)
{
	assert(eventPtr);
	SDL_Event *event = (SDL_Event*)eventPtr;

	//duk_context *ctx, 
	const char *class = "Input";
	const char *method = "addEvent";

	duk_push_global_object(ctx);
	duk_push_string(ctx, class);
	duk_get_prop(ctx, -2);
	duk_push_string(ctx, method);
	duk_get_prop(ctx, -2);
	jsSdlEventToObject(ctx, event);

	duk_int_t returnValue = duk_pcall(ctx, 1); //calls: Input.addEvent(inputEventObject)

	if (returnValue != DUK_EXEC_SUCCESS)
	{
		debugErrorPrintf("eval failed for '%s.%s()': %s\n", class, method, duk_safe_to_string(ctx, -1));
		windowSetTitle("JS ERROR");
	}
	duk_pop_n(ctx, 3);
	
	if (returnValue != DUK_EXEC_SUCCESS && !stackTraceCalled)
	{
		jsEvalString("Utils.debugPrintStackTrace();");
		stackTraceCalled = 1;
	}
}

static int useInput = 0;
void jsSetUseInput(int _useInput)
{
	useInput = _useInput;
}

int jsIsUseInput(void)
{
	return useInput;
}

void jsCallClassMethod(const char *class, const char *method, const char *effectClassName)
{
	duk_push_global_object(ctx);
	duk_push_string(ctx, class);
	duk_get_prop(ctx, -2);
	duk_push_string(ctx, method);
	duk_get_prop(ctx, -2);
	duk_push_string(ctx, effectClassName);
	
	duk_int_t returnValue = duk_pcall(ctx, 1); //calls: class.method("effectClassName")

	if (returnValue != DUK_EXEC_SUCCESS)
	{
		debugErrorPrintf("eval failed for '%s.%s(\"%s\")': %s\n", class, method, effectClassName, duk_safe_to_string(ctx, -1));
		windowSetTitle("JS ERROR");
	}
	duk_pop_n(ctx, 3);
	
	if (returnValue != DUK_EXEC_SUCCESS && !stackTraceCalled)
	{
		jsEvalString("Utils.debugPrintStackTrace();");
		stackTraceCalled = 1;
	}
}

void jsEvalString(const char *string)
{
	
	duk_push_string(ctx, string);
	duk_int_t returnValue = duk_peval(ctx);
	if (returnValue != DUK_EXEC_SUCCESS)
	{
		debugErrorPrintf("eval failed for '%s': %s\n", string, duk_safe_to_string(ctx, -1));
		windowSetTitle("JS ERROR");
	}
	duk_pop(ctx);

	if (returnValue != DUK_EXEC_SUCCESS && !stackTraceCalled)
	{
		jsEvalString("Utils.debugPrintStackTrace();");
		stackTraceCalled = 1;
	}
}

void jsEvalFile(const char *file)
{
	const char *filePath = getFilePath(file);
	duk_int_t returnValue = duk_peval_file(ctx, filePath);
	if (returnValue != DUK_EXEC_SUCCESS)
	{
		debugErrorPrintf("Error in '%s': %s\n", filePath, duk_safe_to_string(ctx, -1));
		windowSetTitle("JS ERROR");
	}
	duk_pop(ctx);
	
	if (returnValue != DUK_EXEC_SUCCESS && !stackTraceCalled)
	{
		jsEvalString("Utils.debugPrintStackTrace();");
		stackTraceCalled = 1;
	}
}

static void jsPreInitEngine()
{
	if (fileExists("engine.js"))
	{
		jsEvalFile("engine.js"); //hack to enable --demoPath argument usage without needing to add engine.js to the data directory
	}
	else
	{
		jsEvalFile("data/js/engine.js");
	}
	jsEvalString("Settings.processDemoScript();");
}

int jsInit()
{
	debugPrintf("Initializing scripting.");
	ctx = duk_create_heap_default();
	if (!ctx) {
		debugErrorPrintf("Failed to create heap.");
		return -1;
	}

	duk_jsDefineGlobalFunctions();

	jsPreInitEngine();

	return 0;
}

void jsGarbageCollect()
{
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);
}

void jsInitEngine()
{
	jsGarbageCollect();
	jsPreInitEngine();
	jsEvalString("Settings.processPlayer();");
}

int jsDeinit()
{
	debugPrintf("Deinitializing scripting.");

	duk_destroy_heap(ctx);

	return 0;
}
