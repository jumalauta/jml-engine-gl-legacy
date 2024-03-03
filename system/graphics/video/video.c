#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <graphicsIncludes.h>

#include "video.h"

#include "system/graphics/video/theoraplay/theoraplay.h"
#include "system/debug/debug.h"
#include "system/datatypes/datatypes.h"
#include "system/datatypes/memory.h"
#include "system/timer/timer.h"
#include "system/io/io.h"
#include "system/ui/input/input.h"
#include "system/ui/window/window.h"



#define ALLOC_STEP_SIZE 256

//dynamically add pointer to array of pointers. array will be extended if there's not enough size
#define add_to_array(type, array, element) \
	assert(element); \
	array##Size++; \
	if (array##Size%ALLOC_STEP_SIZE == 1) \
	{ \
		array = (type**)realloc(array, sizeof(type*)*(array##Size+ALLOC_STEP_SIZE)); \
		assert(array); \
	} \
	array[array##Size-1] = element

//reserve memory only up to array##Size variable, i.e. free the ALLOC_STEP_SIZE's leftovers
#define realloc_to_actual_size(type, array) \
	assert(array); \
	array = (type**)realloc(array, sizeof(type*)*(array##Size)); \
	assert(array)

//static Uint32 baseticks = 0;

typedef struct AudioQueue
{
	const THEORAPLAY_AudioPacket *audio;
	int offset;
	struct AudioQueue *next;
} AudioQueue;

static volatile AudioQueue *audio_queue = NULL;
static volatile AudioQueue *audio_queue_tail = NULL;

static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len)
{

	if (userdata)
	{
		//DO NOTHING
	}
	

	// !!! FIXME: this should refuse to play if item->playms is in the future.
	//const Uint32 now = SDL_GetTicks() - baseticks;
	Sint16 *dst = (Sint16 *) stream;

	while (audio_queue && (len > 0))
	{
		volatile AudioQueue *item = audio_queue;
		AudioQueue *next = item->next;
		const int channels = item->audio->channels;

		const float *src = item->audio->samples + (item->offset * channels);
		int cpy = (item->audio->frames - item->offset) * channels;
		int i;

		if (cpy > (int)(len / sizeof (Sint16)))
			cpy = len / sizeof (Sint16);

		for (i = 0; i < cpy; i++)
		{
			const float val = *(src++);
			if (val < -1.0f)
				*(dst++) = -32768;
			else if (val > 1.0f)
				*(dst++) = 32767;
			else
				*(dst++) = (Sint16) (val * 32767.0f);
		} // for

		item->offset += (cpy / channels);
		len -= cpy * sizeof (Sint16);

		if (item->offset >= item->audio->frames)
		{
			THEORAPLAY_freeAudio(item->audio);
			free((void *) item);
			audio_queue = next;
		} // if
	} // while

	if (!audio_queue)
		audio_queue_tail = NULL;

	if (len > 0)
		memset(dst, '\0', len);
} // audio_callback


static void queue_audio(const THEORAPLAY_AudioPacket *audio)
{
	AudioQueue *item = (AudioQueue *) malloc(sizeof (AudioQueue));
	assert(item);

	item->audio = audio;
	item->offset = 0;
	item->next = NULL;

	SDL_LockAudio();
	if (audio_queue_tail)
		audio_queue_tail->next = item;
	else
		audio_queue = item;
	audio_queue_tail = item;
	SDL_UnlockAudio();
} // queue_audio

typedef struct video_theora_frame_t {
	const THEORAPLAY_VideoFrame *video;
	const THEORAPLAY_AudioPacket *audio;
	long int fileCursor;
} video_theora_frame_t;

typedef struct video_theora_t {
	THEORAPLAY_Decoder *decoder;
	video_theora_frame_t *currentFrame;
	SDL_AudioSpec audioSpec;
} video_theora_t;


#define CODEC_NULL 0
#define CODEC_THEORA 1

static void freeFrameTheora(video_theora_frame_t **frame);
//static int freeVideoFramesTheora(video_t *video);

#define MAXFRAMES 0xFFFF

static int loadVideoTheoraFrame(video_t *video, double time)
{
	assert(video);
	assert(video->codec);
	video_theora_t *codec = (video_theora_t*)video->codec;

	if (time > 0.0 && timerIsAddTimeGracePeriod())
	{
		return 1;
	}

	if (codec->currentFrame != NULL && codec->currentFrame->video->playms/1000.0 > time+0.5)
	{
		THEORAPLAY_stopDecode(codec->decoder);
		codec->decoder = NULL;
	}

	if (codec->decoder == NULL)
	{
		if (codec->decoder)
		{
			THEORAPLAY_stopDecode(codec->decoder);
		}

		debugPrintf("Start decode again '%s'",video->filename);
		codec->decoder = THEORAPLAY_startDecodeFile(video->filename, MAXFRAMES, THEORAPLAY_VIDFMT_RGB);
		if (!codec->decoder)
		{
			debugErrorPrintf("Could not decode file! '%s'");
			return 0;
		}

		//freeVideoFramesTheora(video);

		//THEORAPLAY_setDecodeTime(codec->decoder, 0.0);
	}

	//unsigned int frame = time*video->fps;
	THEORAPLAY_setDecodeTime(codec->decoder, time);

	//loadVideoTheoraFrame(video, 0);

	//const unsigned int MAX_PLAY_TIME_MS = 30000;
	const unsigned int MAX_DECODE_WAIT_MS = 1000;

	//debugPrintf("Getting frame %d (%.2f) '%s'!", frame, frame/video->fps, video->filename);

	//debugPrintf("=> Getting frame %d/%d!", frame, codec->framesSize);
	unsigned int loadStart = SDL_GetTicks();
	const THEORAPLAY_VideoFrame *video_frame = NULL;
	
	int pauseWait = 0;
	while(1)
	{
		video_frame = THEORAPLAY_getVideo(codec->decoder);

		unsigned int wait_sum = SDL_GetTicks()-loadStart;
		if (wait_sum > MAX_DECODE_WAIT_MS && pauseWait == 0)
		{
			pauseWait = 1;
			debugWarningPrintf("Tried to decode video over %d ms. Slowness... video:%s", wait_sum, video->filename);

			if (!timerIsPause())
			{
				pauseWait = 2;
				timerAddTime(-(wait_sum/1000.0));
				timerPause();
			}
		}

		if (video_frame != NULL)
		{
			if (video_frame->playms/1000.0 >= time)
			{
				//debugPrintf("Using frame! time:%.3f, frameTime:%.3f",time,video_frame->playms/1000.0);
				break;
			}
			else
			{
				//debugPrintf("Skipping frame! time:%.3f, frameTime:%.3f",time,video_frame->playms/1000.0);
				THEORAPLAY_freeVideo(video_frame);	//discard frame => will not be used
			}
		}

		if (!THEORAPLAY_isDecoding(codec->decoder))
		{
			return 1;
		}		
	}

	if (pauseWait == 2)
	{
		if (timerIsPause())
		{
			timerPause();
		}
	}

	assert (video_frame != NULL);

	video_theora_frame_t *frame = (video_theora_frame_t*)malloc(sizeof(video_theora_frame_t));
	assert(frame);
	frame->audio = NULL;
	frame->video = video_frame;
	frame->fileCursor = frame->video->filecursor;
	//add_to_array(video_theora_frame_t, codec->frames, frame);


	/*if (frame->video->playms >= MAX_PLAY_TIME_MS)
	{
		debugWarningPrintf("Maximum video length is %.2f seconds. Video truncated to maximum length. video:%s", MAX_PLAY_TIME_MS/1000.0f, video->filename);
		return 0;
	}*/

	while ((frame->audio = THEORAPLAY_getAudio(codec->decoder)) != NULL)
	{
		THEORAPLAY_freeAudio(frame->audio);	
		frame->audio = NULL;
	}

	//TODO: HOW TO APPROPRIATELY HANDLE AUDIO???
	//while ((frame->audio = THEORAPLAY_getAudio(codec->decoder)) != NULL)
	//{
	//	queue_audio(frame->audio);
	//}

	if (codec->currentFrame != NULL && codec->currentFrame != frame)
	{
		freeFrameTheora(&codec->currentFrame);
	}
	codec->currentFrame = frame;

	assert(codec->currentFrame->video != NULL);
	//debugPrintf("Loaded frame '%s'! dimensions:%dx%d, fps:%.2f, length:%.2f, time:%.3f", video->filename, codec->currentFrame->video->width, codec->currentFrame->video->height, codec->currentFrame->video->fps, codec->currentFrame->video->playms/1000.0, time);

	return 1;
}

static int loadVideoTheora(video_t *video)
{
	assert(video);
	assert(video->codec);
	video_theora_t *codec = (video_theora_t*)video->codec;

	loadVideoTheoraFrame(video, 0.0);

	int isInitialized = THEORAPLAY_isInitialized(codec->decoder);
	int hasAudio = THEORAPLAY_hasAudioStream(codec->decoder);
	int hasVideo = THEORAPLAY_hasVideoStream(codec->decoder);

	/*if (THEORAPLAY_isDecoding(codec->decoder))
	{
		THEORAPLAY_stopDecode(codec->decoder);
	}*/

	if (!isInitialized || !hasVideo)
	{
		debugErrorPrintf("Could not initialize video! '%s'");
		return 0;
	}

	//realloc_to_actual_size(video_theora_frame_t, codec->frames);

	if (video->useAudio && codec->currentFrame->audio)
	{
		debugPrintf("Audio stuff...");
		memset(&codec->audioSpec, '\0', sizeof (SDL_AudioSpec));
		codec->audioSpec.freq = codec->currentFrame->audio->freq;
		codec->audioSpec.format = AUDIO_S16SYS;
		codec->audioSpec.channels = codec->currentFrame->audio->channels;
		codec->audioSpec.samples = 2048;
		codec->audioSpec.callback = audio_callback;
		if (SDL_OpenAudio(&codec->audioSpec, NULL) < 0)
		{
			debugErrorPrintf("Audio open failed! error:'%s'", SDL_GetError());
			video->useAudio = 0;
		}
		else
		{
			while (codec->currentFrame->audio)
			{
				queue_audio(codec->currentFrame->audio);
				codec->currentFrame->audio = THEORAPLAY_getAudio(codec->decoder);
			}
		}
	}

	video->w = codec->currentFrame->video->width;
	video->h = codec->currentFrame->video->height;
	video->fps = codec->currentFrame->video->fps;

	debugPrintf("Loaded video '%s'! dimensions:%dx%d, fps:%.2f, audio:%s, video:%s", video->filename, video->w, video->h, video->fps,hasAudio?"true":"false", hasVideo?"true":"false");

	return 1;
}

static int playVideoTheora(video_t *video)
{
	assert(video);

	return 1;
}

static int pauseVideoTheora(video_t *video)
{
	assert(video);

	return 1;
}

static int stopVideoTheora(video_t *video)
{
	assert(video);

	return 1;
}

static void freeFrameTheora(video_theora_frame_t **frame)
{
	assert(*frame);

	if ((*frame)->video)
	{
		THEORAPLAY_freeVideo((*frame)->video);
		(*frame)->video = NULL;
	}
	if ((*frame)->audio)
	{
		THEORAPLAY_freeAudio((*frame)->audio);
		(*frame)->audio = NULL;
	}

	free(*frame);
}

static int freeVideoTheora(video_t *video)
{
	assert(video);
	assert(video->codec);

	video_theora_t *codec = (video_theora_t*)video->codec;

	//freeVideoFramesTheora(video);
	if (codec->currentFrame)
	{
		freeFrameTheora(&codec->currentFrame);
	}

	THEORAPLAY_stopDecode(codec->decoder);

	free(codec);
	video->codec = NULL;

	return 1;
}

#define VIDEO_STOPPED 0
#define VIDEO_PLAYING 1
#define VIDEO_PAUSED 2

video_t* videoInit(video_t *video)
{
	if (video == NULL)
	{
		video = (video_t*)malloc(sizeof(video_t));
		assert(video);
	}

	video->filename = NULL;
	video->w = video->h = video->frame = 0;
    video->speed = 1.0;
	video->fps = 0.0;
	video->paused = 0;
	video->frameTexture = NULL;
	video->codecType = CODEC_NULL;
	video->codec = NULL;
	video->loadVideo = NULL;
	video->playVideo = NULL;
	video->pauseVideo = NULL;
	video->stopVideo = NULL;
	video->freeVideo = NULL;
	video->useAudio = 0;
	video->loop = 0;
	video->startTime = 0.0f;
	video->pauseTime = 0.0f;
	video->length = 0.0f;
	video->currentFrame = 0;
	video->state = VIDEO_STOPPED;

	return video;
}

void videoDeinit(video_t *video)
{
	assert(video);
	assert(video->filename);

	if (video->freeVideo)
	{
		video->freeVideo(video);
	}

	free(video->filename);
}


video_t *videoLoad(const char *filename)
{
	char *fullFilename = strdup((char*)getFilePath(filename));
	video_t* video = getVideoFromMemory(fullFilename);
	if (video)
	{
		free(fullFilename);
		return video;
	}

	video = memoryAllocateVideo(NULL);
	video = videoInit(video);

	video->filename = fullFilename;
	if (endsWithIgnoreCase(video->filename, ".ogv") || endsWithIgnoreCase(video->filename, ".ogg"))
	{
		assert(video->codec == NULL);
		video->codecType = CODEC_THEORA;
		video->codec = (void*)malloc(sizeof(video_theora_t));
		assert(video->codec);
		video->loadVideo = loadVideoTheora;
		video->playVideo = playVideoTheora;
		video->pauseVideo = pauseVideoTheora;
		video->stopVideo = stopVideoTheora;
		video->freeVideo = freeVideoTheora;

		video_theora_t *codec = (video_theora_t*)video->codec;
		codec->currentFrame = NULL;
		codec->decoder = NULL;
	}
	else
	{
		debugErrorPrintf("Video format not recognized! file:'%s'", video->filename);
		return NULL;
	}

	if (video->loadVideo)
	{
		if (!video->loadVideo(video))
		{
			debugErrorPrintf("Could not load video '%s'!", video->filename);
		}
	}

	size_t stringLength = strlen(video->filename)+7;
	char *textureName = (char*)malloc(stringLength*sizeof(char));
	snprintf(textureName, stringLength, "%s.frame", video->filename);
	video->frameTexture = imageCreateTexture((const char*)textureName, 0, GL_RGB, video->w, video->h);
	free(textureName);
	setTextureCenterAlignment(video->frameTexture, 1);
	setTextureUvDimensions(video->frameTexture, 0.0, 0.0, 1.0, -1.0); //THEORA frames are upside down

	return video;
}

void videoSetSpeed(video_t *video, double speed)
{
    if (speed < 0.0)
    {
        debugErrorPrintf("Invalid value given! video:'%s', invalidSpeedValue:%f", video->filename, speed);
        return;
    }

    video->speed = speed;
}

void videoSetFps(video_t *video, double fps)
{
	if (fps < 0.0)
	{
		debugErrorPrintf("Invalid value given! video:'%s', invalidFpsValue:%f", video->filename, fps);
		return;
	}

	video->fps = fps;
}

void videoSetLoop(video_t *video, int loop)
{
	if (loop < 0 || loop > 1)
	{
		debugErrorPrintf("Invalid value given! video:'%s', invalidLoopValue:%d", video->filename, loop);
		return;
	}

	if (loop == 1)
	{
		debugErrorPrintf("Video looping not supported!");
		assert(loop != 1);
	}

	video->loop = loop;
}

void videoPlay(video_t *video)
{
	assert(video);

	if (video->useAudio)
	{
		debugErrorPrintf("Video audio not supported currently! filename:'%s'", video->filename);
		assert(video->useAudio == 0);
		return;
	}

	video->state = VIDEO_PLAYING;

	//debugPrintf("Video '%s' started playing! startTime:%.2f, state:%d", video->filename, video->startTime, video->state);
}

void videoSetStartTime(video_t *video, float startTime)
{
    assert(video);

    if (startTime < 0.0)
    {
        debugErrorPrintf("Invalid value given! video:'%s', invalidStartTimeValue:%f", video->filename, startTime);
        return;
    }

    video->startTime = startTime;
}

void videoSetTime(video_t *video, float time)
{
	assert(video);

	if (video->state != VIDEO_PLAYING)
	{
		debugWarningPrintf("You can't rewind video '%s' when it's not playing!", video->filename);
		return;
	}

	video->startTime = timerGetTime() - time;
}

void videoStop(video_t *video)
{
	assert(video);
	video->state = VIDEO_STOPPED;
	video->startTime = 0.0f;
}

void videoPause(video_t *video)
{
	assert(video);
	if (video->state == VIDEO_PLAYING)
	{
		video->state = VIDEO_PAUSED;
		video->pauseTime = timerGetTime();
	}
	else
	{
		video->state = VIDEO_PLAYING;
		video->startTime += timerGetTime()-video->pauseTime;
		video->pauseTime = 0.0f;
	}
}

static void videoRefreshFrame(video_t *video)
{
	assert(video);

	assert(video->codecType == CODEC_THEORA);
	video_theora_t *codec = (video_theora_t*)video->codec;
	const THEORAPLAY_VideoFrame *videoFrame = codec->currentFrame->video;
	assert(videoFrame);

	const unsigned int w = videoFrame->width;
	const unsigned int h = videoFrame->height;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, video->frameTexture->id);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, (const void *)videoFrame->pixels);

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void videoDraw(video_t *video)
{
	assert(video);
	assert(video->codec);

	assert(video->codecType == CODEC_THEORA);
	video_theora_t *codec = (video_theora_t*)video->codec;

	float runningTime = timerGetTime()-video->startTime;
    if (runningTime < 0.0)
    {
		if (video->state == VIDEO_PLAYING && codec->decoder)
		{
			THEORAPLAY_stopDecode(codec->decoder);
			codec->decoder = NULL;
		}

        return;
    }

    if (video->loop == 1 && video->length > 0.0)
    {
		runningTime = fmodf(runningTime, video->length);
    }

	unsigned int frame_i = video->currentFrame;
	
	if (video->state == VIDEO_PLAYING)
	{
		frame_i = (unsigned int)(video->fps*runningTime*video->speed);
	}
	//debugPrintf("Running! %.2f frame:%d (%d), state:%d", runningTime, frame_i, video->currentFrame, video->state);
	/*if (frame_i >= codec->framesSize)
	{
		return;
	}*/

	if (video->currentFrame == frame_i && runningTime > 0.0f)
	{
		return; //assume that frame has been buffered
	}
	video->currentFrame = frame_i;
	loadVideoTheoraFrame(video, runningTime*video->speed);

	//codec->currentFrame->video = codec->frames[video->currentFrame]->video;

	if (codec->currentFrame->video)
	{
		videoRefreshFrame(video);
	}

	if (codec->currentFrame->audio)
	{
		while ((codec->currentFrame->audio = THEORAPLAY_getAudio(codec->decoder)) != NULL)
			queue_audio(codec->currentFrame->audio);
	}
}
