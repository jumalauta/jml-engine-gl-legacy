#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/graphics/object/lighting.h"
#include "system/debug/debug.h"
#include "system/timer/timer.h"
#include "system/thread/thread.h"
#include "system/datatypes/datatypes.h"
#include "system/extensions/gl/gl.h"
#include "system/ui/input/input.h"
#include "system/ui/window/window.h"
#include "system/io/io.h"
#include "system/javascript/javascript.h"
#include "system/datatypes/memory.h"
#include "effects/playlist.h"

#include "player.h"

#define PLAYER_NOT_INITIALIZED 0
#define PLAYER_INITIALIZING 1
#define PLAYER_LOADING 2
#define PLAYER_RUNNING 3

#define EFFECT_TYPE_C 0
#define EFFECT_TYPE_JS 1
#define EFFECT_TYPE_SHADER 2

#ifdef ANTTWEAKBAR
#include <AntTweakBar/AntTweakBar.h>
#include <duktape.h>

extern duk_context *ctx;

static void writeFile(const char *filename, const char *data) {
	FILE *f = fopen(filename, "wb");
	if (f)
	{
		fwrite(data, sizeof(char), strlen(data), f);
		fclose(f);
	}
}

static void TW_CALL saveScriptCallback(void *clientData)
{ 
	//write demoscript back to file
	duk_eval_string(ctx, "JSON.stringify(demoScript, null, 2)");
	//debugPrintf("=> %s", duk_get_string(ctx, -1));
	writeFile("data/js/script.json", duk_get_string(ctx, -1));
	duk_pop(ctx);
}
#endif

static int playerAutoClear = 1;
void setPlayerAutoClear(int _playerAutoClear) {
	playerAutoClear = _playerAutoClear;
}

static int playerState = PLAYER_NOT_INITIALIZED;

//populates scene->time structure sceneTime_t with time information
static void populateSceneTime(playerScene *scene)
{
	double currentTime        = timerGetTime();
	//populate time values
	scene->time.absolute     = currentTime;
	scene->time.percent      = (currentTime - scene->start) / (scene->end - scene->start);
	scene->time.start        = scene->start;
	scene->time.end          = scene->end;
	scene->time.now          = scene->time.end - scene->time.absolute;
	scene->time.interval     = 0.0f;
	scene->time.triggerStart = 0.0f;
	scene->time.triggerEnd   = 0.0f;
	//printf("%s %f %f %f %f %f %f\n",scene->name, scene->time.percent, scene->time.absolute,scene->time.percent,scene->time.start,scene->time.end,scene->time.now);
}

static playerEffect *playerEffectTail = NULL;
static playerEffect *playerEffectHead = NULL;
static playerEffect *playerEffectPlayerHead = NULL;
//static playerEffect *playerEffectCurrent = NULL;

playerEffect *getPlayerEffect(const char *name)
{
	playerEffect *playerEffectIter = playerEffectHead;
	while(playerEffectIter)
	{
		if (!strcmp(playerEffectIter->name, name))
		{
			return playerEffectIter;
		}

		playerEffectIter = (playerEffect*)playerEffectIter->next;
	}

	return NULL;
}

static int getEffectType(const char *filename)
{
	if (endsWithIgnoreCase(filename, ".js"))
	{
		return EFFECT_TYPE_JS;
	}
	else if (endsWithIgnoreCase(filename, ".fs") || endsWithIgnoreCase(filename, ".vs") || endsWithIgnoreCase(filename, ".gs"))
	{
		return EFFECT_TYPE_SHADER;
	}

	return EFFECT_TYPE_C;
}

static playerScene *playerEffectCurrentScene = NULL;

static void initEffect(playerEffect *effect, playerScene *playerScene)
{
	playerEffectCurrentScene = playerScene;
	if (effect->initialized == 0)
	{
		debugPrintf("Initializing effect '%s'", effect->name);
		populateSceneTime(playerScene);

#ifdef JAVASCRIPT
		char jsCall[512];
#endif

		switch(effect->type)
		{
			case EFFECT_TYPE_C:
				if (effect->init)
				{
					effect->init(NULL);
				}
				break;
#ifdef JAVASCRIPT
			case EFFECT_TYPE_JS:
				sprintf(jsCall, "var %s = function() {}", effect->name);
				jsEvalString(jsCall);
				jsEvalFile(effect->reference);
				jsCallClassMethod("Effect", "init", effect->name);
				break;
#endif
		}
		effect->initialized = 1;
		if (isOpenGlError())
		{
			debugErrorPrintf("OpenGL problems in initialization of effect '%s'",
				effect->name);
			printOpenGlErrors();
			windowSetTitle("OpenGL ERROR");
		}
	}
}
static void deinitEffect(playerEffect *effect, playerScene *playerScene)
{
	playerEffectCurrentScene = playerScene;
	if (effect->initialized == 1)
	{
		debugPrintf("Deinitializing effect '%s'", effect->name);

		switch(effect->type)
		{
			case EFFECT_TYPE_C:
				if (effect->deinit)
				{
					effect->deinit(NULL);
				}
				break;
#ifdef JAVASCRIPT
			case EFFECT_TYPE_JS:
				jsCallClassMethod("Effect", "deinit", effect->name);
				jsGarbageCollect();
				break;
#endif
		}
		effect->initialized = 0;
		
		if (isOpenGlError())
		{
			debugErrorPrintf("OpenGL deinitialization problems in effect '%s'",
							 effect->name);
			printOpenGlErrors();
			windowSetTitle("OpenGL ERROR");
		}
	}
}

static void refreshEffect(playerEffect *effect, playerScene *playerScene)
{
	deinitEffect(effect, playerScene);
	windowSetTitle("");
	clearScreenLog();
	initEffect(effect, playerScene);
	playerForceRedraw();
}

const char* getSceneName()
{
	return playerEffectCurrentScene->name;
}

double getSceneStartTime()
{
	return playerEffectCurrentScene->time.start;
}

double getSceneEndTime()
{
	return playerEffectCurrentScene->time.end;
}

double getSceneTimeFromStart()
{
	return playerEffectCurrentScene->time.absolute - playerEffectCurrentScene->time.start;
}
double getSceneProgressPercent()
{
	return playerEffectCurrentScene->time.percent;
}

static int forceRedrawHandling = 0;
static int isForceRedraw = 0;
void playerForceRedraw()
{
	timerUpdate();
	isForceRedraw = 1;
}

static void runEffect(playerEffect *effect, playerScene *playerScene)
{
	playerEffectCurrentScene = playerScene;
	switch(effect->type)
	{
		case EFFECT_TYPE_C:
			if (effect->run && (!timerIsPause() || forceRedrawHandling))
			{
				effect->run(playerScene);
			}
			break;
#ifdef JAVASCRIPT
		case EFFECT_TYPE_JS:
			if (isPlayerEditor())
			{
				//if file has been modified, refresh it
				if (fileModified(effect->reference, &effect->fileLastModifiedTime) == 1)
				{
					debugPrintf("File '%s' has been modified, effect will be refreshed", effect->reference);

					refreshEffect(effect, playerScene);
				}
			}

			if (!timerIsPause() || forceRedrawHandling)
			{
				jsCallClassMethod("Effect", "run", effect->name);
			}
			break;
#endif
	}
}

static int playerEffectSize = 0;
playerEffect *addPlayerEffect(const char *name, const char *reference, 
	void (*init)(playerScene*), void (*run)(playerScene*), void (*deinit)(playerScene*))
{
	playerEffect *pe = getPlayerEffect(name);
	if (pe == NULL)
	{
		pe = (playerEffect*)malloc(sizeof(playerEffect));
		if (pe == NULL)
		{
			debugPrintf("effect memory allocation failed! name:'%s', reference:'%s', ptr:'%X', effects:'%d'", name, reference, pe, playerEffectSize);
			return NULL;
		}

		pe->name = strdup(name);
		assert(pe->name);

		pe->type = getEffectType(reference);

		pe->reference = strdup(reference);
		assert(pe->reference);
		if (pe->type != EFFECT_TYPE_C)
		{
			char *newPath = strdup(getFilePath(reference));

			free(pe->reference);
			pe->reference = newPath;
		}

		pe->initialized = 0;
		pe->init = init;
		pe->run = run;
		pe->deinit = deinit;
		pe->next = NULL;
		pe->fileLastModifiedTime = 0;
		fileModified(pe->reference, &pe->fileLastModifiedTime);
	
		if (playerEffectHead == NULL)
		{
			playerEffectHead = playerEffectPlayerHead
				= playerEffectTail = pe;
		}
		else
		{
			playerEffectTail->next = (struct playerEffect*)pe;
			playerEffectTail = pe;
		}

		playerEffectSize++;
		
		//debugPrintf("effect added! name:'%s', reference:'%s', type:'%d', ptr:'%X', effects:'%d'", pe->name, pe->reference, pe->type, pe, playerEffectSize);
	}
	else
	{
		debugPrintf("effect exists! name:'%s', reference:'%s', type:'%d', ptr:'%X'", name, reference, pe->type, pe);
	}

	return pe;
}


playerScene *playerSceneTail = NULL;
playerScene *playerSceneHead = NULL;
playerScene *playerScenePlayerHead = NULL;
playerScene *playerSceneCurrent = NULL;

playerScene *getPlayerScene(const char *name)
{
	playerScene *playerSceneIter = playerSceneHead;
	while(playerSceneIter)
	{
		if (!strcmp(playerSceneIter->name, name))
		{
			return playerSceneIter;
		}

		playerSceneIter = (playerScene*)playerSceneIter->next;
	}

	return NULL;
}

void playScene(const char *name, playerScene *pScene)
{
	if (pScene)
	{
		populateSceneTime(pScene);
		runEffect(playerSceneCurrent->effect, playerSceneCurrent);
	}
	else
	{
		pScene = getPlayerScene(name);
		if (pScene)
		{
			populateSceneTime(pScene);
			runEffect(playerSceneCurrent->effect, playerSceneCurrent);
		}
	}
}

unsigned int playerSceneSize = 0;
playerScene *addPlayerScene(playerScene *parentScene, const char *name, const char *effectName, const char *startString, const char *durationString)
{

	playerScene *ps = getPlayerScene(name);
	
	//debugPrintf("addPlayerScene '%p' '%s' '%s' '%p'", parentScene, name, effectName, ps);
	
	if (ps == NULL)
	{
		ps = (playerScene*)malloc(sizeof(playerScene));

		if (parentScene == NULL)
		{
			if (playerSceneHead == NULL)
			{
				playerSceneHead = playerScenePlayerHead
					= playerSceneTail = ps;
			}
			else
			{
				playerSceneTail->next = (struct playerScene*)ps;
				playerSceneTail = ps;
			}

			playerSceneSize++;
		}
	}

	ps->playerSceneHead = NULL;
	ps->playerSceneTail = NULL;

	if (parentScene != NULL)
	{
		if (parentScene->playerSceneHead == NULL)
		{
			parentScene->playerSceneHead = (struct playerScene*)ps;
			parentScene->playerSceneTail = (struct playerScene*)ps;
		}
		else
		{
			((playerScene*)parentScene->playerSceneTail)->next = (struct playerScene*)ps;
			parentScene->playerSceneTail = (struct playerScene*)ps;
		}
	}
	
	double start    = convertTimeToSeconds(startString);
	double duration = convertTimeToSeconds(durationString);

	ps->effect = getPlayerEffect(effectName);
	if (ps->effect == NULL && strcmp("undefined",effectName)) //undefined == no effect provided in JSON
	{
		debugPrintf("Effect '%s' doesn't exist for scene '%s'!", effectName, name);
	}

	
	ps->name = strdup(name);
	ps->start = start;
	if (durationString[0] == '#')
	{
		//if '#' preceeds the durationString, then it's the actual end time, not duration
		ps->end = duration;
	}
	else
	{
		ps->end = start+duration;
	}

	ps->next = NULL;
	ps->time.start = ps->start;
	ps->time.end   = ps->end;

	return ps;
}

static void cleanPlayerEffect(void)
{
	playerEffect *playerEffectCurrent = playerEffectHead;
	while(playerEffectCurrent)
	{
		playerEffect *next = (playerEffect*)playerEffectCurrent->next;

		free(playerEffectCurrent->name);
		free(playerEffectCurrent->reference);
		free(playerEffectCurrent);
		playerEffectCurrent = next;
	}
	
	playerEffectTail = NULL;
	playerEffectHead = NULL;
	playerEffectPlayerHead = NULL;
	playerEffectSize = 0;
}

void cleanPlayerScene(void)
{
	playerSceneCurrent = playerSceneHead;
	while(playerSceneCurrent)
	{
		if (playerSceneCurrent->effect)
		{
			deinitEffect(playerSceneCurrent->effect, playerSceneCurrent);
		}

		playerScene *next = (playerScene*)playerSceneCurrent->next;

		free(playerSceneCurrent->name);
		free(playerSceneCurrent);
		playerSceneCurrent = next;
	}
	
	playerSceneTail = NULL;
	playerSceneHead = NULL;
	playerScenePlayerHead = NULL;
	playerSceneCurrent = NULL;
	playerSceneSize = 0;

	playerState = PLAYER_NOT_INITIALIZED;
	
	cleanPlayerEffect();
}

static double loadingPercentage = 0.0f;
static double currentPercentage = 0.0f;
static double nextPercentage = 0.0f;

static void setLoadingStatus(double _currentPercentage, double _nextPercentage)
{
	if (_currentPercentage < 0.0)
	{
		_currentPercentage = 0.0;
	}
	else if (_currentPercentage > 1.0)
	{
		_currentPercentage = 1.0;
	}

	if (_nextPercentage < 0.0)
	{
		_nextPercentage = 0.0;
	}
	else if (_nextPercentage > 1.0)
	{
		_nextPercentage = 1.0;
	}

	loadingPercentage = currentPercentage = _currentPercentage;
	nextPercentage = _nextPercentage;
}

static int playerEditor = 0;
int isPlayerEditor()
{
	return playerEditor;
}

void setPlayerEditor(int _playerEditor)
{
	playerEditor = _playerEditor;
	setDebug(playerEditor); //enable debug
}

static void setSubLoadingStatus(double percentage)
{
	if (percentage < 0.0)
	{
		percentage = 0.0;
	}
	else if (percentage > 1.0)
	{
		percentage = 1.0;
	}

	loadingPercentage = currentPercentage + (nextPercentage-currentPercentage)*percentage;
}

static int sceneResourceCount = 0;
static int sceneResourceI = 0;
void setResourceCount(int _sceneResourceCount)
{
	sceneResourceCount += _sceneResourceCount;
}

static void playerInitLoadingBar()
{
#ifdef JAVASCRIPT
	char jsCall[512];
	sprintf(jsCall, "if (Loader.initLoadingBar !== void null) { Loader.initLoadingBar(); }");
	jsEvalString(jsCall);
#endif
}

static void playerDrawLoadingBarPercent(double percent)
{
	//debugPrintf("loaderbar '%.2f', sceneResourceCount:%d, sceneResourceI:%d, subProgress:%.2f", percent, sceneResourceCount, sceneResourceI, sceneResourceI/(double)sceneResourceCount);

#ifdef JAVASCRIPT
	char jsCall[512];
	sprintf(jsCall, "Loader.drawLoadingBar(%f);", percent);
	jsEvalString(jsCall);
#else
	perspective2dBegin(800, 600);	

	const unsigned int sx = 300;
	const unsigned int sy = 298;
	const unsigned int ex = (int)(sx+200*percent);
	const unsigned int maxx = 500; 
	const unsigned int ey = 302;

	double fade_out = 1.0;
	const double FADE_THRESHOLD = 0.95;
	if (percent > FADE_THRESHOLD)
	{
		fade_out = 1.0 - ((percent-FADE_THRESHOLD)/(1.0-FADE_THRESHOLD)); 
	}
	if (fade_out > 1.0)
	{
		fade_out = 1.0;
	}
	else if (fade_out < 0.0)
	{
		fade_out = 0.0;
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glColor4d(0.6,0.6,0.6,fade_out);
	glBegin(GL_LINE_STRIP);
		glVertex3f( sx-5, sy-5 , 0);
		glVertex3f( maxx+5, sy-5 , 0);
		glVertex3f( maxx+5, ey+5 , 0);
		glVertex3f( sx-5, ey+5 , 0);
		glVertex3f( sx-5, sy-5 , 0);
	glEnd();

	glColor4d(0.3,0.3,0.3,fade_out);
	glBegin(GL_QUADS);
		glVertex3f( sx, sy , 0);
		glVertex3f( ex, sy , 0);
		glVertex3f( ex, ey , 0);
		glVertex3f( sx, ey , 0);
	glEnd();
	
	glDisable(GL_BLEND);

	glColor4d(1.0,1.0,1.0,1.0);

	perspective2dEnd();
#endif
}

void playerDrawLoaderBar(void)
{
	if (playerState != PLAYER_LOADING)
	{
		return;
	}

	double subProgress = 0.0;
	if (sceneResourceCount > 0)
	{
		subProgress = sceneResourceI/(double)sceneResourceCount;
	}
	setSubLoadingStatus(subProgress);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	resetViewport();
	viewReset();

	playerDrawLoadingBarPercent(loadingPercentage);

	graphicsFlush();
}

void notifyResourceLoaded()
{
	if (sceneResourceCount <= 0)
	{
		return;
	}

	threadGlobalMutexLock();

	sceneResourceI++;
	//debugPrintf("notifyResourceLoaded(): %d/%d", sceneResourceI, sceneResourceCount);

	if (!threadIsEnabled())
	{
		playerDrawLoaderBar();
	}

	threadGlobalMutexUnlock();
}

static int showScreenLog = 0;
void playerShowScreenLog(int show)
{
	showScreenLog = show;
	if (show)
	{
		playerForceRedraw();
	}
}

void playerInit(void)
{
	timerCounter_t *counter = timerCounterStart(__func__);

	clearScreenLog();

#ifdef ANTTWEAKBAR
	if (isPlayerEditor())
	{
		TwInit(TW_OPENGL, NULL);
		TwWindowSize(getWindowWidth(), getWindowHeight());
		TwNewBar("Settings");
		
		TwAddButton(TwGetBarByName("Settings"), "Save", saveScriptCallback, NULL, "");
	}
#endif

	cameraInit();
	playlistInit(); //initializes playlist and adds effects to player

#ifdef JAVASCRIPT
	jsInitEngine();
#endif

	playerState = PLAYER_INITIALIZING;
	
	lightingInit();

	//Enable precision depth testing (less Z error on overlapping polygons)
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
#ifndef MORPHOS
    glEnable(GL_RESCALE_NORMAL);
#endif
    //glEnable(GL_NORMALIZE);

	//Enable precision alpha sorting (e.g. overlapping particles etc)
	//glEnable(GL_ALPHA_TEST);				
	//glAlphaFunc(GL_GREATER,0.1);

	//Enable line and point smoothing
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);

	glClearDepth(1.0);
	
	color_t *clearColor = getClearColor();
	glClearColor(clearColor->r, clearColor->g, clearColor->b, clearColor->a);

	resetViewport();
	viewReset();

	if (isOpenGlError())
	{
		debugErrorPrintf("OpenGL initialization problems in initialization");
		printOpenGlErrors();
		windowSetTitle("OpenGL ERROR");
	}
	
	srand(1); //fix pseudo-random to initial value

	playerState = PLAYER_LOADING;

	int loaderBarSize = playerSceneSize;

	playerInitLoadingBar();

	int i;
	playerSceneCurrent = playerSceneHead;
	for(i = 0; playerSceneCurrent != NULL; i++)
	{
		if (isUserExit())
		{
			debugPrintf("Initialization exit.");
			break;
		}

		setLoadingStatus((i)/((double)(loaderBarSize)), (i+1)/((double)(loaderBarSize)));
		//debugPrintf("scene '%s': loadingPercentage:%.2f, currentPercentage:%.2f, nextPercentage:%.2f", playerSceneCurrent->name, loadingPercentage, currentPercentage, nextPercentage);

		playerDrawLoaderBar();

		if (playerSceneCurrent->effect)
		{
			initEffect(playerSceneCurrent->effect, playerSceneCurrent);
		}

		playerSceneCurrent = (playerScene*)playerSceneCurrent->next;
	}

	sceneResourceCount = -1;

	timerCounterEnd(counter);
}

static void playerRunSubScenes(playerScene *parentScene)
{
	const double currentTime = timerGetTime();

	playerScene *playerSubSceneCurrent = (playerScene*)parentScene->playerSceneHead;
	while(playerSubSceneCurrent)
	{
		if (currentTime >= playerSubSceneCurrent->start)
		{
			if (currentTime < playerSubSceneCurrent->end)
			{
				populateSceneTime(playerSubSceneCurrent);

				if (playerSubSceneCurrent->effect)
				{
					runEffect(playerSubSceneCurrent->effect, playerSubSceneCurrent);
				}
			}
		}

		playerSubSceneCurrent = (playerScene*)playerSubSceneCurrent->next;
	}
}

void playerRun(void)
{	
	playerState = PLAYER_RUNNING;
	const double currentTime = timerGetTime();

	playerSceneCurrent = playerScenePlayerHead;
	while(playerSceneCurrent)
	{
		if (currentTime >= playerSceneCurrent->start)
		{
			if (currentTime < playerSceneCurrent->end)
			{
				glPushMatrix();
				glPushAttrib(GL_ALL_ATTRIB_BITS);

				populateSceneTime(playerSceneCurrent);

				if (playerSceneCurrent->effect)
				{
					runEffect(playerSceneCurrent->effect, playerSceneCurrent);
				}
				else if (playerSceneCurrent->playerSceneHead)
				{
					playerRunSubScenes(playerSceneCurrent);
				}
				
				glPopAttrib();
				glPopMatrix();

				if (isOpenGlError())
				{
					debugErrorPrintf("OpenGL problems in scene '%s'",
						playerSceneCurrent->name);
					printOpenGlErrors();
					windowSetTitle("OpenGL ERROR");
				}
			}
		}

		playerSceneCurrent = (playerScene*)playerSceneCurrent->next;
	}
}

static void drawScreenLog(void)
{
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	int width = getScreenWidth();
	int height = getScreenHeight();
	
	perspective2dBegin(width, height);

	glEnable(GL_BLEND);
	glColor4f(0,0,0,0.5);
	glBegin(GL_QUADS);
	glVertex2f(0,0);
	glVertex2f(width,0);
	glVertex2f(width,height);
	glVertex2f(0,height);
	glEnd();
	glDisable(GL_BLEND);

	setTextDefaults();
	setTextWrap(1);
	setDrawTextString(getScreenLog());
	double size = 0.25;
	setTextSize(size, size);
	setTextCenterAlignment(4); //LEFT
	setTextPosition(0, getScreenHeight() - (getTextStringHeight()+getTextCharacterHeight())*size/2.0, 0);
	glColor4f(1,1,1,1);
	drawText2d();
	setTextDefaults();

	perspective2dEnd();
	
	glPopAttrib();
	glPopMatrix();
}

static double shaderRefreshCheck = 0.0f;
static const double shaderRefreshDelay = 0.1f;

static void playerRefresh(int full);
void playerDraw(void)
{
	forceRedrawHandling = 0;
	if (isForceRedraw)
	{
		forceRedrawHandling = 1;
		isForceRedraw = 0;
	}

	if (!timerIsPause() || forceRedrawHandling)
	{
#ifdef SUPPORT_VIDEO
		videoRedrawFrames();
#endif

		if (playerAutoClear) {
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			resetViewport();
			viewReset();
		}
	}

	playerRun();

	if (isPlayerEditor())
	{
		if (showScreenLog)
		{
			drawScreenLog();
		}
		
		if (getPlayerRefreshRequest() > -1)
		{
			playerRefresh(getPlayerRefreshRequest());
		}
	}

	if (!timerIsPause() || forceRedrawHandling)
	{
#ifdef ANTTWEAKBAR
		if (isPlayerEditor())
		{
			TwDraw();
		}
#endif
		graphicsFlush();
	}
	
	if (isPlayerEditor())
	{
		double now = timerGetSeconds();
		if (now > shaderRefreshCheck+shaderRefreshDelay)
		{
			shaderProgramCheckForUpdatesAndRefresh();
			shaderRefreshCheck = now;
		}
	}
}

void playerDeinit(void)
{
	timerCounter_t *counter = timerCounterStart(__func__);

#ifdef SUPPORT_GLSL
	disableShaderProgram();
#endif

	cleanPlayerScene();
	
#ifdef ANTTWEAKBAR
	if (isPlayerEditor())
	{
		TwTerminate();
	}
#endif

	timerCounterEnd(counter);
}

static void playerRefresh(int full)
{
	timerCounter_t *counter = timerCounterStart(__func__);

	threadQueueInit();

	if (full == 1)
	{
		debugPrintf("Doing deep refresh");
	}
	else
	{
		debugPrintf("Doing shallow refresh");
	}
	
	int had_pause = 0;
	if (!timerIsPause())
	{
		timerPause();
	}
	else
	{
		had_pause = 1;
	}
	
	playerDeinit();
	
	if (full)
	{
		memoryDeinit();
	}
	else
	{
		memoryDeinitGeneral();
	}

	setPlayerRefreshRequest(-1);

	sceneResourceI = 0;
	sceneResourceCount = 0;
	loadingPercentage = 0.0;
	currentPercentage = 0.0;
	nextPercentage = 0.0;

	windowSetTitle("");
	playerInit();
	
	threadQueueDeinit();
	
	if (!had_pause)
	{
		timerPause();
	}
	else
	{
		//shaders are not used after refresh in pause state. This is a hack to work-around the bug.
		timerPause();
		playerDraw();
		playerDraw();
		timerPause();

		playerForceRedraw();
	}

	timerCounterEnd(counter);
}

static int refreshRequest = -1;
void setPlayerRefreshRequest(int full)
{
	refreshRequest = full;
}

int getPlayerRefreshRequest()
{
	return refreshRequest;
}
