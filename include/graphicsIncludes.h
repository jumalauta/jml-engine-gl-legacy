#ifndef EXH_GRAPHICSINCLUDES_H_
#define EXH_GRAPHICSINCLUDES_H_

#ifndef TINYGL
#define SUPPORT_GL_FBO
#define SUPPORT_GL_VBO
#define SUPPORT_GLSL
#endif

#ifdef SDL
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#endif

#ifdef WINDOWS

#include <windows.h>

//#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>


#define engineGetProcAddress(x) wglGetProcAddress(x)
#define gluCheckExtension(fuck, windows) strstr((const char*)fuck, (const char*)windows)
#define glActiveTextureARB(x) glActiveTextureARB_ptr(x)
//#define glMultiTexCoord2fARB(a,b,c) glMultiTexCoord2fARB_ptr(a,b,c)

#elif __X11__

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#define engineGetProcAddress(x) (*glXGetProcAddress)((const GLubyte*)(x))

#elif MORPHOS

#include <GL/gl.h>
#include <GL/glu.h>

#else /*MAC OS X*/

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

#endif /*WINDOWS*/

#define bindOpenGlFunction(c_function_name, c_function_type) \
	c_function_name = (c_function_type)engineGetProcAddress(#c_function_name); \
	if (c_function_name == NULL) debugErrorPrintf("Could not bind function '%s'!", #c_function_name)

#include "system/extensions/gl/gl.h"

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif

#define UNUSED_ARRAY (void)

#endif /*EXH_GRAPHICSINCLUDES_H_*/
