#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "version.h"
#include "timer.h"
#include "system/ui/window/window.h"
#include "system/audio/sound.h"
#include "system/debug/debug.h"
#include "effects/playlist.h"
#include "system/rocket/synceditor.h"

/**
 * @defgroup timer Global time handling functionality
 */

#ifdef SDL
#include <SDL/SDL.h>
#elif WINDOWS
#include <windows.h>
static double tps;
#endif


static double stime=0,ctime,ltime,endTime;
static double deltaspeed;

static double fpsCorrection = 1.0f;

static int endMinute = 0;
static int endSecond = 0;

static double timerPauseTime = 0.0f;
static int timerPaused = 0;

static double targetFps = 500.0;
void timerSetTargetFps(double fps)
{
	targetFps = fps;
}

double timerGetTargetFps()
{
	return targetFps;
}

double timerGetSeconds(void)
{
#ifdef SDL
        return SDL_GetTicks()/1000.0;
#elif WINDOWS
        return timeGetTime()/1000.0;
#endif
}

#ifndef NDEBUG
#define TITLE_SIZE 64
static int frames=0;
static double oldTime=1.0f;

static void timerGetFps(void)
{
	frames++;
	if (fabs(ctime - oldTime) >= 0.5f)
	{
		//debugPrintf("frames:'%d', ctime:'%f', oldTime:'%f'",frames,ctime,oldTime);
		double fps = frames / (ctime - oldTime);
		fpsCorrection = timerGetTargetFps()/fps;

		if (isDebug())
		{
			//FPS is displayed in the window title
			char title[TITLE_SIZE];
			int currentMinute = (int)ctime / 60;
			int currentSecond = (int)ctime % 60;

			snprintf(title, TITLE_SIZE, "v%s - Time: %d:%02d/%d:%02d FPS: %.f",
					DEMO_ENGINE_VERSION_STRING,
					currentMinute, currentSecond,
					endMinute, endSecond,
					fps);
			windowSetTitleTimer(title);
		}

		oldTime = ctime;
		frames = 0;
	}
}
#endif

#if defined(WINDOWS) && !defined(SDL)
static double windowsAdditionalTime = 0.0f;
#endif

void timerInit(double newEndTime)
{
	endMinute = (int)newEndTime / 60;
	endSecond = (int)newEndTime % 60;

	endTime = newEndTime;

	stime = timerGetSeconds();
}

static void timerSetCurrentTime(void)
{
	if (timerPaused)
	{
		int pauseDelay = 100;
		if (isSyncEditor())
		{
			pauseDelay = 5;
		}

		timerSleep(pauseDelay);

		return;
	}

#ifdef SDL
	ctime = timerGetSeconds() - stime;
	deltaspeed = (ctime - ltime);
#elif WINDOWS
	ltime = ctime;
	ctime = timerGetSeconds() - stime + windowsAdditionalTime;		
	deltaspeed = (ctime - ltime);
#endif

	if (ctime < 0)
	{
		ctime = 0;
	}
}

void timerUpdate(void)
{
	timerSetCurrentTime();

	timerGetFps();
}

static void timerAddTimeWithoutSound(double at)
{
	if (timerPaused)
	{
		timerPauseTime += at;
		ctime += at;
	}

#ifdef SDL
	stime -= at;
#elif WINDOWS
	stime -= at;
	windowsAdditionalTime = at;
#endif
	timerUpdate();
}

static double timerGrace = -1.0;

int timerIsAddTimeGracePeriod()
{
	const double REWIND_GRACE_PERIOD = 0.25;
	if (timerGrace > REWIND_GRACE_PERIOD && timerGetTime()-timerGrace < REWIND_GRACE_PERIOD)
	{
		return 1;
	}

	return 0;
}

void timerAddTime(double at)
{
	//timer must not go to negative
	if (timerGetTime()+at < 0)
	{
		at = at+fabs(timerGetTime()+at);
	}

	timerAddTimeWithoutSound(at);
	soundSetPosition(timerGetTime());

	timerGrace = timerGetTime();
}

void timerSetTime(double time)
{
	timerAddTime(time - timerGetTime());
}

int timerIsPause()
{
	return timerPaused;
}

void timerPause()
{
	if (timerPaused)
	{
		timerPaused = 0;
	}
	else
	{
		timerPaused = 1;
	}

	if (timerPaused)
	{
		timerPauseTime = timerGetTime();
		debugPrintf("Demo paused.");
	}
	else
	{
		timerSetCurrentTime();
		double pauseRemovalTime = timerGetTime();

		timerAddTimeWithoutSound(timerPauseTime-pauseRemovalTime);

		debugPrintf("Demo unpaused.");
	}

	soundPause();
}

static double bpm = 120.0;

/**
 * Get average BPM
 * @return average beats per minute
 * @ingroup timer
 * @ref JSAPI
 */
double timerGetBeatsPerMinute(void)
{
	return bpm;
}

/**
 * Set average BPM
 * @param _bpm [in] beats per minute
 * @ingroup timer
 * @ref JSAPI
 */
void timerSetBeatsPerMinute(double _bpm)
{
	bpm = _bpm;
}

/**
 * Get beat in seconds
 * @return amount of seconds of single beat
 * @ingroup timer
 * @ref JSAPI
 */
double timerGetBeatInSeconds(void)
{
	return 60.0/timerGetBeatsPerMinute();
}

/**
 * Get beat from playing time according to average BPM
 * @return beat from playing time according to average BPM
 * @ingroup timer
 * @ref JSAPI
 */
double timerGetCurrentBeat(void)
{
	return timerGetTime()/timerGetBeatInSeconds();
}

double timerGetTime(void)
{
	return ctime;
}

double timerGetEndTime(void)
{
	return endTime;
}

double timerGetDs(void)
{
	return deltaspeed;
}

int timerIsEnd(void)
{
	//-1.0 = endless
	if (endTime == -1.0f)
	{
		return 0;
	}

	if (ctime >= endTime)
	{
		return 1;
	}

	return 0;
}

void timerAdjustFramerate(void)
{
	double delayTime = ctime;
	timerSetCurrentTime();
	int milliDelayTime = (ctime-delayTime)*1000;
	const double sleepDelay = 1000/timerGetTargetFps();

	if (milliDelayTime < sleepDelay)
	{
		timerSleep(sleepDelay-milliDelayTime);
	}
}

double timerGetFpsCorrection(void)
{
	return fpsCorrection;
}

timerCounter_t *timerCounterStart(const char *counterName)
{
	timerCounter_t* counter = (timerCounter_t*)malloc(sizeof(timerCounter_t));
	assert(counter);

	counter->name = strdup(counterName);
	counter->time = timerGetSeconds();

	return counter;
}

double timerCounterEnd(timerCounter_t *counter)
{
	double duration = timerGetSeconds() - counter->time;
	debugPrintf("'%s' duration: %.3f seconds", counter->name, duration);

	if (counter->name)
	{
		free(counter->name);
	}
	free(counter);

	return duration;
}

void timerSetTimeString(char* timeString)
{
	assert(timeString);

	double currentTime = timerGetTime();
	int currentMinute = (int)currentTime / 60;
	int currentSecond = (int)currentTime % 60;

	//get the currentTime variable's decimal values
	char currentMillisString[4];
	snprintf(currentMillisString, 4, "%03d", (int)((currentTime-floor(currentTime))*1000));
	
	sprintf(timeString, "%d:%02d.%s", currentMinute, currentSecond, currentMillisString);
}

//converts time string that is in format [minutes]:[seconds].[milliseconds] to doubleing point
//returns the time string converted to seconds
double convertTimeToSeconds(const char *time)
{
	if (time == NULL || !strcmp(time, "N/A"))
	{
		return -1.0f;
	}

	int min      = 0;
	int sec      = 0;
	
	//Don't use char[], causes GCC compiler warning
	char *millis = NULL;
	millis = (char*)malloc(8);
	millis[0] = '\0';

	int substring = 0;
	if (time[0] == '#')
	{
		substring = 1;
	}

	sscanf(time+substring, "%9d:%2d%7s", &min, &sec, millis);
	
	if (millis[0] != '\0' && millis[0] != '.')
	{
		free(millis);
		debugPrintf("Function convertTimeToSeconds(%%s) failed: time \"%s\" is not in format %%d:%%d.%%d", time);
		return -2.0f;
	}

	double milliseconds = atof(millis);
	free(millis);

	return min*60+sec+milliseconds;	
}

void convertSecondsToTime(double seconds, char *time)
{
	assert(time);

	int min          = (int)(seconds/60.0f);
	int sec          = (int)(seconds-min*60);
	int milliseconds = (int)((seconds-floorf(seconds))*1000);
	
	snprintf(time, 16, "%d:%02d.%03d",min,sec,milliseconds);
}

void convertSecondsToAbsoluteTime(double seconds, char *time)
{
	assert(time);

	char tempTime[16];
	convertSecondsToTime(seconds, tempTime);
	
	snprintf(time, 16, "#%s", tempTime);
}

void timerSleep(int millis)
{
#ifdef SDL
	SDL_Delay(millis);
#elif WINDOWS
	sleep(millis);
#endif
}
