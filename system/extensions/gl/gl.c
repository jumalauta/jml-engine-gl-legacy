/*
 * OpenGL Extensions
 * References:
 * http://oss.sgi.com/projects/ogl-sample/registry/
 * http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fGL_5fGetProcAddress
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "system/debug/debug.h"
#include "gl.h"

#ifndef __MACOSX__

#ifdef SUPPORT_GLSL
//Shaders
PFNGLCREATEPROGRAMOBJECTARBPROC         glCreateProgramObjectARB        = NULL;
PFNGLDELETEOBJECTARBPROC                glDeleteObjectARB               = NULL;
PFNGLDETACHOBJECTARBPROC                glDetachObjectARB               = NULL;
PFNGLCREATESHADEROBJECTARBPROC          glCreateShaderObjectARB         = NULL;
PFNGLSHADERSOURCEARBPROC                glShaderSourceARB               = NULL;
PFNGLCOMPILESHADERARBPROC               glCompileShaderARB              = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC        glGetObjectParameterivARB       = NULL;
PFNGLATTACHOBJECTARBPROC                glAttachObjectARB               = NULL;
PFNGLGETINFOLOGARBPROC                  glGetInfoLogARB                 = NULL;
PFNGLLINKPROGRAMARBPROC                 glLinkProgramARB                = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC            glUseProgramObjectARB           = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC          glGetUniformLocationARB         = NULL;
PFNGLUNIFORM1FARBPROC                   glUniform1fARB                  = NULL;
PFNGLUNIFORM2FARBPROC                   glUniform2fARB                  = NULL;
PFNGLUNIFORM3FARBPROC                   glUniform3fARB                  = NULL;
PFNGLUNIFORM4FARBPROC                   glUniform4fARB                  = NULL;
PFNGLUNIFORM1IARBPROC                   glUniform1iARB                  = NULL;
PFNGLUNIFORM2IARBPROC                   glUniform2iARB                  = NULL;
PFNGLUNIFORM3IARBPROC                   glUniform3iARB                  = NULL;
PFNGLUNIFORM4IARBPROC                   glUniform4iARB                  = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC            glUniformMatrix4fvARB           = NULL;
#endif

#ifdef SUPPORT_GL_VBO
//VBOs
PFNGLGENBUFFERSARBPROC                  glGenBuffersARB                 = NULL;
PFNGLBINDBUFFERARBPROC                  glBindBufferARB                 = NULL;
PFNGLBUFFERDATAARBPROC                  glBufferDataARB                 = NULL;
PFNGLDELETEBUFFERSARBPROC               glDeleteBuffersARB              = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC        glGetBufferParameterivARB       = NULL;
#endif

#ifdef SUPPORT_GL_FBO
//FBOs
PFNGLISFRAMEBUFFEREXTPROC                       glIsFramebufferEXT                       = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC                     glBindFramebufferEXT                     = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC                  glDeleteFramebuffersEXT                  = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC                     glGenFramebuffersEXT                     = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC              glCheckFramebufferStatusEXT              = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC                glFramebufferTexture1DEXT                = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC                glFramebufferTexture2DEXT                = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC                glFramebufferTexture3DEXT                = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC             glFramebufferRenderbufferEXT             = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC                      glGenerateMipmapEXT                      = NULL;

PFNGLBINDRENDERBUFFEREXTPROC                    glBindRenderbufferEXT                    = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC                 glDeleteRenderbuffersEXT                 = NULL;
PFNGLGENRENDERBUFFERSEXTPROC                    glGenRenderbuffersEXT                    = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC                 glRenderbufferStorageEXT                 = NULL;
#endif

#ifdef WINDOWS
PFNGLBLENDFUNCSEPARATEPROC                      glBlendFuncSeparate                      = NULL;
GL_ActiveTextureARB_Func                        glActiveTextureARB_ptr                   = 0;
GL_MultiTexCoord2fARB_Func                      glMultiTexCoord2fARB_ptr                 = 0;
#endif

#endif

static int hasShaderExtension      = 1;
static int hasVboExtension         = 1;
static int hasFboExtension         = 1;


int openGlExtensionsInit(void)
{
	assert(glGetString(GL_VERSION) != NULL);

	debugPrintf("Initializing OpenGL extensions");
	debugPrintf("OpenGL: %s",(const char*)glGetString(GL_VERSION));
	debugPrintf("GLSL: %s",(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint myMaxTextureUnits, myMaxTextureSize;
	GLfloat maxPointSize;
#if defined(GL_POINT_SIZE_MAX_ARB) && defined(GL_MAX_TEXTURE_UNITS)
	glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxPointSize);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &myMaxTextureUnits);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &myMaxTextureSize); 
	debugPrintf(
		"Max Texture Units: %d\n" \
		"Max Texture Size: %d\n" \
		"Max Point Size: %.2f",
		(int)myMaxTextureUnits,
		(int)myMaxTextureSize,
		(float)maxPointSize
	);
#endif

#ifndef SUPPORT_GL_FBO
	debugPrintf("FBO disabled in build");
#endif

#ifndef SUPPORT_GL_VBO
	debugPrintf("VBO disabled in build");
#endif

#ifndef SUPPORT_GLSL
	debugPrintf("Shaders disabled in build");
#endif

	const GLubyte* glExtensionList = glGetString(GL_EXTENSIONS);

	const char* neededExtensions[64] = {
		"GL_ARB_fragment_shader",
		"GL_ARB_fragment_program",
		"GL_ARB_shading_language_100",
		"GL_ARB_vertex_shader",
		"GL_ARB_shader_objects",
		"GL_ARB_multitexture",
		"GL_EXT_texture_env_combine",
		"GL_EXT_framebuffer_object",
		"GL_ARB_depth_texture",
		"GL_ARB_texture_compression",
		"GL_ARB_texture_cube_map",
		"GL_ARB_texture_env_add",
		"GL_ARB_vertex_program",
		"GL_ARB_vertex_buffer_object",
		"GL_ARB_point_sprite",
		"GL_ARB_point_parameters",
		NULL
	};

	debugPrintf("Checking for required OpenGL extensions");
	//Check if user has the needed extensions
	int i;
	for(i = 0; neededExtensions[i] != NULL; i++)
	{
#if !defined(WINDOWS) && !defined(TINYGL)
		if (!gluCheckExtension((const GLubyte*)neededExtensions[i], glExtensionList))
#else
		if (!strstr((const char*)glExtensionList, neededExtensions[i]))
#endif
		{
			debugWarningPrintf("Graphics card doesn't support %s", neededExtensions[i]);

			if (!strcmp(neededExtensions[i], "GL_ARB_vertex_buffer_object"))
			{
				debugWarningPrintf("Extension is needed for Vertex Buffer Objects to work");
				hasVboExtension = 0;
			}
			else if ((!strcmp(neededExtensions[i], "GL_EXT_framebuffer_object")) ||
				(!strcmp(neededExtensions[i], "GL_ARB_depth_texture")))
			{
				debugWarningPrintf("Extension is needed for Frame Buffer Objects to work");
				hasFboExtension = 0;
			}
			else if ((!strcmp(neededExtensions[i], "GL_ARB_fragment_shader"))     ||
				(!strcmp(neededExtensions[i], "GL_ARB_fragment_program"))     ||
				(!strcmp(neededExtensions[i], "GL_ARB_shading_language_100")) ||
				(!strcmp(neededExtensions[i], "GL_ARB_vertex_shader"))        ||
				(!strcmp(neededExtensions[i], "GL_ARB_shader_objects")))
			{
				debugWarningPrintf("Extension is needed for Shaders to work");
				hasShaderExtension = 0;
			}
		}
	}

	//extension enabling is necessary only in Windows and Linux
	//because OS X has all the extensions enabled by default
#ifndef __MACOSX__
	if (hasShaderExtension)
	{
#ifdef SUPPORT_GLSL
		debugPrintf("loading shaders...");
		//shaders
		bindOpenGlFunction(glCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC);
		bindOpenGlFunction(glDeleteObjectARB, PFNGLDELETEOBJECTARBPROC);
		bindOpenGlFunction(glDetachObjectARB, PFNGLDETACHOBJECTARBPROC);
		bindOpenGlFunction(glCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC);
		bindOpenGlFunction(glShaderSourceARB, PFNGLSHADERSOURCEARBPROC);
		bindOpenGlFunction(glCompileShaderARB, PFNGLCOMPILESHADERARBPROC);
		bindOpenGlFunction(glGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC);
		bindOpenGlFunction(glAttachObjectARB, PFNGLATTACHOBJECTARBPROC);
		bindOpenGlFunction(glGetInfoLogARB, PFNGLGETINFOLOGARBPROC);
		bindOpenGlFunction(glLinkProgramARB, PFNGLLINKPROGRAMARBPROC);
		bindOpenGlFunction(glUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC);
		bindOpenGlFunction(glGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC);
		bindOpenGlFunction(glUniform1fARB, PFNGLUNIFORM1FARBPROC);
		bindOpenGlFunction(glUniform2fARB, PFNGLUNIFORM2FARBPROC);
		bindOpenGlFunction(glUniform3fARB, PFNGLUNIFORM3FARBPROC);
		bindOpenGlFunction(glUniform4fARB, PFNGLUNIFORM4FARBPROC);
		bindOpenGlFunction(glUniform1iARB, PFNGLUNIFORM1IARBPROC);
		bindOpenGlFunction(glUniform2iARB, PFNGLUNIFORM2IARBPROC);
		bindOpenGlFunction(glUniform3iARB, PFNGLUNIFORM3IARBPROC);
		bindOpenGlFunction(glUniform4iARB, PFNGLUNIFORM4IARBPROC);
		bindOpenGlFunction(glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC);
#else
		hasShaderExtension = 0;
#endif
	}
	else
	{
		debugErrorPrintf("Could not load GLSL extensions!");
	}

	if (hasVboExtension)
	{
#ifdef SUPPORT_GL_VBO
		debugPrintf("loading VBOs...");
		//VBOs
		bindOpenGlFunction(glGenBuffersARB, PFNGLGENBUFFERSARBPROC);
		bindOpenGlFunction(glBindBufferARB, PFNGLBINDBUFFERARBPROC);
		bindOpenGlFunction(glBufferDataARB, PFNGLBUFFERDATAARBPROC);
		bindOpenGlFunction(glDeleteBuffersARB, PFNGLDELETEBUFFERSARBPROC);
		bindOpenGlFunction(glGetBufferParameterivARB, PFNGLGETBUFFERPARAMETERIVARBPROC);
#else
		hasVboExtension = 0;
#endif
	}
	else
	{
		debugErrorPrintf("Could not load VBO extensions!");
	}

	if (hasFboExtension)
	{
#ifdef SUPPORT_GL_FBO
		debugPrintf("loading FBOs...");
		//FBOs
		bindOpenGlFunction(glIsFramebufferEXT, PFNGLISFRAMEBUFFEREXTPROC);
		bindOpenGlFunction(glBindFramebufferEXT, PFNGLBINDFRAMEBUFFEREXTPROC);
		bindOpenGlFunction(glDeleteFramebuffersEXT, PFNGLDELETEFRAMEBUFFERSEXTPROC);
		bindOpenGlFunction(glGenFramebuffersEXT, PFNGLGENFRAMEBUFFERSEXTPROC);
		bindOpenGlFunction(glCheckFramebufferStatusEXT, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC);
		bindOpenGlFunction(glFramebufferTexture1DEXT, PFNGLFRAMEBUFFERTEXTURE1DEXTPROC);
		bindOpenGlFunction(glFramebufferTexture2DEXT, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC);
		bindOpenGlFunction(glFramebufferTexture3DEXT, PFNGLFRAMEBUFFERTEXTURE3DEXTPROC);
		bindOpenGlFunction(glFramebufferRenderbufferEXT, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC);
		bindOpenGlFunction(glGetFramebufferAttachmentParameterivEXT, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC);
		bindOpenGlFunction(glGenerateMipmapEXT, PFNGLGENERATEMIPMAPEXTPROC);

		bindOpenGlFunction(glBindRenderbufferEXT, PFNGLBINDRENDERBUFFEREXTPROC);
		bindOpenGlFunction(glDeleteRenderbuffersEXT, PFNGLDELETERENDERBUFFERSEXTPROC);
		bindOpenGlFunction(glGenRenderbuffersEXT, PFNGLGENRENDERBUFFERSEXTPROC);
		bindOpenGlFunction(glRenderbufferStorageEXT, PFNGLRENDERBUFFERSTORAGEEXTPROC);

		int res;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
		debugPrintf("Max Color Attachments: %d", res);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &res);
		debugPrintf("Max Draw Buffers: %d", res);
		
		/*glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &res);
		debugPrintf("Max Framebuffer Width: %d", res);
		glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &res);
		debugPrintf("Max Framebuffer Height: %d", res);
		glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &res);
		debugPrintf("Max Framebuffer Samples: %d", res);
		glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &res);
		debugPrintf("Max Framebuffer Layers: %d", res);*/
#else
		hasFboExtension = 0;
#endif
	}
	else
	{
		debugErrorPrintf("Could not load FBO extensions!");
	}

#ifdef WINDOWS
//typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
//GLAPI void APIENTRY glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
	glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)
		engineGetProcAddress("glBlendFuncSeparate");
	assert(glBlendFuncSeparate);
#ifdef SDL
	glActiveTextureARB_ptr = (GL_ActiveTextureARB_Func)
		SDL_GL_GetProcAddress("glActiveTextureARB");
	glMultiTexCoord2fARB_ptr = (GL_MultiTexCoord2fARB_Func)
		SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
#endif
#endif

#endif

	if ((hasVboExtension) &&
		(hasShaderExtension) &&
		(hasFboExtension))
	{
		return 1;
	}

	return 0;
}
