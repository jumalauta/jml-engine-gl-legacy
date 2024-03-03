#include <assert.h>
#include "system/timer/timer.h"
#include "system/thread/thread.h"
#include "system/io/io.h"
#include "system/player/player.h"
#include "system/ui/window/window.h"

#include "memory.h"

typedef struct memory_t memory_t;
struct memory_t {
	void *ptr;
	void (*deinit)(void*);
	unsigned int type;
	struct memory_t *next;
};

#define MEMORY_TYPE_COUNT 8

#define MEMORY_TYPE_GENERAL 0
#define MEMORY_TYPE_FBO 1
#define MEMORY_TYPE_TEXTURE 2
#define MEMORY_TYPE_OBJECT 3
#define MEMORY_TYPE_VIDEO 4
#define MEMORY_TYPE_SHADER_PROGRAM 5
#define MEMORY_TYPE_SHADER 6
#define MEMORY_TYPE_FONT 7


static memory_t *memoryHead[MEMORY_TYPE_COUNT];
static memory_t *memoryTail[MEMORY_TYPE_COUNT];

static void memoryDeinitType(unsigned int type)
{
	assert(type < MEMORY_TYPE_COUNT);

	threadGlobalMutexLock();

	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		if (memoryCurrent->ptr != NULL)
		{
			switch(type)
			{
				case MEMORY_TYPE_OBJECT:
					objectDeinit((object3d_t*)memoryCurrent->ptr);
					break;
				case MEMORY_TYPE_TEXTURE:
					textureDeinit((texture_t*)memoryCurrent->ptr);
					break;
#ifdef SUPPORT_GL_FBO
                case MEMORY_TYPE_FBO:
					fboDeinit((fbo_t*)memoryCurrent->ptr);
					break;
#endif
#ifdef SUPPORT_VIDEO
				case MEMORY_TYPE_VIDEO:
					videoDeinit((video_t*)memoryCurrent->ptr);
					break;
#endif
                case MEMORY_TYPE_SHADER_PROGRAM:
					shaderProgramDeinit((shaderProgram_t*)memoryCurrent->ptr);
					break;
				case MEMORY_TYPE_SHADER:
					shaderDeinit((shader_t*)memoryCurrent->ptr);
					break;
				case MEMORY_TYPE_FONT:
					deinitFont((font_t*)memoryCurrent->ptr);
					break;
				case MEMORY_TYPE_GENERAL:
					if (memoryCurrent->deinit)
					{
						memoryCurrent->deinit(memoryCurrent->ptr);
					}
					break;
				default:
					break;
			}

			free(memoryCurrent->ptr);
		}

		free(memoryCurrent);
		memoryCurrent = memoryNext;
	}
	
	memoryHead[type] = NULL;
	memoryTail[type] = NULL;

	threadGlobalMutexUnlock();
}

void memoryDeinitGeneral()
{
	memoryDeinitType(MEMORY_TYPE_GENERAL);
}

void memoryDeinit()
{
	timerCounter_t *counter = timerCounterStart(__func__);

	debugPrintf("Deinitializing memory");

	unsigned int i;
	for(i = 0; i < MEMORY_TYPE_COUNT; i++)
	{
		memoryDeinitType(i);
	}

	timerCounterEnd(counter);
}

void memoryInit()
{
	debugPrintf("Initializing memory");
	unsigned int i;
	for(i = 0; i < MEMORY_TYPE_COUNT; i++)
	{
		memoryHead[i] = NULL;
		memoryTail[i] = NULL;
	}
}

memory_t* memoryAddToGarbageCollection(unsigned int type, void *ptr, void (*deinit)(void*))
{
	assert(type < MEMORY_TYPE_COUNT);

	memory_t *memory = (memory_t*)malloc(sizeof(memory_t));
	assert(memory);

	memory->ptr = ptr;
	memory->deinit = deinit;
	memory->next = NULL;

	threadGlobalMutexLock();
	
	if (memoryHead[type] == NULL)
	{
		memoryHead[type] = memory;
		memoryTail[type] = memory;
	}
	else
	{
		memoryTail[type]->next = (struct memory_t*)memory;
		memoryTail[type] = memory;
	}

	threadGlobalMutexUnlock();
	
	return memory;
}

void memoryAddGeneralPointerToGarbageCollection(void *ptr, void (*deinit)(void*))
{
	memoryAddToGarbageCollection(MEMORY_TYPE_GENERAL, ptr, deinit);
}

void* memoryAllocateGeneral(void *ptr, size_t size, void (*deinit)(void*))
{
	assert(size > 0 || ptr);

	memory_t *memory = NULL;
	
	if (ptr == NULL)
	{
		ptr = malloc(size);
		assert(ptr);
		//debugPrintf("Allocated new memory '0x%p', bytes:'%d'", ptr, size);
	}
	else //try to reallocate memory if possible
	{
		unsigned int type = MEMORY_TYPE_GENERAL;
		memory_t *memoryCurrent = memoryHead[type];
		while(memoryCurrent)
		{
			memory_t *memoryNext = (memory_t*)memoryCurrent->next;

			assert(memoryCurrent->ptr);
			if (memoryCurrent->ptr == ptr)
			{
				memory = memoryCurrent;
			}

			memoryCurrent = memoryNext;
		}
	}

	if (memory == NULL)
	{
		memory = memoryAddToGarbageCollection(MEMORY_TYPE_GENERAL, (void*)ptr, deinit);
	}
	else
	{
		//void *oldPtr = ptr;
		memory->ptr = realloc(ptr, size);
		assert(memory->ptr);
		//debugPrintf("Reallocated memory '0x%p', bytes:'%d', oldPtr:'0x%p'", ptr, size, oldPtr);
	}
	
	return memory->ptr;
}

object3d_t* getObjectFromMemory(const char *filename)
{
	if (filename == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_OBJECT;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		object3d_t *object = (object3d_t*)memoryCurrent->ptr;
		if (object && !strcmp(object->filename, filename))
		{
			//debugPrintf("No loading, found from memory. name:'%s'",filename);
			return object;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

object3d_t* memoryAllocateObject(object3d_t *object)
{
	if (object == NULL)
	{
		object = (object3d_t*)malloc(sizeof(object3d_t));
		assert(object);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_OBJECT, (void*)object, NULL);
	
	return object;
}


texture_t* getTextureFromMemory(const char *filename)
{
	if (filename == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_TEXTURE;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		texture_t *texture = (texture_t*)memoryCurrent->ptr;
		if (texture && !strcmp(texture->name, filename))
		{
			//debugPrintf("No loading, found from memory. name:'%s'",filename);
			return texture;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

texture_t* memoryAllocateTexture(texture_t *texture)
{
	if (texture == NULL)
	{
		texture = (texture_t*)malloc(sizeof(texture_t));
		assert(texture);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_TEXTURE, (void*)texture, NULL);
	
	return texture;
}

fbo_t* getFboFromMemory(const char *name)
{
	if (name == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_FBO;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		fbo_t *fbo = (fbo_t*)memoryCurrent->ptr;
		if (fbo && !strcmp(fbo->name, name))
		{
			return fbo;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

fbo_t* memoryAllocateFbo(fbo_t *fbo)
{
	if (fbo == NULL)
	{
		fbo = (fbo_t*)malloc(sizeof(fbo_t));
		assert(fbo);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_FBO, (void*)fbo, NULL);
	
	return fbo;
}

#ifdef SUPPORT_VIDEO
void videoRedrawFrames()
{
	unsigned int type = MEMORY_TYPE_VIDEO;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		video_t *video = (video_t*)memoryCurrent->ptr;
		if (video)
		{
			videoDraw(video);
		}

		memoryCurrent = memoryNext;
	}
}

video_t* getVideoFromMemory(const char *filename)
{
	if (filename == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_VIDEO;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		video_t *video = (video_t*)memoryCurrent->ptr;
		if (video && !strcmp(video->filename, filename))
		{
			return video;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

video_t* memoryAllocateVideo(video_t *video)
{
	if (video == NULL)
	{
		video = (video_t*)malloc(sizeof(video_t));
		assert(video);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_VIDEO, (void*)video, NULL);
	
	return video;
}
#endif

static int shaderProgramIsSourceModified(shaderProgram_t *shaderProgram)
{
	int modified = 0;
	unsigned int i;
	for (i = 0; i < shaderProgram->attachedShadersCount; i++)
	{
		shader_t *shader = shaderProgram->attachedShaders[i];
		time_t fileLastModifiedTime = shader->fileLastModifiedTime;
		if (fileModified(shader->filename, &fileLastModifiedTime) == 1)
		{
			disableShaderProgram();

			//clear error logs before refresh
			windowSetTitle("");
			clearScreenLog();

			shader_t *shaderReloaded = shaderLoad(shader->name, shader->filename);
			assert(shader == shaderReloaded); //sanity check that we must not change pointers during reload

			modified = 1;
		}
	}

	if (modified)
	{
		debugPrintf("Reloading shader program '%s'", shaderProgram->name);
		disableShaderProgram();
		shaderProgramCreateId(shaderProgram);
		shaderProgramAttachAndLink(shaderProgram);
		shaderProgramUse(shaderProgram);
		disableShaderProgram();
		playerForceRedraw();
	}

	return modified;
}

int shaderProgramCheckForUpdatesAndRefresh()
{
	int updateStatus = 0;

	unsigned int type = MEMORY_TYPE_SHADER_PROGRAM;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;
		assert(memoryCurrent->ptr);
		shaderProgram_t *shaderProgram = (shaderProgram_t*)memoryCurrent->ptr;

		if (shaderProgramIsSourceModified(shaderProgram))
		{
			updateStatus = 1;
		}

		memoryCurrent = memoryNext;
	}

	return updateStatus;
}

shaderProgram_t* getShaderProgramFromMemory(const char *name)
{
	if (name == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_SHADER_PROGRAM;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		shaderProgram_t *shaderProgram = (shaderProgram_t*)memoryCurrent->ptr;
		if (shaderProgram && !strcmp(shaderProgram->name, name))
		{
			return shaderProgram;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

shaderProgram_t* memoryAllocateShaderProgram(shaderProgram_t *shaderProgram)
{
	if (shaderProgram == NULL)
	{
		shaderProgram = (shaderProgram_t*)malloc(sizeof(shaderProgram_t));
		assert(shaderProgram);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_SHADER_PROGRAM, (void*)shaderProgram, NULL);
	
	return shaderProgram;
}

shader_t* getShaderFromMemory(const char *name)
{
	if (name == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_SHADER;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		shader_t *shader = (shader_t*)memoryCurrent->ptr;
		if (shader && !strcmp(shader->name, name))
		{
			return shader;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

shader_t* memoryAllocateShader(shader_t *shader)
{
	if (shader == NULL)
	{
		shader = (shader_t*)malloc(sizeof(shader_t));
		assert(shader);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_SHADER, (void*)shader, NULL);
	
	return shader;
}

font_t* getFontFromMemory(const char *filename)
{
	if (filename == NULL)
	{
		return NULL;
	}

	unsigned int type = MEMORY_TYPE_FONT;
	memory_t *memoryCurrent = memoryHead[type];
	while(memoryCurrent)
	{
		memory_t *memoryNext = (memory_t*)memoryCurrent->next;

		assert(memoryCurrent->ptr);
		font_t *font = (font_t*)memoryCurrent->ptr;
		if (font && !strcmp(font->name, filename))
		{
			//debugPrintf("No loading, found from memory. name:'%s'",filename);
			return font;
		}

		memoryCurrent = memoryNext;
	}
	
	return NULL;
}

font_t* memoryAllocateFont(font_t *font)
{
	if (font == NULL)
	{
		font = (font_t*)malloc(sizeof(font_t));
		assert(font);
	}

	memoryAddToGarbageCollection(MEMORY_TYPE_FONT, (void*)font, NULL);
	
	return font;
}
