#include <assert.h>

#include "graphicsIncludes.h"
#include "system/debug/debug.h"

#include "window.h"
#include "windowSdl.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

static fbo_t* fbo = NULL;
static int screenPowerOfTwoSize = 2048;
static int fullscreen = 0;
static int windowWidth = DEFAULT_WIDTH, windowHeight = DEFAULT_HEIGHT;
static int screenWidth = DEFAULT_WIDTH, screenHeight = DEFAULT_HEIGHT;
static int screenPositionX = 0, screenPositionY = 0;
static double windowScreenAreaAspectRatio = DEFAULT_WIDTH/(double)DEFAULT_HEIGHT;

void windowInit()
{
	assert(windowWidth > 0);
	assert(windowHeight > 0);
	assert(screenWidth > 0);
	assert(screenHeight > 0);

	
	if (screenWidth <= 1024)
	{
		screenPowerOfTwoSize = 1024;
	}
	else if (screenWidth > 2048)
	{
		screenPowerOfTwoSize = 4096;
	}

//#ifndef DEBUG

	if (getWindowScreenAreaAspectRatio() < 1.0)
	{
		debugWarningPrintf("Aspect ratio is funky!");
	}

	screenPositionY = (int)((getWindowHeight()-getWindowScreenAreaHeight())/2.0f);
	screenPositionX = (int)((getWindowWidth()-getWindowScreenAreaWidth())/2.0f);

//#endif

	debugPrintf("Window dimensions: %dx%d", getWindowWidth(), getWindowHeight());
	debugPrintf("Screen dimensions: %dx%d", getScreenWidth(), getScreenHeight());
	debugPrintf("Screen aspect: %.2f", getWindowScreenAreaAspectRatio());
	debugPrintf("Screen area: x:%d, y:%d, w:%d, h:%d", screenPositionX, screenPositionY, getWindowScreenAreaWidth(), getWindowScreenAreaHeight());
	//debugPrintf("Window info: screen dimensions:%dx%d, window dimensions:%dx%d, aspect:%.2f",screenWidth,screenHeight,windowWidth,windowHeight,aspectRatio);
#ifdef SDL
	windowInitSdl();
#endif
}

int getScreenPowerOfTwoSize(void)
{
	return screenPowerOfTwoSize;
}

int getScreenWidth(void)
{
	return screenWidth;
}

int getScreenHeight(void)
{
	return screenHeight;
}

int getWindowScreenAreaWidth(void)
{
	double screenAreaWidth = getWindowHeight() * getWindowScreenAreaAspectRatio();
	if (screenAreaWidth > getWindowWidth())
	{
		screenAreaWidth = getWindowWidth();
	}
	
	return screenAreaWidth+0.5;
}


int getWindowScreenAreaHeight(void)
{
	double screenAreaHeight = getWindowWidth()/getWindowScreenAreaAspectRatio();
	if (screenAreaHeight > getWindowHeight())
	{
		screenAreaHeight = getWindowHeight();
	}
	
	return screenAreaHeight+0.5;
}

void setScreenDimensions(int width, int height)
{
	screenWidth  = width;
	screenHeight = height;
}

void setWindowDimensions(int width, int height)
{
	windowWidth  = width;
	windowHeight = height;
}

double getWindowScreenAreaAspectRatio()
{
	return windowScreenAreaAspectRatio;
}

void setWindowScreenAreaAspectRatio(double width, double height)
{
	windowScreenAreaAspectRatio = width/height;
}

void setWindowFullscreen(int _fullscreen)
{
	fullscreen = _fullscreen;
}

int windowIsFullscreen(void)
{
	return fullscreen;
}

int getWindowWidth(void)
{
	return windowWidth;
}

int getWindowHeight(void)
{
	return windowHeight;
}

int getScreenPositionX(void)
{
	return screenPositionX;
}

int getScreenPositionY(void)
{
	return screenPositionY;
}

void setWindowFbo(fbo_t* _fbo)
{
	fbo = _fbo;
}

void resetViewport(void)
{
	if (!fbo)
	{
		glViewport(screenPositionX, screenPositionY, getWindowScreenAreaWidth(), getWindowScreenAreaHeight());
	}
	else
	{
#ifdef SUPPORT_GL_FBO
		glViewport(0, 0, fboGetWidth(fbo), fboGetHeight(fbo));
#endif
	}
}
