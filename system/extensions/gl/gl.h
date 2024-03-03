#ifndef SYSTEM_EXTENSIONS_GL_GL_H_
#define SYSTEM_EXTENSIONS_GL_GL_H_

#include "graphicsIncludes.h"

extern int openGlExtensionsInit(void);

#ifdef MORPHOS
#define glMultiTexCoord2f(param, u, v) glTexCoord2f((u), (v))
#define glActiveTexture (void)sizeof  
#endif

#if !defined(__MACOSX__) && !defined(MORPHOS)

#ifdef SUPPORT_GLSL
//Shaders
extern PFNGLCREATEPROGRAMOBJECTARBPROC          glCreateProgramObjectARB;
extern PFNGLDELETEOBJECTARBPROC                 glDeleteObjectARB;
extern PFNGLDETACHOBJECTARBPROC                 glDetachObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC           glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC                 glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC                glCompileShaderARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC         glGetObjectParameterivARB;
extern PFNGLATTACHOBJECTARBPROC                 glAttachObjectARB;
extern PFNGLGETINFOLOGARBPROC                   glGetInfoLogARB;
extern PFNGLLINKPROGRAMARBPROC                  glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC             glUseProgramObjectARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC           glGetUniformLocationARB;
extern PFNGLUNIFORM1FARBPROC                    glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC                    glUniform2fARB;
extern PFNGLUNIFORM3FARBPROC                    glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                    glUniform4fARB;
extern PFNGLUNIFORM1IARBPROC                    glUniform1iARB;
extern PFNGLUNIFORM2IARBPROC                    glUniform2iARB;
extern PFNGLUNIFORM3IARBPROC                    glUniform3iARB;
extern PFNGLUNIFORM4IARBPROC                    glUniform4iARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC             glUniformMatrix4fvARB;
#endif

#ifdef SUPPORT_GL_VBO
//VBOs
extern PFNGLGENBUFFERSARBPROC                   glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC                   glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC                   glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC                glDeleteBuffersARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC         glGetBufferParameterivARB;
#endif

#ifdef SUPPORT_GL_FBO
//FBOs
extern PFNGLISRENDERBUFFEREXTPROC                      glIsRenderbufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC                    glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC                 glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC                    glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC                 glRenderbufferStorageEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC          glGetRenderbufferParameterivEXT;
extern PFNGLISFRAMEBUFFEREXTPROC                       glIsFramebufferEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC                     glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC                  glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC                     glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC              glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC                glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC                glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC                glFramebufferTexture3DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC             glFramebufferRenderbufferEXT;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC                      glGenerateMipmapEXT;

extern PFNGLBINDRENDERBUFFEREXTPROC                    glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC                 glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC                    glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC                 glRenderbufferStorageEXT;
#endif

//blending
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
extern PFNGLBLENDFUNCSEPARATEPROC                      glBlendFuncSeparate;

#ifdef __WIN32__
//a crude hack
typedef void (APIENTRY * GL_ActiveTextureARB_Func)(unsigned int);
extern GL_ActiveTextureARB_Func glActiveTextureARB_ptr;
typedef void (APIENTRY * GL_MultiTexCoord2fARB_Func)(unsigned int, float, float);
extern GL_MultiTexCoord2fARB_Func glMultiTexCoord2fARB_ptr;
#define glMultiTexCoord2fARB glMultiTexCoord2fARB_ptr
#define glMultiTexCoord2f glMultiTexCoord2fARB_ptr
#define GL_ActiveTextureARB glActiveTextureARB_ptr
#define glActiveTexture glActiveTextureARB_ptr
#endif

#define glBindBuffer glBindBufferARB
#define glDeleteBuffers glDeleteBuffersARB
#define glGenBuffers glGenBuffersARB
#define glBindBuffer glBindBufferARB
#define glBufferData glBufferDataARB
#define glGetBufferParameteriv glGetBufferParameterivARB

#define glShaderSource glShaderSourceARB
#define glCreateProgram glCreateProgramObjectARB
#define glAttachShader glAttachObjectARB
#define glLinkProgram glLinkProgramARB
#define glUseProgram glUseProgramObjectARB
#define glCreateShader glCreateShaderObjectARB
#define glCompileShader glCompileShaderARB
#define glDeleteShader glDeleteObjectARB
#define glDeleteProgram glDeleteObjectARB
#define glDetachShader glDetachObjectARB
#define glGetShaderiv glGetObjectParameterivARB
#define glGetShaderInfoLog glGetInfoLogARB
#define glGetProgramiv glGetObjectParameterivARB
#define glGetProgramInfoLog glGetInfoLogARB
#define glGetObjectParameteriv glGetObjectParameterivARB
#define glGetUniformLocation glGetUniformLocationARB
#define glUniform1f glUniform1fARB
#define glUniform2f glUniform2fARB
#define glUniform3f glUniform3fARB
#define glUniform4f glUniform4fARB
#define glUniform1i glUniform1iARB
#define glUniform2i glUniform2iARB
#define glUniform3i glUniform3iARB
#define glUniform4i glUniform4iARB
#define glUniformMatrix4fv glUniformMatrix4fvARB

#elif __MACOSX__

#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT

#endif

#define glIsFramebuffer glIsFramebufferEXT
#define glBindFramebuffer glBindFramebufferEXT
#define glDeleteFramebuffers glDeleteFramebuffersEXT
#define glGenFramebuffers glGenFramebuffersEXT
#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#define glFramebufferTexture1D glFramebufferTexture1DEXT
#define glFramebufferTexture2D glFramebufferTexture2DEXT
#define glFramebufferTexture3D glFramebufferTexture3DEXT
#define glFramebufferRenderbuffer glFramebufferRenderbufferEXT
#define glGetFramebufferAttachmentParameteriv glGetFramebufferAttachmentParameterivEXT

#define glBindRenderbuffer glBindRenderbufferEXT
#define glDeleteRenderbuffers glDeleteRenderbuffersEXT
#define glGenRenderbuffers glGenRenderbuffersEXT
#define glRenderbufferStorage glRenderbufferStorageEXT

#ifndef SUPPORT_GL_FBO
#undef glIsFramebuffer
#define glIsFramebuffer(...) debugWarningPrintf("glIsFramebuffer is not supported!", __VA_ARGS__)
#undef glBindFramebuffer
#define glBindFramebuffer(...) debugWarningPrintf("glBindFramebuffer is not supported!", __VA_ARGS__)
#undef glDeleteFramebuffers
#define glDeleteFramebuffers(...) debugWarningPrintf("glDeleteFramebuffers is not supported!", __VA_ARGS__)
#undef glGenFramebuffers
#define glGenFramebuffers(...) debugWarningPrintf("glGenFramebuffers is not supported!", __VA_ARGS__)
#undef glCheckFramebufferStatus
#define glCheckFramebufferStatus(...) GL_FRAMEBUFFER_UNSUPPORTED
#undef glFramebufferTexture1D
#define glFramebufferTexture1D(...) debugWarningPrintf("glFramebufferTexture1D is not supported!", __VA_ARGS__)
#undef glFramebufferTexture2D
#define glFramebufferTexture2D(...) debugWarningPrintf("glFramebufferTexture2D is not supported!", __VA_ARGS__)
#undef glFramebufferTexture3D
#define glFramebufferTexture3D(...) debugWarningPrintf("glFramebufferTexture3D is not supported!", __VA_ARGS__)
#undef glFramebufferRenderbuffer
#define glFramebufferRenderbuffer(...) debugWarningPrintf("glFramebufferRenderbuffer is not supported!", __VA_ARGS__)
#undef glGetFramebufferAttachmentParameteriv
#define glGetFramebufferAttachmentParameteriv(...) debugWarningPrintf("glGetFramebufferAttachmentParameteriv is not supported!", __VA_ARGS__)
#undef glBindRenderbuffer
#define glBindRenderbuffer(...) debugWarningPrintf("glBindRenderbuffer is not supported!", __VA_ARGS__)
#undef glDeleteRenderbuffers
#define glDeleteRenderbuffers(...) debugWarningPrintf("glDeleteRenderbuffers is not supported!", __VA_ARGS__)
#undef glGenRenderbuffers
#define glGenRenderbuffers(...) debugWarningPrintf("glGenRenderbuffers is not supported!", __VA_ARGS__)
#undef glRenderbufferStorage
#define glRenderbufferStorage(...) debugWarningPrintf("glRenderbufferStorage is not supported!", __VA_ARGS__)
#endif

#ifndef SUPPORT_GL_VBO
#undef glBindBuffer
#define glBindBuffer(...) debugWarningPrintf("glBindBuffer is not supported!", __VA_ARGS__)
#undef glDeleteBuffers
#define glDeleteBuffers(...) debugWarningPrintf("glDeleteBuffers is not supported!", __VA_ARGS__)
#undef glGenBuffers
#define glGenBuffers(...) debugWarningPrintf("glGenBuffers is not supported!", __VA_ARGS__)
#undef glBindBuffer
#define glBindBuffer(...) debugWarningPrintf("glBindBuffer is not supported!", __VA_ARGS__)
#undef glBufferData
#define glBufferData(...) debugWarningPrintf("glBufferData is not supported!", __VA_ARGS__)
#undef glGetBufferParameteriv
#define glGetBufferParameteriv(...) debugWarningPrintf("glGetBufferParameteriv is not supported!", __VA_ARGS__)
#endif

#ifndef SUPPORT_GLSL
#undef glShaderSource
#define glShaderSource(...) debugWarningPrintf("glShaderSource is not supported!", __VA_ARGS__)
#undef glCreateProgram
#define glCreateProgram(...) 0
#undef glAttachShader
#define glAttachShader(...) debugWarningPrintf("glAttachShader is not supported!", __VA_ARGS__)
#undef glLinkProgram
#define glLinkProgram(...) debugWarningPrintf("glLinkProgram is not supported!", __VA_ARGS__)
#undef glUseProgram
#define glUseProgram(...) debugWarningPrintf("glUseProgram is not supported!", __VA_ARGS__)
#undef glCreateShader
#define glCreateShader(...) 0
#undef glCompileShader
#define glCompileShader(...) debugErrorPrintf("glCompileShader is not supported!", __VA_ARGS__)
#undef glDeleteShader
#define glDeleteShader(...) debugWarningPrintf("glDeleteShader is not supported!", __VA_ARGS__)
#undef glDeleteProgram
#define glDeleteProgram(...) debugWarningPrintf("glDeleteProgram is not supported!", __VA_ARGS__)
#undef glDetachShader
#define glDetachShader(...) debugWarningPrintf("glDetachShader is not supported!", __VA_ARGS__)
#undef glGetShaderiv
#define glGetShaderiv(...) debugWarningPrintf("glGetShaderiv is not supported!", __VA_ARGS__)
#undef glGetShaderInfoLog
#define glGetShaderInfoLog(...) debugWarningPrintf("glGetShaderInfoLog is not supported!", __VA_ARGS__)
#undef glGetProgramiv
#define glGetProgramiv(...) debugWarningPrintf("glGetProgramiv is not supported!", __VA_ARGS__)
#undef glGetProgramInfoLog
#define glGetProgramInfoLog(...) debugWarningPrintf("glGetProgramInfoLog is not supported!", __VA_ARGS__)
#undef glGetObjectParameteriv
#define glGetObjectParameteriv(...) debugWarningPrintf("glGetObjectParameteriv is not supported!", __VA_ARGS__)
#undef glGetUniformLocation
#define glGetUniformLocation(...) GL_INVALID_OPERATION
#undef glUniform1f
#define glUniform1f(...) debugWarningPrintf("glUniform1f is not supported!", __VA_ARGS__)
#undef glUniform2f
#define glUniform2f(...) debugWarningPrintf("glUniform2f is not supported!", __VA_ARGS__)
#undef glUniform3f
#define glUniform3f(...) debugWarningPrintf("glUniform3f is not supported!", __VA_ARGS__)
#undef glUniform4f
#define glUniform4f(...) debugWarningPrintf("glUniform4f is not supported!", __VA_ARGS__)
#undef glUniform1i
#define glUniform1i(...) debugWarningPrintf("glUniform1i is not supported!", __VA_ARGS__)
#undef glUniform2i
#define glUniform2i(...) debugWarningPrintf("glUniform2i is not supported!", __VA_ARGS__)
#undef glUniform3i
#define glUniform3i(...) debugWarningPrintf("glUniform3i is not supported!", __VA_ARGS__)
#undef glUniform4i
#define glUniform4i(...) debugWarningPrintf("glUniform4i is not supported!", __VA_ARGS__)
#undef glUniformMatrix4fv
#define glUniformMatrix4fv(...) debugWarningPrintf("glUniformMatrix4fv is not supported!", __VA_ARGS__)
#endif

#endif /* SYSTEM_EXTENSIONS_GL_GL_H_ */
