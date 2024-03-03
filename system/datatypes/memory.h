#ifndef SYSTEM_DATATYPES_MEMORY_H_
#define SYSTEM_DATATYPES_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system/graphics/graphics.h"
#include "system/graphics/object/object3d.h"
#include "system/graphics/fbo.h"
#include "system/graphics/video/video.h"
#include "system/graphics/shader/shader.h"
#include "system/graphics/font/font.h"

extern void memoryDeinitGeneral();
extern void memoryDeinit();
extern void memoryInit();
extern void memoryAddGeneralPointerToGarbageCollection(void *ptr, void (*deinit)(void*));
extern void* memoryAllocateGeneral(void *ptr, size_t size, void (*deinit)(void*));

extern object3d_t* getObjectFromMemory(const char *filename);
extern object3d_t* memoryAllocateObject(object3d_t *object);

extern texture_t* getTextureFromMemory(const char *filename);
extern texture_t* memoryAllocateTexture(texture_t *texture);

extern fbo_t* getFboFromMemory(const char *name);
extern fbo_t* memoryAllocateFbo(fbo_t *fbo);

extern void videoRedrawFrames();
extern video_t* getVideoFromMemory(const char *filename);
extern video_t* memoryAllocateVideo(video_t *video);

extern int shaderProgramCheckForUpdatesAndRefresh();
extern shaderProgram_t* getShaderProgramFromMemory(const char *name);
extern shaderProgram_t* memoryAllocateShaderProgram(shaderProgram_t *shaderProgram);
extern shader_t* getShaderFromMemory(const char *name);
extern shader_t* memoryAllocateShader(shader_t *shader);

extern font_t* getFontFromMemory(const char *filename);
extern font_t* memoryAllocateFont(font_t *font);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*SYSTEM_DATATYPES_MEMORY_H_*/
