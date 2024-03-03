#ifndef SYSTEM_GRAPHICS_VIDEO_VIDEO_H_
#define SYSTEM_GRAPHICS_VIDEO_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system/graphics/texture.h"

typedef struct video_t video_t;

struct video_t {
	char *filename;
	unsigned int w, h, frame;
	float startTime;
	float pauseTime;
	float length;
	int state;
	double fps;
	double speed;
	unsigned int currentFrame;
	int paused;
	int useAudio;
	int loop;
	texture_t *frameTexture;
	int codecType;
	void *codec;
	int (*loadVideo)(video_t*);
	int (*playVideo)(video_t*);
	int (*pauseVideo)(video_t*);
	int (*stopVideo)(video_t*);
	int (*freeVideo)(video_t*);
};

extern video_t* videoInit(video_t *video);
extern void videoDeinit(video_t *video);
extern video_t *videoLoad(const char *filename);
extern void videoSetSpeed(video_t *video, double speed);
extern void videoSetFps(video_t *video, double fps);
extern void videoSetLoop(video_t *video, int loop);
extern void videoPlay(video_t *video);
extern void videoSetStartTime(video_t *video, float startTime);
extern void videoSetTime(video_t *video, float time);
extern void videoStop(video_t *video);
extern void videoPause(video_t *video);
extern void videoDraw(video_t *video);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif
