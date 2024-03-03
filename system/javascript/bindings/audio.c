#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "system/audio/sound.h"
#include "effects/playlist.h"

#include "bindings.h"

static int duk_soundAddSongToPlaylist(duk_context *ctx)
{
	const char* filename = (const char*)duk_get_string(ctx, 0);
	const char* title = "";
	int length = 0;

	int argc = duk_get_top(ctx);
	if (argc > 1)
	{
		title = (const char*)duk_get_string(ctx, 1);
	}
	if (argc > 2)
	{
		length = (int)duk_get_int(ctx, 2);
	}

	duk_push_int(ctx, (int)soundAddSongToPlaylist(filename, title, length));

	return 1;
}

static int duk_soundLoadPlaylist(duk_context *ctx)
{
	const char* filename = (const char*)duk_get_string(ctx, 0);
	soundLoadPlaylist(filename);

	return 0;
}

static int duk_soundLoadSong(duk_context *ctx)
{
	int song_number = (int)duk_get_int(ctx, 0);
	soundLoadSong(song_number);

	return 0;
}

static int duk_soundGetSongCurrentPlayTime(duk_context *ctx)
{
	duk_push_number(ctx, (double)soundGetSongCurrentPlayTime());

	return 1;
}

static int duk_soundPlaySong(duk_context *ctx)
{
	int song_number = (int)duk_get_int(ctx, 0);
	soundPlaySong(song_number);

	return 0;
}

static int duk_soundPause(duk_context *ctx)
{
	soundPause();

	return 0;
}

static int duk_soundStop(duk_context *ctx)
{
	soundStop();

	return 0;
}

static int duk_soundMute(duk_context *ctx)
{
	int mute_sound = (int)duk_get_int(ctx, 0);

	soundMute(mute_sound);

	return 0;
}

static int duk_soundIsMute(duk_context *ctx)
{
	duk_push_int(ctx, (int)soundIsMute());

	return 1;
}

static int duk_soundPreviousTrack(duk_context *ctx)
{
	soundPreviousTrack();

	return 0;
}

static int duk_soundNextTrack(duk_context *ctx)
{
	soundNextTrack();

	return 0;
}

static int duk_soundGetPlaylistSize(duk_context *ctx)
{
	duk_push_int(ctx, (int)soundGetPlaylistSize());

	return 1;
}

static int duk_soundGetCurrentSong(duk_context *ctx)
{
	duk_push_int(ctx, (int)soundGetCurrentSong());

	return 1;
}

static int duk_soundGetSongLength(duk_context *ctx)
{
	int song_number = (int)duk_get_int(ctx, 0);

	duk_push_int(ctx, (int)soundGetSongLength(song_number));

	return 1;
}

static int duk_soundGetSongFilename(duk_context *ctx)
{
	int song_number = (int)duk_get_int(ctx, 0);

	duk_push_string(ctx, (const char*)soundGetSongFilename(song_number));

	return 1;
}

static int duk_soundGetSongName(duk_context *ctx)
{
	int song_number = (int)duk_get_int(ctx, 0);

	duk_push_string(ctx, (const char*)soundGetSongName(song_number));

	return 1;
}

static int duk_soundGetTrackNumber(duk_context *ctx)
{
	int song_number = (int)duk_get_int(ctx, 0);

	duk_push_int(ctx, soundGetTrackNumber(song_number));

	return 1;
}

static int duk_soundIsPlaying(duk_context *ctx)
{
	duk_push_int(ctx, soundIsPlaying());

	return 1;
}

static int duk_soundClearPlaylist(duk_context *ctx)
{
	soundClearPlaylist();

	return 0;
}

static int duk_soundSetPosition(duk_context *ctx)
{
	float position = (float)duk_get_number(ctx, 0);

	soundSetPosition(position);

	return 0;
}

static int duk_setPlaylistMusic(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx); 

	for (i = 0; i < n; i++)
	{
		setPlaylistMusic(duk_get_string(ctx, i));
	}

	return 0;  // no return value
}

static int duk_setPlaylistLength(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx); 

	for (i = 0; i < n; i++)
	{
		setPlaylistLength(duk_get_string(ctx, i));
	}

	return 0;  // no return value
}

void bindJsAudioFunctions(duk_context *ctx)
{
	bindCFunctionToJs(soundLoadPlaylist, 1);
	bindCFunctionToJs(soundAddSongToPlaylist, DUK_VARARGS);
	bindCFunctionToJs(soundLoadSong, 1);
	bindCFunctionToJs(soundGetSongCurrentPlayTime, 0);
	bindCFunctionToJs(soundPlaySong, 1);
	bindCFunctionToJs(soundPause, 0);
	bindCFunctionToJs(soundStop, 0);
	bindCFunctionToJs(soundMute, 1);
	bindCFunctionToJs(soundIsMute, 0);
	bindCFunctionToJs(soundPreviousTrack, 0);
	bindCFunctionToJs(soundNextTrack, 0);
	bindCFunctionToJs(soundGetPlaylistSize, 0);
	bindCFunctionToJs(soundGetCurrentSong, 0);
	bindCFunctionToJs(soundGetSongLength, 1);
	bindCFunctionToJs(soundGetSongFilename, 1);
	bindCFunctionToJs(soundGetSongName, 1);
	bindCFunctionToJs(soundGetTrackNumber, 1);
	bindCFunctionToJs(soundIsPlaying, 0);
	bindCFunctionToJs(soundClearPlaylist, 0);
	bindCFunctionToJs(soundSetPosition, 1);

	bindCFunctionToJs(setPlaylistMusic, 1);
	bindCFunctionToJs(setPlaylistLength, 1);
}
