#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "graphicsIncludes.h"
#include "system/player/player.h"
#include "system/timer/timer.h"
#include "system/debug/debug.h"
#include "system/io/io.h"
#include "system/xml/xml.h"

#include "sync.h"
#include "synceditor.h"

extern void parseRocketXml(struct sync_device *d, const char *filename);

/**
 * @defgroup syncEditor GNU Rocket Sync Editor
 */
 
typedef struct sync_device sync_device;

static sync_device *rocket = NULL;
static int rowsPerBeat = 1;

/**
 * Set rows per beat. This is to control the precision of sync editor.
 * @param _rowsPerBeat [in] set the rows per beat, default is 8
 * @ingroup syncEditor
 */
void syncEditorSetRowsPerBeat(int _rowsPerBeat)
{
	rowsPerBeat = _rowsPerBeat;
}

/**
 * Get rows per beat.
 * @return Get the rows per beat, default is 8
 * @ingroup syncEditor
 */
int syncEditorGetRowsPerBeat(void)
{
	return rowsPerBeat;
}

static double syncEditorCurrentRow = 0.0;
static void syncEditorUpdateRow(void)
{
	syncEditorCurrentRow = timerGetCurrentBeat() * syncEditorGetRowsPerBeat();
}

static double syncEditorGetRow(void)
{
	return syncEditorCurrentRow;
}

/**
 * Get pointer to track data structure
 * @param trackName [in] Track name
 * @return pointer to track data structure
 * @ingroup syncEditor
 */
void* syncEditorGetTrack(const char *trackName)
{
	void *pointer = (void*)sync_get_track(rocket, trackName);
	assert(pointer);
	
	return pointer;
}

/**
 * Get current sync value in time
 * @param trackPointer [in] pointer to track data structure
 * @return current sync value
 * @ingroup syncEditor
 */
double syncEditorGetTrackCurrentValue(void *trackPointer)
{
	return sync_get_val((const struct sync_track*)trackPointer, syncEditorGetRow());
}

// callback functions for GNU Rocket

static void sync_pause_callback(void * UNUSED(d), int isPause)
{
	if (isPause)
	{
		if (!timerIsPause())
		{
			timerPause();
		}
	}
	else
	{
		if (timerIsPause())
		{
			timerPause();
		}
	}
}

static void sync_set_row_callback(void * UNUSED(d), int row)
{
	timerSetTime(row / (double)syncEditorGetRowsPerBeat() / timerGetBeatsPerMinute() * 60.0);
	if (timerIsPause())
	{
		playerForceRedraw();
	}
}

static int oldisplaying = -1;
static int sync_is_playing_callback(void * UNUSED(d))
{
	if (oldisplaying != !timerIsPause())
	{
		oldisplaying = !timerIsPause();
	}
	return !timerIsPause();
}

static struct sync_cb sync_editor_callback = {
	sync_pause_callback,
	sync_set_row_callback,
	sync_is_playing_callback
};

#define SYNC_DEFAULT_HOST "localhost"

static int syncEditor = 0;

/**
 * Check if demo tool is connected to GNU Rocket
 * @return 1 if tool is connected to GNU Rocket, 0 if not
 * @ingroup syncEditor
 */
int isSyncEditor(void)
{
	return syncEditor;
}

/**
 * Set sync editor value
 * @param _syncEditor [in] 1 if tool is connected to GNU Rocket, 0 if not
 * @ingroup syncEditor
 */
static void setSyncEditor(int _syncEditor)
{
	syncEditor = _syncEditor;
}

#define PATH_SIZE 2048

/**
 * Initialize sync editor. If tool mode is enabled then attempt to connect to GNU Rocket.
 * @ingroup syncEditor
 */
int syncEditorInit(void)
{
	char syncPath[PATH_SIZE];
	snprintf(syncPath, PATH_SIZE, "%s%s", getStartPath(), "data/sync/sync");
	rocket = sync_create_device(syncPath);
	if (!rocket)
	{
		debugErrorPrintf("Failed to initialize GNU Rocket sync device.");
		return -1;
	}
	
	if (isPlayerEditor())
	{
		if (sync_connect(rocket, SYNC_DEFAULT_HOST, SYNC_DEFAULT_PORT))
		{
			debugWarningPrintf("Could not connect to GNU Rocket. server:'%s:%d'", SYNC_DEFAULT_HOST, SYNC_DEFAULT_PORT);
		}
		else
		{
			debugPrintf("Connected to GNU Rocket. server:'%s:%d'", SYNC_DEFAULT_HOST, SYNC_DEFAULT_PORT);
			setSyncEditor(1);
		}
	}

	if (!isSyncEditor())
	{
		parseRocketXml(rocket, "data/sync/rocketman.rocket");
	}

	return 1;
}

/**
 * Update sync values and timing per frame.
 * @ingroup syncEditor
 */
void syncEditorRun(void)
{
	syncEditorUpdateRow();

	if (isSyncEditor())
	{
		if (sync_update(rocket, (int)floor(syncEditorGetRow()), &sync_editor_callback, NULL))
		{
			sync_connect(rocket, SYNC_DEFAULT_HOST, SYNC_DEFAULT_PORT);
		}
	}
}

/**
 * Deinitialize sync editor. Save created tracks if GNU Rocket connection was established.
 * @ingroup syncEditor
 */
void syncEditorDeinit(void)
{
	if (isSyncEditor())
	{
#ifndef LITTLE_ENDIAN
		debugErrorPrintf("Endianess issues may appear... FIXMEFIXME");
#endif
		sync_save_tracks(rocket);
	}

	sync_destroy_device(rocket);
}
