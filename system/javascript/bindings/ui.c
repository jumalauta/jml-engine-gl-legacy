#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "system/ui/input/input.h"
#include "system/ui/window/window.h"
#include "system/ui/window/menu.h"

#include "bindings.h"

static int duk_setMenuComponentSelected(duk_context *ctx)
{
	unsigned int component_i = (unsigned int)duk_get_uint(ctx, 0);
	assert(component_i < 4);
	
	gui_component_t *component = get_gui_component((int)component_i);

	gui_component_set_selected(component);
	return 0;
}

static int duk_setMenuResolution(duk_context *ctx)
{
	unsigned int component_i = (unsigned int)duk_get_uint(ctx, 0);
	assert(component_i < 4);
	
	gui_component_t *component = get_gui_component((int)component_i);
	const char *text = (const char*)duk_get_string(ctx, 1);

	gui_component_set_text(component, text);
	return 0;
}

static int duk_windowSetTitle(duk_context *ctx)
{
	const char *title = (const char*)duk_get_string(ctx, 0);

	windowSetTitle(title);
	
	return 0;
}

static int duk_setWindowScreenAreaAspectRatio(duk_context *ctx)
{
	double width = (double)duk_get_number(ctx, 0);
	double height = (double)duk_get_number(ctx, 1);

	setWindowScreenAreaAspectRatio(width, height);
	
	return 0;
}

static int duk_getWindowScreenAreaAspectRatio(duk_context *ctx)
{
	duk_push_number(ctx, (double)getWindowScreenAreaAspectRatio());
	
	return 1;
}

static int duk_setScreenDimensions(duk_context *ctx)
{
	int width = (int)duk_get_int(ctx, 0);
	int height = (int)duk_get_int(ctx, 1);

	setScreenDimensions(width, height);
	
	return 0;
}

static int duk_getScreenWidth(duk_context *ctx)
{
	duk_push_int(ctx, getScreenWidth());
	
	return 1;
}

static int duk_getScreenHeight(duk_context *ctx)
{
	duk_push_int(ctx, getScreenHeight());
	
	return 1;
}


static int duk_resetViewport(duk_context *ctx)
{
	resetViewport();
	
	return 0;
}

static int duk_viewReset(duk_context *ctx)
{
	viewReset();
	
	return 0;
}

static int duk_isUserExit(duk_context *ctx)
{
	duk_push_int(ctx, isUserExit());
	
	return 1;	
}

void bindJsUiFunctions(duk_context *ctx)
{
	bindCFunctionToJs(setMenuComponentSelected, 1);
	bindCFunctionToJs(setMenuResolution, 2);
	bindCFunctionToJs(windowSetTitle, 1);
	bindCFunctionToJs(setWindowScreenAreaAspectRatio, 2);
	bindCFunctionToJs(getWindowScreenAreaAspectRatio, 0);
	bindCFunctionToJs(setScreenDimensions, 2);
	bindCFunctionToJs(getScreenWidth, 0);
	bindCFunctionToJs(getScreenHeight, 0);
	bindCFunctionToJs(resetViewport, 0);
	bindCFunctionToJs(viewReset, 0);
	bindCFunctionToJs(isUserExit, 0);
}
