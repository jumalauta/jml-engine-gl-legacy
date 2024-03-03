#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "system/rocket/synceditor.h"

#include "bindings.h"

static int duk_syncEditorSetRowsPerBeat(duk_context *ctx)
{
	int rpb = (int)duk_get_int(ctx, 0);
	syncEditorSetRowsPerBeat(rpb);

	return 0;
}

static int duk_syncEditorGetRowsPerBeat(duk_context *ctx)
{
	duk_push_int(ctx, (int)syncEditorGetRowsPerBeat());

	return 1;
}

static int duk_syncEditorGetTrack(duk_context *ctx)
{
	const char *trackName = (const char*)duk_get_string(ctx, 0);
	void *ptr = syncEditorGetTrack(trackName);

	duk_idx_t sync_track_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, ptr);
	duk_put_prop_string(ctx, sync_track_obj, "ptr");
	duk_push_string(ctx, trackName);
	duk_put_prop_string(ctx, sync_track_obj, "name");
	
	return 1;
}

static int duk_syncEditorGetTrackCurrentValue(duk_context *ctx)
{
	void *trackPointer = (void*)duk_get_pointer(ctx, 0);

	double value = syncEditorGetTrackCurrentValue(trackPointer);
	duk_push_number(ctx, (double)value);

	return 1;
}

void bindJsSyncEditorFunctions(duk_context *ctx)
{
	bindCFunctionToJs(syncEditorSetRowsPerBeat, 1);
	bindCFunctionToJs(syncEditorGetRowsPerBeat, 0);
	bindCFunctionToJs(syncEditorGetTrack, 1);
	bindCFunctionToJs(syncEditorGetTrackCurrentValue, 1);
}
