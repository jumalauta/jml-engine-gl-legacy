#define EDITOR_INPUT

#include "graphicsIncludes.h"
#include "system/debug/debug.h"
#include "system/javascript/javascript.h"
#include "system/graphics/shader/shader.h"
#include "system/player/player.h"

#include "input.h"

#ifdef ANTTWEAKBAR
#include <AntTweakBar/AntTweakBar.h>
#endif

static SDL_Event event;

#ifdef EDITOR_INPUT
#include "system/ui/window/window.h"
#include "system/timer/timer.h"
char buttonPressed=0, buttonReleased=0, ctrlPressed=0, shiftPressed=0, enterPressed=0, f1Pressed=0, f2Pressed=0, deletePressed=0, zPressed=0;
unsigned int lineWidth=3; //8
int mouseX=0, mouseY=0;
static const float WAIT_TIME = 0.2;
static float buttonWait = 0.0f, ctrlWait = 0.0f, shiftWait = 0.0f, enterWait = 0.0f, f1Wait = 0.0f, f2Wait = 0.0f, deleteWait = 0.0f, zWait = 0.0f;
int lineRedraw = 4;
#endif

#ifndef NDEBUG
static int screenLog = 0;
#endif

static int exitPending = 0;

void inputGetMouseState(int* x, int* y)
{
	*x = mouseX;
	*y = mouseY;
}

int isUserExit(void)
{
	if (exitPending)
	{
		return 1;
	}

#ifdef EDITOR_INPUT
	float currentTime = timerGetTime();
	SDL_GetMouseState(&mouseX, &mouseY);
	mouseY = (getWindowHeight()-mouseY);
#endif
	enterPressed = 0;
//	buttonPressed = 0;
	buttonReleased = 0;
	deletePressed = 0;
	f1Pressed = f2Pressed = 0;
	zPressed = 0;

	while (SDL_PollEvent(&event))
	{
#ifdef ANTTWEAKBAR
		TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
#endif

#ifdef JAVASCRIPT
		if (jsIsUseInput())
		{
			jsAddInputEvent(&event);
		}
#endif

		if ((event.type == SDL_QUIT)
			|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
		{
			debugPrintf("User requested exit.");
			exitPending = 1;
			return 1;
		}

#ifndef NDEBUG
		if (event.type == SDL_KEYDOWN
			&& event.key.state == SDL_PRESSED)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					ctrlPressed = 1;
					break;
				case SDLK_RSHIFT:
				case SDLK_LSHIFT:
					shiftPressed = 1;
					break;
				case SDLK_TAB:
					if (isPlayerEditor())
					{
						screenLog = !screenLog;
						playerShowScreenLog(screenLog);
					}
					break;
				case SDLK_PAGEUP:
					break;
				case SDLK_PAGEDOWN:
					break;
				case SDLK_HOME:
					if (ctrlPressed && isPlayerEditor())
					{
						timerSetTime(0);
						playerForceRedraw();
					}
					break;
				case SDLK_END:
					if (ctrlPressed && isPlayerEditor())
					{
						if (!timerIsPause())
						{
							timerPause();
						}

						timerSetTime(timerGetEndTime() - 0.1f);
						playerForceRedraw();
					}
					break;
				case SDLK_1:
					if (ctrlPressed && isPlayerEditor())
					{
						timerAddTime(-1);
						playerForceRedraw();
					}
					break;
				case SDLK_2:
					if (ctrlPressed && isPlayerEditor())
					{
						timerAddTime(1);
						playerForceRedraw();
					}
					break;
				case SDLK_3:
					if (ctrlPressed && isPlayerEditor())
					{
						timerPause();
					}
					break;
#ifdef JAVASCRIPT
				case SDLK_5:
					if (ctrlPressed && isPlayerEditor())
					{
						int fullRefresh = shiftPressed;
						setPlayerRefreshRequest(fullRefresh);
					}
					
					break;
#endif
				case SDLK_p:
					if (ctrlPressed && isPlayerEditor())
					{
						char filename[128];
						sprintf(filename, "demo_ss_%d.png", SDL_GetTicks());
						imageTakeScreenshot((const char*)filename);
					}
					
					break;
				default:
					break;
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					ctrlPressed = 0;
					break;
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					shiftPressed = 0;
					break;
				default:
					break;
			}
		}
#endif

#ifdef EDITOR_INPUT
		switch (event.type)
		{
			case SDL_MOUSEBUTTONUP:
				buttonPressed = 0;
				buttonReleased = 1;
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (buttonWait+0.1 < currentTime)
				{

					buttonPressed = 1;
					buttonWait = currentTime;
				}
				break;

		}

		if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
		{
			if (event.key.keysym.sym == SDLK_RETURN)
			{
				if (enterWait+0.25f < currentTime)
				{
					enterPressed = 1;
					enterWait = currentTime;
				}
			}

			if ((event.key.keysym.sym == SDLK_LCTRL) || (event.key.keysym.sym == SDLK_RCTRL))
			{
				if (ctrlWait+WAIT_TIME < currentTime)
				{
					ctrlPressed = 1;
					ctrlWait = currentTime;
				}
				if (event.type == SDL_KEYUP) { ctrlPressed = 0; }
			} else if ((event.key.keysym.sym == SDLK_LSHIFT) || (event.key.keysym.sym == SDLK_RSHIFT))
			{
				if (shiftWait+WAIT_TIME < currentTime)
				{
					shiftPressed = 1;
					shiftWait = currentTime;
				}
				if (event.type == SDL_KEYUP) { shiftPressed = 0; }
			}
		}

		if (event.type == SDL_KEYUP)
			switch(event.key.keysym.sym)
		{
			case SDLK_PAGEUP:
				lineWidth++;
				break;
			case SDLK_PAGEDOWN:
				lineWidth--;
				break;
			case SDLK_1:
				lineRedraw = 4*1;
				break;
			case SDLK_2:
				lineRedraw = 4*2;
				break;
			case SDLK_3:
				lineRedraw = 4*3;
				break;
			case SDLK_4:
				lineRedraw = 4*4;
				break;
			case SDLK_5:
				lineRedraw = 4*5;
				break;
			case SDLK_6:
				lineRedraw = 4*6;
				break;
			case SDLK_7:
				lineRedraw = 4*7;
				break;
			case SDLK_8:
				lineRedraw = 4*8;
				break;
			case SDLK_9:
				lineRedraw = 4*9;
				break;
			case SDLK_0:
				lineRedraw = 4*10;
				break;
			case SDLK_F1:
				if (f1Wait+WAIT_TIME < currentTime)
				{
					f1Pressed = 1;
					f1Wait = currentTime;
				}
				break;
			case SDLK_F2:
				if (f2Wait+WAIT_TIME < currentTime)
				{
					f2Pressed = 1;
					f2Wait = currentTime;
				}
				break;
			case SDLK_z:
				if (zWait+WAIT_TIME < currentTime)
				{
					zPressed = 1;
					zWait = currentTime;
				}
				break;
			case SDLK_DELETE:
				if (deleteWait+WAIT_TIME < currentTime)
				{
					deletePressed = 1;
					deleteWait = currentTime;
				}
				break;
			default:
				break;
		}

#endif
	}

	return 0;
}

