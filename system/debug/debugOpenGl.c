#include "graphicsIncludes.h"
#include "debug.h"
#include "system/extensions/gl/gl.h"

static GLenum openGlError=GL_NO_ERROR, previousOpenGlError=GL_NO_ERROR;

int isOpenGlError(void)
{
	openGlError = glGetError();

	if ((openGlError != GL_NO_ERROR)
		&& (openGlError != GL_STACK_OVERFLOW)
		&& (openGlError != GL_STACK_UNDERFLOW))
	{
		if (previousOpenGlError != openGlError)
		{
			return 2;
		}

		return 1;
	}

	return 0;
}

int getOpenGlError(void)
{
	return openGlError;
}

void printOpenGlErrors(void)
{
	while (openGlError != GL_NO_ERROR)
	{
		debugErrorPrintf("OpenGL Error: %s", (char*)gluErrorString(openGlError));
		previousOpenGlError = openGlError;

		openGlError = glGetError();
	}
}

void printOpenGlShaderInfo(unsigned int obj)
{
	int infologLength = 0;
	
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	
	if (infologLength > 0)
	{
		char *infoLog = (char *)malloc(infologLength);
		int charsWritten  = 0;
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		debugErrorPrintf("OpenGL shader error: %s",infoLog);
		free(infoLog);
	}
}
