#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <graphicsIncludes.h>
#include "system/graphics/graphics.h"
#include "system/io/io.h"
#include "system/debug/debug.h"
#include "system/extensions/gl/gl.h"
#include "system/player/player.h"
#include "system/timer/timer.h"
#include "system/ui/window/window.h"
#include "system/thread/thread.h"
#include "system/datatypes/memory.h"

#include "shader.h"

#define VERTEX_SHADER   1
#define FRAGMENT_SHADER 2
#define GEOMETRY_SHADER 3

static shaderProgram_t *activeShaderProgram = NULL;

static int getShaderType(const char *filename)
{
	if (endsWithIgnoreCase(filename, ".vs"))
	{
		return VERTEX_SHADER;
	}
	else if (endsWithIgnoreCase(filename, ".fs"))
	{
		return FRAGMENT_SHADER;
	}
	else if (endsWithIgnoreCase(filename, ".gs"))
	{
		return GEOMETRY_SHADER;
	}
	
	debugErrorPrintf("Shader type not recognized for file '%s'. Please use file extension '.vs' or '.fs'.", filename);
	return -1;
}

void shaderProgramAddShader(shaderProgram_t *shaderProgram, shader_t *shader)
{
	if (shaderProgram == NULL || shader == NULL)
	{
		debugErrorPrintf("Shader program '%p' or shader '%p' must not be NULL when attaching shader!",shaderProgram,shader);
		return;
	}

	unsigned int i;
	for(i = 0; i < shaderProgram->attachedShadersCount; i++)
	{
		if (shaderProgram->attachedShaders[i] == shader)
		{
			return; //shader already attached => Do not reattach
		}
	}

	shaderProgram->attachedShadersCount++;
	shaderProgram->attachedShaders = (shader_t**)realloc(shaderProgram->attachedShaders, sizeof(shader_t*)*shaderProgram->attachedShadersCount);
	shaderProgram->attachedShaders[shaderProgram->attachedShadersCount-1] = shader;
}

void shaderProgramAddShaderByName(const char *shaderProgramName, const char *shaderName)
{
	shaderProgram_t *shaderProgram = getShaderProgramFromMemory(shaderProgramName);
	assert(shaderProgram);
	shader_t *shader = getShaderFromMemory(shaderName);
	assert(shader);

	shaderProgramAddShader(shaderProgram, shader);
}

void shaderProgramCreateId(shaderProgram_t *shaderProgram)
{
	assert(shaderProgram);

	if (shaderProgram->id != 0)
	{
		glDeleteProgram(shaderProgram->id);
	}

	shaderProgram->id = glCreateProgram();
	if (shaderProgram->id == 0)
	{
		debugErrorPrintf("Shader program '%s' could not be created!",shaderProgram->name);
	}
}

void shaderProgramAttachShader(shaderProgram_t *shaderProgram, shader_t *shader)
{
	if (shaderProgram == NULL || shader == NULL)
	{
		debugErrorPrintf("Shader program '%p' or shader '%p' must not be NULL when attaching shader!",shaderProgram,shader);
		return;
	}
	if (shaderProgram->id == 0)
	{
		shaderProgramCreateId(shaderProgram);
	}
	//debugPrintf("Shader program '%s' adding shader '%p'!",shaderProgram->name,shader);
	if (shader->id == 0)
	{
		debugErrorPrintf("Shader '%s' (ptr:%p, id:%d, type:%d) must be compiled before attaching to program '%s'!",shader->name,shader,shader->id,shader->type,shaderProgram->name);
		return;
	}

	debugPrintf("Attaching shader '%s' (%d) to program '%s' (%d)", shader->name, shader->id, shaderProgram->name, shaderProgram->id);
	glAttachShader(shaderProgram->id, shader->id);
	//printOpenGlShaderProgramInfo(shaderProgram->id);
}

static void shaderProgramAttachShaders(shaderProgram_t *shaderProgram)
{
	if (shaderProgram == NULL)
	{
		debugErrorPrintf("Shader program '%p' must not be NULL when attaching shaders!",shaderProgram);
		return;
	}
	if (shaderProgram->attachedShadersCount == 0)
	{
		debugErrorPrintf("No shaders belong to the program '%s'!",shaderProgram->name);
		return;
	}

	unsigned int i = 0;
	for (i = 0; i < shaderProgram->attachedShadersCount; i++)
	{
		//debugPrintf("'%s' %d: %p", shaderProgram->name, i, shaderProgram->attachedShaders[i]);
		shaderProgramAttachShader(shaderProgram, shaderProgram->attachedShaders[i]);
	}
}


static void shaderProgramDetachShader(shaderProgram_t *shaderProgram, shader_t *shader)
{
	if (shaderProgram == NULL || shader == NULL)
	{
		debugErrorPrintf("Shader program '%p' or shader '%p' must not be NULL when detaching shader!",shaderProgram,shader);
		return;
	}
	//debugPrintf("Shader program '%s' adding shader '%p'!",shaderProgram->name,shader);
	if (shader->id == 0)
	{
		debugWarningPrintf("Shader '%s' (ptr:%p, id:%d, type:%d) was not compiled when attempting detach from program '%s'!",shader->name,shader,shader->id,shader->type,shaderProgram->name);
		return;
	}

	debugPrintf("Detaching shader '%s' (%d) from program '%s' (%d)", shader->name, shader->id, shaderProgram->name, shaderProgram->id);
	glDetachShader(shaderProgram->id, shader->id);
	//printOpenGlShaderProgramInfo(shaderProgram->id);
}

static void shaderProgramDetachShaders(shaderProgram_t *shaderProgram)
{
	if (shaderProgram == NULL)
	{
		debugErrorPrintf("Shader program '%p' must not be NULL when detaching shaders!",shaderProgram);
		return;
	}
	if (shaderProgram->attachedShadersCount == 0)
	{
		debugErrorPrintf("No shaders belong to the program '%s'!",shaderProgram->name);
		return;
	}

	unsigned int i = 0;
	for (i = 0; i < shaderProgram->attachedShadersCount; i++)
	{
		//debugPrintf("'%s' %d: %p", shaderProgram->name, i, shaderProgram->attachedShaders[i]);
		shaderProgramDetachShader(shaderProgram, shaderProgram->attachedShaders[i]);
	}
}

static int shaderProgramLinkStatus(shaderProgram_t *shaderProgram)
{
	shaderProgram->ok = 0;
	int attachedShaders = 0;
	glGetProgramiv(shaderProgram->id, GL_ATTACHED_SHADERS, &attachedShaders);
	if ((unsigned int)attachedShaders != shaderProgram->attachedShadersCount)
	{
		windowSetTitle("GLSL ERROR");
		debugErrorPrintf("Could not attached shaders properly to the shader program '%s', %d<>%d", shaderProgram->name, attachedShaders, shaderProgram->attachedShadersCount);
		return 0;
	}

	//check how shader compilation went
	int logSize = 0;
	glGetProgramiv(shaderProgram->id, GL_INFO_LOG_LENGTH, &logSize);
	char *log = (char*)malloc(sizeof(char)*(logSize+1));
	log[0] = '\0';
	glGetProgramInfoLog(shaderProgram->id, logSize, 0, log);

	int linkStatus = GL_FALSE;
	glGetProgramiv(shaderProgram->id, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		windowSetTitle("GLSL ERROR");
		debugErrorPrintf("Failed to successfully link shader program '%s', log: %s", shaderProgram->name, log);
		free(log);
		return 0;
	}

	free(log);

	shaderProgram->ok = 1;
	return 1;
}

void shaderProgramLink(shaderProgram_t *shaderProgram)
{
	if (shaderProgram == NULL)
	{
		debugErrorPrintf("Shader program '%p' must not be NULL when linking!", shaderProgram);
		return;
	}
	if (shaderProgram->attachedShadersCount == 0)
	{
		debugErrorPrintf("Shader program '%p' must contain shaders '%d' before linking!", shaderProgram, shaderProgram->attachedShadersCount);
		return;
	}

	debugPrintf("Linking '%s' (%d) (attached shaders:%d)", shaderProgram->name, shaderProgram->id, shaderProgram->attachedShadersCount);
	glLinkProgram(shaderProgram->id);
	shaderProgramLinkStatus(shaderProgram);

	//glUseProgram(shaderProgram->id);
	//printOpenGlShaderProgramInfo(shaderProgram->id);
}

void shaderProgramAttachAndLink(shaderProgram_t *shaderProgram)
{
	assert(shaderProgram);

	//debugPrintf("%d Attaching %d shaders to program '%s'", i, shaderPrograms[i].attachedShadersCount, shaderPrograms[i].name);
	shaderProgramAttachShaders(shaderProgram);
	//debugPrintf("%d Linking shaders to program '%s'", i, shaderPrograms[i].name);
	shaderProgramLink(shaderProgram);
	shaderProgramDetachShaders(shaderProgram);
}

unsigned int getUniformLocation(const char *variable)
{
	return glGetUniformLocation(activeShaderProgram->id, variable);
}

void shaderProgramUse(shaderProgram_t *shaderProgram)
{
	activeShaderProgram = shaderProgram;

	if (shaderProgram == NULL)
	{
		glUseProgram(0); //disable shader
		return;
	}

	glUseProgram(shaderProgram->id);
}

void disableShaderProgram()
{
	shaderProgramUse(NULL);
}

void activateShaderProgram(const char *name)
{
	shaderProgram_t *shaderProgram = getShaderProgramFromMemory(name);
	shaderProgramUse(shaderProgram);
}

static int shaderCompileStatus(shader_t *shader)
{
	//check how shader compilation went
	shader->ok = 0;
	int logSize = 0;
	glGetShaderiv(shader->id, GL_INFO_LOG_LENGTH, &logSize);
	char *log = (char*)malloc(sizeof(char)*(logSize+1));
	log[0] = '\0';
	glGetShaderInfoLog(shader->id, logSize, 0, log);

	int compileStatus = GL_FALSE;
	glGetShaderiv(shader->id, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		windowSetTitle("GLSL ERROR");
		debugErrorPrintf("Failed to successfully compile shader '%s/%s', log: %s", shader->name, shader->filename, log);
		free(log);
		return 0;
	}

	free(log);

	shader->ok = 1;
	return 1;
}

static void createShaderId(shader_t *shader)
{
	assert(shader);

	if (shader->id != 0)
	{
		debugErrorPrintf("Shader must not have existing id! id:'%d', shader:'%s'", shader->id, shader->filename);
		return;
	}

	int shaderType = getShaderType(shader->filename);
	unsigned int glShaderType = 0;
	switch (shaderType)
	{
		case VERTEX_SHADER:
			glShaderType = GL_VERTEX_SHADER;
			break;
		case FRAGMENT_SHADER:
			glShaderType = GL_FRAGMENT_SHADER;
			break;
		/*case GEOMETRY_SHADER:
			glShaderType = GL_GEOMETRY_SHADER;
			break;*/
		default:
			debugErrorPrintf("Shader type not recognized! shader:'%s'", shader->filename);
			return;
	}

	//debugPrintf("type: %d %d, src: %s", shaderType, glShaderType, shaderSource);

	shader->id = glCreateShader(glShaderType);
	shader->type = shaderType;
}

int shaderCompile(shader_t *shader)
{
	if (shader == NULL)
	{
		debugPrintf("Shader '%p' must exist when compiling!", shader);
		return 0;
	}

	if (shader->id == 0)
	{
		createShaderId(shader);
	}

	unsigned int fileSize = 0;
	const char *shaderSource = (const char*)ioReadFileToBuffer(shader->filename, &fileSize);
	
	glShaderSource(shader->id, 1, &shaderSource, NULL);
	free((void*)shaderSource);
	
	glCompileShader(shader->id);

	//debugPrintf("Shader '%s' (%d) compiled! type:'%d'", shader->name, shader->id, shader->type);

	return 1;
}

shader_t *shaderLoad(const char *name, const char *_filename)
{
	shader_t *shader = getShaderFromMemory(name);
	if (shader != NULL && shader->id != 0)
	{
		if (fileModified(shader->filename, &shader->fileLastModifiedTime) == 1)
		{
			debugPrintf("Shader '%s' has been modified, reloading", shader->filename);
		}
		else
		{
			return shader;
		}
	}
	else
	{
		shader = memoryAllocateShader(NULL);
	}

	shader->name = strdup(name);
	assert(shader->name);
	shader->filename = strdup(getFilePath(_filename));
	assert(shader->filename);
	shader->id = 0;
	shader->type = 0;
	shader->ok = 0;

	fileModified(shader->filename, &shader->fileLastModifiedTime);

	debugPrintf("Loading shader '%s'. filename:'%s'", shader->name, shader->filename);

	shaderCompile(shader);
	shaderCompileStatus(shader);

	return shader;
}

void shaderDeinit(shader_t* shader)
{
	assert(shader);
	glDeleteShader(shader->id);
	shader->id = 0;
	free(shader->name);
	free(shader->filename);
}

shaderProgram_t *shaderProgramLoad(const char *name)
{
	shaderProgram_t *shaderProgram = getShaderProgramFromMemory(name);
	if (shaderProgram != NULL && shaderProgram->id != 0)
	{
		return shaderProgram;
	}
	else
	{
		shaderProgram = memoryAllocateShaderProgram(NULL);
	}

	shaderProgram->id = 0;
	shaderProgram->attachedShaders = NULL;
	shaderProgram->attachedShadersCount = 0;
	shaderProgram->name = strdup(name);
	assert(shaderProgram->name);

	//debugPrintf("Loading shader program '%s'.", shaderProgram->name);

	shaderProgramCreateId(shaderProgram);

	return shaderProgram;
}

void shaderProgramDeinit(shaderProgram_t* shaderProgram)
{
	assert(shaderProgram);
	disableShaderProgram();

	glDeleteProgram(shaderProgram->id);
	shaderProgram->id = 0;
	free(shaderProgram->attachedShaders);
	free(shaderProgram->name);
}
