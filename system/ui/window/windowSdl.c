#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "graphicsIncludes.h"
#include "system/debug/debug.h"
#include "system/player/player.h"
#include "window.h"

static SDL_Surface *screen;
static int options;
#define TITLE_LENGTH 256
static char title[TITLE_LENGTH];

void windowInitSdl(void)
{
	options = SDL_OPENGL;

	SDL_Init(SDL_INIT_VIDEO); //|SDL_INIT_AUDIO);

	if (windowIsFullscreen())
	{
		options |= SDL_FULLSCREEN;

		if (!isPlayerEditor())
		{
			SDL_ShowCursor(SDL_DISABLE);
		}
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY/3, SDL_DEFAULT_REPEAT_INTERVAL);
}

void windowDeinit(void)
{
	debugPrintf("Deinitializing window");

	SDL_Quit();
}

int windowShow(void)
{
	screen = SDL_SetVideoMode(getWindowWidth(), getWindowHeight(), 0, options);
	return 1;
}

void windowSetTitle(const char *newTitle)
{
	strncpy(title, newTitle, TITLE_LENGTH);
	SDL_WM_SetCaption(title, 0);
}

void windowSetTitleTimer(const char *titleTimer)
{
	const int WHOLE_TITLE_STRING_LENGTH = TITLE_LENGTH + 64;
	char titleExtraInfo[WHOLE_TITLE_STRING_LENGTH];
	snprintf(titleExtraInfo, WHOLE_TITLE_STRING_LENGTH, "%s %s", title, titleTimer);
	SDL_WM_SetCaption(titleExtraInfo, 0);
}
