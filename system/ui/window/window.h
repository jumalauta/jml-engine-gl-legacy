#ifndef EXH_UI_WINDOW_WINDOW_H_
#define EXH_UI_WINDOW_WINDOW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system/graphics/graphics.h"

extern void windowInit();
extern int getScreenWidth(void);
extern int getScreenHeight(void);
extern int getWindowScreenAreaWidth(void);
extern int getWindowScreenAreaHeight(void);
extern double getWindowScreenAreaAspectRatio();
extern void setWindowScreenAreaAspectRatio(double width, double height);
extern int getScreenPositionX(void);
extern int getScreenPositionY(void);
extern int getWindowWidth(void);
extern int getWindowHeight(void);
extern void setWindowFullscreen(int _fullscreen);
extern int windowIsFullscreen(void);
extern void resetViewport(void);
extern int getScreenPowerOfTwoSize(void);
extern void setWindowDimensions(int width, int height);
extern void setScreenDimensions(int height, int width);
extern void setWindowFbo(fbo_t* _fbo);

extern void windowDeinit(void);
extern int windowShow(void);
extern void windowSetTitle(const char *newTitle);
extern void windowSetTitleTimer(const char *titleTimer);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif
