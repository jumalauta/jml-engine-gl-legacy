#ifndef EXH_SYSTEM_PLAYER_PLAYER_H_
#define EXH_SYSTEM_PLAYER_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <time.h>
#include "system/datatypes/datatypes.h"

typedef struct playerScene playerScene;
typedef struct playerEffect playerEffect;

struct playerEffect {
	char *name;
	char *reference;
	time_t fileLastModifiedTime;
	void (*init)(playerScene*);
	void (*run)(playerScene*);
	void (*deinit)(playerScene*);
	int initialized;
	int type;

	struct playerEffect *next;
};

typedef struct {
	double percent;  //Scene running, 0.0 - 1.0
	double absolute; //Absolute seconds value from timerGetTime()
	double now;      //Scene run time extracted from absolute value and start time
	double start;    //Start time of the scene
	double end;      //End time of the scene
	double interval; //Step interval (used in syncs)
	double triggerStart; //Next interval's start point
	double triggerEnd; //Next interval's end point
} sceneTime_t;

struct playerScene {
	char *name;
	double start, end;
	playerEffect *effect;
	
	sceneTime_t time;
	unsigned int variablesSize;

	struct playerScene *next;
	struct playerScene *playerSceneHead;
	struct playerScene *playerSceneTail;
};

extern void playerDrawLoaderBar(void);
extern void playerForceRedraw(void);
extern void playerShowScreenLog(int show);
extern void playerInit(void);
extern void playerRun(void);
extern void playerDraw(void);
extern void playerDeinit(void);
extern void setPlayerRefreshRequest(int full);
extern int getPlayerRefreshRequest();
extern void setPlayerAutoClear(int _playerAutoClear);


extern playerEffect *getPlayerEffect(const char *name);
extern playerEffect *addPlayerEffect(const char *name, const char *reference, 
	void (*init)(playerScene*), void (*run)(playerScene*), void (*deinit)(playerScene*));

extern playerScene *addPlayerScene(playerScene *parentScene, const char *name, const char *effectName, const char *startString, const char *durationString);

extern void playScene(const char *name, playerScene *pScene);

extern playerScene *getPlayerScene(const char *name);

extern const char* getSceneName();
extern double getSceneStartTime();
extern double getSceneEndTime();

extern double getSceneTimeFromStart();
extern double getSceneProgressPercent();

extern void setResourceCount(int _sceneResourceCount);
extern void notifyResourceLoaded();

extern int isPlayerEditor();
extern void setPlayerEditor(int _playerEditor);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*EXH_SYSTEM_PLAYER_PLAYER_H_*/
