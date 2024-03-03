#include <stdio.h>
#include <stdlib.h>

#include "playlist.h"
#include "system/player/player.h"
#include "system/timer/timer.h"
#include "system/audio/sound.h"

#include "system/datatypes/memory.h"

#include "scene_globals.h"


static float playlistTotalTime = 0.0f;
static char *playlistMusic = NULL;

/**
 * Add manual C/C++ effect/scene functions to the player.
 * @ingroup player
 */
void playlistInit(void)
{
	addPlayerEffect("GLOBALS", "scene_globals.c", sceneGlobalsInit, sceneGlobalsRun, sceneGlobalsDeinit);
	addPlayerScene(NULL, "GLOBALS", "GLOBALS", "0:00",  "#999:00");
}

/**
 * Set the main music file.
 * @ingroup audio
 * @ref JSAPI
 */
void setPlaylistMusic(const char *file)
{
	playlistMusic = memoryAllocateGeneral(NULL, strlen(file)+1, NULL);
	snprintf(playlistMusic, strlen(file)+1, "%s", file);
}

/**
 * Get the main music file.
 * @ingroup audio
 * @ref JSAPI
 */
const char *getPlaylistMusic(void)
{
	return (const char*)playlistMusic;
}

/**
 * Set length of the demo as time string. For example "1:30.5" = 90,5 seconds.
 * @ingroup player
 * @ref JSAPI
 */
void setPlaylistLength(const char *length)
{
	playlistTotalTime = convertTimeToSeconds(length);
}

/**
 * Get the length of the demo in seconds.
 * @ingroup player
 * @ref JSAPI
 */
float getPlaylistLength(void)
{
	return playlistTotalTime;
}
