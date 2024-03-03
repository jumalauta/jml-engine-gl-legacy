#ifndef EXH_SYSTEM_GRAPHICS_GRAPHICS_H_
#define EXH_SYSTEM_GRAPHICS_GRAPHICS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDL
#define graphicsFlush() SDL_GL_SwapBuffers()
#elif WINDOWS
#include <windows.h>
extern HDC hDC;
#define graphicsFlush() SwapBuffers(hDC)
#endif

#include "graphicsIncludes.h"

#include "system/math/general/general.h"
#include "system/graphics/object/object3d.h"

#include "system/graphics/camera.h" 
#include "system/graphics/texture.h" 
#include "system/graphics/image/image.h" 
#include "system/graphics/font/font.h"
#include "system/graphics/shader/shader.h"
#include "system/graphics/fbo.h"
#include "system/graphics/object/vbo.h"

extern void setClearColor(float r, float g, float b, float a);
extern color_t* getClearColor();

extern void viewReset(void);
extern void perspective2dBegin(int w, int h);
extern void perspective2dEnd(void);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*EXH_SYSTEM_GRAPHICS_GRAPHICS_H_*/
