#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @defgroup audio Sound functionality.
 */

//Syncs - boldly going where no hack has gone before
#define LEAD	0
#define KICK	1
#define SNARE	2

#ifdef MINIFMOD
#include "minifmod.h"
#endif
static int sync = -1;
#ifdef FMOD
#include "fmod.h"
#include "fmod_errors.h"
static FMOD_SYSTEM *fmod_system = 0;
static FMOD_CHANNEL *channel = 0;
static unsigned int fmod_sync;
#elif MINIFMOD
static FMUSIC_MODULE *mod[MAX_SONGS];
#elif BASS
#include "bass.h"


int strc=0;
HMUSIC *mods=NULL;
int modc=0;
HSAMPLE *sams=NULL;
int samc=0;

char **strlist=NULL;
char **modlist=NULL;
char **samlist=NULL;
#elif SDL_MIXER
#include <SDL_mixer.h>

static int audio_rate = 44100;
static Uint16 audio_format = MIX_DEFAULT_FORMAT;
static int audio_channels = 2;
static int audio_buffers = 4096;
static int loop = 0;
#endif

#include "effects/playlist.h"
#include "system/debug/debug.h"
#include "system/timer/timer.h"
#include "system/io/io.h"

#include "sound.h"

#define MAX_SONGS 256
int song_current = 0; //the current playing song
int song_i = 0; //amount of songs
int song_playing = 0;
int init_done = 0;

typedef struct {
	unsigned int length;
	char *name;
	char *filename;
#ifdef FMOD
	FMOD_SOUND *sound;
#elif MINIFMOD
	FMUSIC_MODULE *sound;
#elif BASS
	HSTREAM sound;
#elif SDL_MIXER
	Mix_Music* sound;
#endif
} song_t;

song_t songs[MAX_SONGS];

static double song_play_time = 0.0f;
static double song_play_time_start = 0.0f;
/*
god damn thygrion, you make too many varying patterns for syncing ;D
*/

//legend
//-3 = empty order
//-2 = mapping ends
//-1 = change instrument
//sync_num1, sync_num2 ... sync_numN

static const signed char instrumentOrderMap[] = {
	//chime
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,
	//snare
//18 | 7
	-3,-3,-3,-3,-3,-3,1,1,1,1,1,1,1,1,1,1,1,1,1,2,-1,
	-2
};

static const signed short soundSync[] = {
	//lead
	8,20,40,52,-1,
	//snare
	8,24,40,56,-1,
	//snare2
	12,24,44,-1,
	//ending char of the sync
	-2
};


#ifdef MINIFMOD
//Since we're using MiniFMOD the song is
//going to be loaded from song.h "char * data" variable
#include "resources/song.h"

typedef struct
{
	int length;
	int pos;
	void *data;
} MEMFILE;

static unsigned int memopen(char *name)
{
	MEMFILE *memfile;
	memfile = (MEMFILE *)calloc(sizeof(MEMFILE),1);

	memfile->length = song.dataLength;
	memfile->data = calloc(memfile->length,1);
	memcpy(memfile->data, (void*)song.data, memfile->length);
	memfile->pos = 0;

	return((unsigned int)memfile);
}

static void memclose(unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;
	free(memfile->data);
	free(memfile);
}

static int memread(void *buffer, int size, unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;

	if (memfile->pos + size >= memfile->length)
	{
		size = memfile->length - memfile->pos;
	}

	memcpy(buffer, (char *)memfile->data+memfile->pos, size);
	memfile->pos += size;

	return(size);
}

static void memseek(unsigned int handle, int pos, signed char mode)
{
	MEMFILE *memfile = (MEMFILE *)handle;

	switch(mode)
	{
		case SEEK_SET:
			memfile->pos = pos;
			break;
		case SEEK_CUR:
			memfile->pos += pos;
			break;
		case SEEK_END:
			memfile->pos = memfile->length + pos;
			break;
	}

	if (memfile->pos > memfile->length)
	{
		memfile->pos = memfile->length;
	}
}

static int memtell(unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;
	return(memfile->pos);
}
#endif

int sound_getOrder(void)
{
#ifdef FMOD
	FMOD_Channel_GetPosition(channel, &fmod_sync, FMOD_TIMEUNIT_MODORDER);
	return fmod_sync;
#elif MINIFMOD
	return FMUSIC_GetOrder(mod);
#elif BASS
	debugErrorPrintf("Not supported with BASS lib");
	return -1;
#elif SDL_MIXER
    debugErrorPrintf("Not supported with SDL_Mixer");
    return -1;
#endif
}

int sound_getRow(void)
{
#ifdef FMOD
	FMOD_Channel_GetPosition(channel, &fmod_sync, FMOD_TIMEUNIT_MODROW);
	return fmod_sync;
#elif MINIFMOD
	return FMUSIC_GetRow(mod);
#elif BASS
	debugErrorPrintf("Not supported with BASS lib");
	return -1;
#elif SDL_MIXER
    debugErrorPrintf("Not supported with SDL_Mixer");
    return -1;
#endif
}

int sound_isSync(int instrument)
{
	int ins, i;
	for(ins = 0, i = 0; instrumentOrderMap[i] != -2; i++)
	{
		if (ins == instrument)
		{
			int ins_pos = instrumentOrderMap[i + sound_getOrder()];
			for(ins = 0, i = 0; soundSync[i] != -2; i++)
			{

				if ((ins == ins_pos)
					&& (sound_getRow() == soundSync[i])
					&& (sync != soundSync[i]))
				{
/*printf("weeaboo %d %d\n",sound_getRow(), sound_getOrder());*/
					sync = soundSync[i];
					return 1;
				}
				else if (soundSync[i] == -1) { ins++; }
			}

			return 0;
		}
		else if (instrumentOrderMap[i] == -1) { ins++; }
	}

	return 0;
}

static double timePosition = 0.0f;

/**
 * Rewind sound to position in seconds.
 * @ingroup audio
 * @ref JSAPI
 */
void soundSetPosition(double position)
{
	timePosition = position;
	
#ifdef FMOD
	FMOD_Channel_SetPosition(channel, (unsigned int)(timePosition*1000), FMOD_TIMEUNIT_MS);
#elif MINIFMOD
	debugErrorPrintf("soundSetPosition not supported by MINIFMOD mode");
#elif BASS
	BASS_ChannelSetPosition(sound[song_current], BASS_ChannelSeconds2Bytes(sound[song_current],timePosition), 0);
#elif SDL_MIXER
	Mix_RewindMusic();
	Mix_SetMusicPosition(timePosition);
#endif
}

/**
 * Initialize audio.
 * @ingroup audio
 */
void soundInit(void)
{
	if (init_done)
	{
		return;
	}

#ifdef FMOD
	FMOD_RESULT f = FMOD_OK;
	unsigned int version = 0;

	//f = FMOD_Debug_SetLevel(FMOD_DEBUG_LEVEL_ALL);
	//debugPrintf("FMOD Debug Set Level: '%s'", FMOD_ErrorString(f));

	f = FMOD_System_Create(&fmod_system);
	debugPrintf("FMOD System Create: '%s'", FMOD_ErrorString(f));
	f = FMOD_System_GetVersion(fmod_system, &version);
	debugPrintf("FMOD System Get Version: '%s' '%d' <-> '%d'", FMOD_ErrorString(f), FMOD_VERSION, version);
	f = FMOD_System_Init(fmod_system, 512, FMOD_INIT_NORMAL, NULL);
	debugPrintf("FMOD System Init: '%s'", FMOD_ErrorString(f));

#elif MINIFMOD
	FSOUND_File_SetCallbacks(memopen,
		memclose, memread, memseek, memtell);
	FSOUND_Init(44100, 0);

#elif BASS
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		debugWarningPrintf("Used BASS library versions do not match: Used: '%d', Engine: '%d'",HIWORD(BASS_GetVersion()), BASSVERSION);
	}
	if (!BASS_Init(-1,44100,0,0, NULL))
	{
		debugErrorPrintf("BASS Init error: '%d'", BASS_ErrorGetCode());
	}
	if (!BASS_Start())
	{
		debugErrorPrintf("BASS Start error: '%d'", BASS_ErrorGetCode());
	}	

#elif SDL_MIXER
	if(Mix_OpenAudio(audio_rate, MIX_DEFAULT_FORMAT, audio_channels, audio_buffers) < 0)
	{
		debugErrorPrintf(
			"Audio initialization failed! rate:%d, format:%d, channels:%d, buffer:%d",
			audio_rate, audio_format, audio_channels, audio_buffers
		);
	}
	else
	{
		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
		debugPrintf(
			"Audio initialized successfully! rate:%d, format:%d, channels:%d, buffer:%d",
			audio_rate, audio_format, audio_channels, audio_buffers
		);
	}
#endif

	init_done = 1;
}

//soundLoadSong(getPlaylistMusic());
//soundPlaySong(0);

/**
 * Load playlist in PLS format
 * @param _filename PLS file 
 * @ingroup audio
 * @ref JSAPI
 */
void soundLoadPlaylist(const char *_filename)
{
	soundClearPlaylist();
	
	const char *filename = getFilePath(_filename);

	#define LINE_LENGTH 4096
	
	//void soundLoadSong(int song_number)
	//void soundAddSongToPlaylist(const char *filename, const char *title, int length)
//soundLoadSong(soundAddSongToPlaylist(const char *filename, const char *title, int length))
/*typedef struct {
	unsigned int length;
	char *name;
	char *filename;
} song_t;*/
	song_t load_song;
	load_song.length      = 0;
	load_song.name        = (char*)malloc(sizeof(char) * LINE_LENGTH);
	load_song.filename    = (char*)malloc(sizeof(char) * LINE_LENGTH);
	load_song.name[0]     = '\0';
	load_song.filename[0] = '\0';
	//strcpy(load_song.name, title);
	//strcpy(load_song.filename, title);

	char path[LINE_LENGTH] = {'\0'};
	int path_i = 0;
	for(path_i = strlen(filename)-1; path_i >= 0; path_i--)
	{
		if (filename[path_i] == '/' || filename[path_i] == '\\')
		{
			path_i++;
			strncpy(path, filename, path_i);
			path[path_i] = '\0';
			break;
		}
	}

	FILE *playlist = fopen(filename, "r");
	if (playlist)
	{
		char line[LINE_LENGTH];
		int line_i = 0;
		int cursor = 0;
		char character = 0;

		while ((character = fgetc (playlist)) > 0)
		{
			int linebreak = 0;
			switch (character)
			{
				case '\n':
				case '\r':
					linebreak = 1;
					break;
				default:
					line[cursor] = character;
					cursor++;
					line[cursor] = '\0';
					break;
			}
			
			if (linebreak == 1 && line[0] != '\0')
			{
				line_i++;

				char type[LINE_LENGTH];
				//char track_number[LINE_LENGTH];
				char value[LINE_LENGTH];
				int type_i=0,/*track_number_i=0,*/value_i=0;
				type[0] = '\0';
				//track_number[0] = '\0';
				value[0] = '\0';

				int change=0;
				for(cursor=0; cursor<LINE_LENGTH; cursor++)
				{
					character = line[cursor];
					if (character == '\n' || character == '\r')
					{
						break;
					}
					if (character == '=')
					{
						change = 1;
						continue;
					}

					if (change == 0)
					{
						if (character >= '0' && character <= '9')
						{
							//track_number[track_number_i++] = character;
						}
						else
						{
							type[type_i++] = character;
						}
					}
					else
					{
						value[value_i++] = character;
					}
				}

				type[type_i] = '\0';
				//track_number[track_number_i] = '\0';
				value[value_i] = '\0';
				cursor = 0;

				//printf("%02d: '%s'\n", line_i, line);
									
				if (!strcmp(type, "Length"))
				{
					load_song.length = atoi(value);
				}
				else if (!strcmp(type, "File"))
				{
					strcpy(load_song.filename, path);
					strcpy(load_song.filename+strlen(path), value);
					load_song.filename[strlen(path)+strlen(value)] = '\0';
				}
				else if (!strcmp(type, "Title"))
				{
					strcpy(load_song.name, value);
					load_song.name[strlen(value)] = '\0';
				}

				if (load_song.length > 0 && strlen(load_song.filename) > 0 && strlen(load_song.name) > 0)
				{
					soundAddSongToPlaylist(load_song.filename, load_song.name, load_song.length);

					load_song.length = 0;
					load_song.filename[0] = '\0';
					load_song.name[0] = '\0';
				}
				
				linebreak = 0;
				cursor = 0;
				line[cursor] = '\0';
			}
		}

		fclose(playlist);
	}
	else
	{
		debugWarningPrintf("Playlist not found! filename:'%s'", filename);
	}
	
	free(load_song.name);
	free(load_song.filename);
}

/**
 * Add audio file to current playlist
 * @param _filename Audio file.
 * @param title Title of the audio file.
 * @param length Length of the audio file. 
 * @return Song position in the playlist.
 * @ingroup audio
 * @ref JSAPI
 */
int soundAddSongToPlaylist(const char *_filename, const char *title, int length)
{
	const char *filename = getFilePath(_filename);
	
	songs[song_i].length   = length;
	
	songs[song_i].name     = NULL;
	songs[song_i].name     = (char*)malloc(sizeof(char) * strlen(title)+1);
	songs[song_i].name[0]  = '\0';
	strcat(songs[song_i].name, title);
	
	songs[song_i].filename = NULL;
	songs[song_i].filename = (char*)malloc(sizeof(char) * strlen(filename)+1);
	songs[song_i].filename[0]  = '\0';
	strcat(songs[song_i].filename, filename);

	debugPrintf("%02d: filename:'%s', title:'%s', length:'%u'", song_i, songs[song_i].filename, songs[song_i].name, songs[song_i].length);

	song_i++;
	
	return song_i-1;
}

/**
 * Load song in the playlist.
 * @param song_number Song index position in the playlist.
 * @ingroup audio
 * @ref JSAPI
 */
void soundLoadSong(int song_number)
{
	song_number = song_number % song_i;

#ifdef FMOD
	songs[song_number].sound    = 0;
	
	FMOD_RESULT f = FMOD_OK;
	f = FMOD_System_CreateSound(fmod_system,
		songs[song_number].filename, FMOD_DEFAULT, 0, &songs[song_number].sound);
	debugPrintf("FMOD System Create Sound: '%s'", FMOD_ErrorString(f));
#elif MINIFMOD
	songs[song_number].sound = FMUSIC_LoadSong(songs[song_number].filename, NULL);
#elif BASS
	songs[song_number].sound = BASS_StreamCreateFile(FALSE, songs[song_number].filename, 0, 0, 0);
	if (!songs[song_number].sound)
	{
		debugErrorPrintf("BASS StreamCreateFile error: '%d'", BASS_ErrorGetCode());
	}
#endif
}

static int sound_pause = 0;
/**
 * Pause or unpause the audio
 * @ingroup audio
 * @ref JSAPI
 */
void soundPause(void)
{
	if (song_playing)
	{
		song_playing = 0;
		sound_pause = 1;
	}
	else if (sound_pause)
	{
		sound_pause = 0;
		song_playing = 1;
	}
	else
	{
		debugPrintf("Nothing playing. Sound is not paused.");
		return;
	}

#ifdef FMOD
	FMOD_RESULT f = FMOD_OK;
	f = FMOD_Channel_SetPaused(channel, sound_pause);
	debugPrintf("FMOD Channel Pause Sound: '%s'", FMOD_ErrorString(f));
#elif MINIFMOD
	debugErrorPrintf("Pause is not supported!!!");
#elif BASS
	if (!BASS_Pause())
	{
		debugErrorPrintf("BASS Free error: '%d'", BASS_ErrorGetCode());
	}
#elif SDL_MIXER
	if (sound_pause)
	{
		Mix_Pause(-1);
		Mix_PauseMusic();
	}
	else
	{
		Mix_Resume(-1);
		Mix_ResumeMusic();
	}
#endif
}

static int sound_mute = 0;
/**
 * Mute or unmute the audio
 * @param _mute_sound 1 is mute, 0 is unmute
 * @ingroup audio
 * @ref JSAPI
 */
void soundMute(int _mute_sound)
{
	sound_mute = _mute_sound;
}
/**
 * Check if audio is muted.
 * @return 1 is mute, 0 is unmute
 * @ingroup audio
 * @ref JSAPI
 */
int soundIsMute(void)
{
	return sound_mute;
}

/**
 * Stop audio.
 * @ingroup audio
 * @ref JSAPI
 */
void soundStop(void)
{
	sound_pause = 0;
	soundPause();

	song_play_time = 0.0f;
	soundSetPosition(song_play_time);
}

/**
 * Free sound and cleanup memory.
 * @ingroup audio
 */
void soundFree(void)
{
#ifdef FMOD
	FMOD_RESULT f = FMOD_OK;
	f = FMOD_Sound_Release(songs[song_current].sound);
	debugPrintf("FMOD Sound Release: '%s'", FMOD_ErrorString(f));
	f = FMOD_System_Close(fmod_system);
	debugPrintf("FMOD System Close: '%s'", FMOD_ErrorString(f));
	f = FMOD_System_Release(fmod_system);
	debugPrintf("FMOD System Release: '%s'", FMOD_ErrorString(f));
#elif MINIFMOD
	FMUSIC_FreeSong(songs[song_current].sound);
	FSOUND_Close();
#elif BASS
	if (!BASS_Free())
	{
		debugErrorPrintf("BASS Free error: '%d'", BASS_ErrorGetCode());
	}
#elif SDL_MIXER
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
	Mix_CloseAudio();
#endif
}

/**
 * Play previous track in the playlist. When rewinding first track the player will jump to the last track in the playlist.
 * @ingroup audio
 * @ref JSAPI
 */
void soundPreviousTrack(void)
{
	song_current = (soundGetCurrentSong()-1);
	if (song_current < 0)
	{
		song_current = song_i-1;
	}
	
	if (song_playing)
	{
		soundPlaySong(song_current);
	}
}

/**
 * Play next track in the playlist.  When rewinding last track the player will jump to the first track in the playlist.
 * @ingroup audio
 * @ref JSAPI
 */
void soundNextTrack(void)
{
	//printf("current_song:%d, song_i:%d, modulo:%d\n",soundGetCurrentSong(),song_i,(soundGetCurrentSong()+1) % song_i);
	song_current = (soundGetCurrentSong()+1) % song_i;

	if (song_playing)
	{
		soundPlaySong(song_current);
	}
}

/**
 * Get size of the playlist.
 * @return size of the playlist
 * @ingroup audio
 * @ref JSAPI
 */
int soundGetPlaylistSize(void)
{
	return song_i;
}

/**
 * Get the index of the current track.
 * @return Index of the current track in the playlist.
 * @ingroup audio
 * @ref JSAPI
 */
int soundGetCurrentSong(void)
{
	return song_current;
}

/**
 * Get length of the song.
 * @param song_number Index of the track in the playlist.
 * @return length of the song in seconds.
 * @ingroup audio
 * @ref JSAPI
 */
int soundGetSongLength(int song_number)
{
	return songs[song_number].length;
}

/**
 * Get file name of the track.
 * @param song_number Index of the track in the playlist.
 * @return file name of the track.
 * @ingroup audio
 * @ref JSAPI
 */
const char* soundGetSongFilename(int song_number)
{
	return songs[song_number].filename;
}

/**
 * Get song name of the track.
 * @param song_number Index of the track in the playlist.
 * @return song name of the track.
 * @ingroup audio
 * @ref JSAPI
 */
const char* soundGetSongName(int song_number)
{
	return songs[song_number].name;
}

/**
 * Get the track number.
 * @return Track index + 1
 * @ingroup audio
 * @ref JSAPI
 */
int soundGetTrackNumber(int song_number)
{
	return song_number+1;
}

/**
 * Check if audio is playing.
 * @return 1 if audio is playing, 0 if audio is not playing.
 * @ingroup audio
 * @ref JSAPI
 */
int soundIsPlaying(void)
{
	return song_playing;
}

/**
 * Check player status and rewind to next track in the playlist if the song has ended.
 * @return Track index + 1
 * @ingroup audio
 */
void soundCheckPlayer(void)
{
	if (soundIsPlaying() && soundGetSongLength(soundGetCurrentSong()) > 0)
	{
		song_play_time = timerGetTime()-song_play_time_start;

		if (song_play_time > (double)soundGetSongLength(soundGetCurrentSong())+0.4f)
		{
			soundNextTrack();
		}
	}
}

/**
 * Get the time how long the current track has played.
 * @return Play time in seconds.
 * @ingroup audio
 * @ref JSAPI
 */
double soundGetSongCurrentPlayTime(void)
{
	return song_play_time;
}

/**
 * Play sound.
 * @param song_number Index of the song to-be-played in the playlist.
 * @ingroup audio
 * @ref JSAPI
 */
void soundPlaySong(int song_number)
{
	song_number = song_number % song_i;

	debugPrintf("Playing: %02d: filename:'%s', title:'%s', length:'%d'", song_number, songs[song_number].filename, songs[song_number].name, songs[song_number].length);
		
#ifdef FMOD
	FMOD_RESULT f = FMOD_OK;
#endif

	if (song_playing)
	{
		soundStop();

		soundFree();
	}

	
	soundInit();

	soundLoadSong(song_number);

	song_playing = 1;

	song_current = song_number;

	char song_length_string[12];
	int currentMinute = (int)soundGetSongLength(soundGetCurrentSong()) / 60;
	int currentSecond = (int)soundGetSongLength(soundGetCurrentSong()) % 60;
	sprintf(song_length_string, "%02d:%02d", currentMinute, currentSecond);

	/*#define TITLE_SIZE 512
	char title[TITLE_SIZE] = {'\0'};
	snprintf(title, TITLE_SIZE, "%02d/%02d. %s (%s)", soundGetTrackNumber(song_current), soundGetPlaylistSize(), soundGetSongName(song_current), song_length_string);
	windowSetTitle(title);*/

#ifdef FMOD
	f = FMOD_System_PlaySound(fmod_system, FMOD_CHANNEL_FREE, songs[song_current].sound, 0, &channel);
	debugPrintf("FMOD System Play Sound: '%s'", FMOD_ErrorString(f));
	
	f = FMOD_Channel_SetPosition(channel, (unsigned int)(timePosition*1000), FMOD_TIMEUNIT_MS);
	debugPrintf("FMOD Channel Set Position: '%s'", FMOD_ErrorString(f));
#elif MINIFMOD
	FMUSIC_PlaySong(songs[song_current].sound);
#elif BASS
	if (!BASS_ChannelPlay(songs[song_current].sound,FALSE))
	{
		debugErrorPrintf("BASS StreamPlay error: '%d'", BASS_ErrorGetCode());
	}
	if (!BASS_ChannelSetPosition(songs[song_current].sound, BASS_ChannelSeconds2Bytes(songs[song_current].sound,timePosition), 0))
	{
		debugErrorPrintf("BASS ChannelSetPosition error: '%d'", BASS_ErrorGetCode());
	}
#elif SDL_MIXER
	songs[song_current].sound = Mix_LoadMUS(songs[song_current].filename);
	Mix_PlayMusic(songs[song_current].sound, loop);
#endif

	timerUpdate();
	song_play_time_start = timerGetTime();
}

/**
 * Clear the current playlist.
 * @ingroup audio
 * @ref JSAPI
 */
void soundClearPlaylist(void)
{
	int i;
	for (i = 0; i < song_i; i++)
	{
		if (songs[i].name)
		{
			free(songs[i].name);
		}
		if (songs[i].filename)
		{
			free(songs[i].filename);
		}
	}
	
	song_i = 0;
}

/**
 * Deinitialize the audio.
 * @ingroup audio
 */
void soundDeinit(void)
{
	soundFree();
	soundClearPlaylist();
	
	init_done = 0;
}
