#include <assert.h>

#include "graphicsIncludes.h"
#include "system/debug/debug.h"
#include "vbo.h"

void vboSetDefaults(vbo_t* vbo)
{
	assert(vbo);
	
	vbo->id = 0;
	vbo->vertexId = 0;
	vbo->normalId = 0;
	vbo->texCoordId = 0;
	vbo->count = 0;
}

vbo_t* vboInit(vbo_t* vbo)
{
	if (!vbo)
	{
		vbo = (vbo_t*)malloc(sizeof(vbo_t));
	}
	
	vboSetDefaults(vbo);

	glGenBuffers( 1, &vbo->id );
	
	return vbo;
}

void vboDeinit(vbo_t* vbo)
{
	assert(vbo);
	assert(vbo->id > 0);
	
	if (vbo->vertexId)
	{
		glDeleteBuffers(1, &vbo->vertexId);
	}
	if (vbo->normalId)
	{
		glDeleteBuffers(1, &vbo->normalId);
	}
	if (vbo->texCoordId)
	{
		glDeleteBuffers(1, &vbo->texCoordId);
	}
	
	glDeleteBuffers(1, &vbo->id);
}

void vboLoadArray(GLuint* bufferId, GLenum arrayType, unsigned int elementCount, float* buffer)
{
	assert(bufferId);
	assert(buffer);
	
	if (*bufferId == 0)
	{
		glGenBuffers( 1, bufferId );
	}

	glBindBuffer( GL_ARRAY_BUFFER, *bufferId );
	int factor = 0;
	switch (arrayType)
	{
		case GL_VERTEX_ARRAY:
			factor = 3*sizeof(float);
			break;
		case GL_TEXTURE_COORD_ARRAY:
			factor = 2*sizeof(float);
			break;
		case GL_NORMAL_ARRAY:
			factor = 1*sizeof(float);
			break;
		default:
			debugErrorPrintf("Not supported array type:'%d'!", arrayType);
			break;
	}	
	assert(factor > 0);

	glBufferData( GL_ARRAY_BUFFER, elementCount*factor, buffer, GL_STATIC_DRAW );	
}

void vboLoad(vbo_t* vbo, unsigned int elementCount, float* vertexBuffer, float* texcoordBuffer, float* normalBuffer)
{
	assert(vbo);
	
	if (elementCount == 0)
	{
		debugWarningPrintf("Element range must be given! vbo:%d, vbo_ptr:%p", vbo->id, vbo);
		return;
	}

	if (vertexBuffer)
	{
		vboLoadArray(&vbo->vertexId, GL_VERTEX_ARRAY, elementCount, vertexBuffer);
	}
	else if (texcoordBuffer)
	{
		vboLoadArray(&vbo->texCoordId, GL_TEXTURE_COORD_ARRAY, elementCount, texcoordBuffer);
	}
	else if (normalBuffer)
	{
		vboLoadArray(&vbo->normalId, GL_NORMAL_ARRAY, elementCount, normalBuffer);
	}
	else
	{
		debugWarningPrintf("At least one array buffer must be given! vbo:%d, vbo_ptr:%p", vbo->id, vbo);
	}
	
	if (vbo->count == 0)
	{
		vbo->count = elementCount;
	}
}

void vboSetPointer(vbo_t* vbo, GLenum arrayType, int status)
{
	assert(vbo);

	if (status)
	{
		if (arrayType == GL_VERTEX_ARRAY && vbo->vertexId > 0)
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, vbo->vertexId);
			glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);
		}
		else if (arrayType == GL_TEXTURE_COORD_ARRAY && vbo->texCoordId > 0)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, vbo->texCoordId);
			glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL);
		}
		else if (arrayType == GL_NORMAL_ARRAY && vbo->normalId > 0)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, vbo->normalId);
			glNormalPointer(GL_FLOAT, 0, (char*)NULL);
		}
	}
	else
	{
		if (arrayType == GL_VERTEX_ARRAY && vbo->vertexId > 0)
		{
			glDisableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else if (arrayType == GL_TEXTURE_COORD_ARRAY && vbo->texCoordId > 0)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else if (arrayType == GL_NORMAL_ARRAY && vbo->normalId > 0)
		{
			glDisableClientState(GL_NORMAL_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}
}

void vboEnablePointer(vbo_t* vbo, GLenum arrayType)
{
	vboSetPointer(vbo, arrayType, 1);
}

void vboDisablePointer(vbo_t* vbo, GLenum arrayType)
{
	vboSetPointer(vbo, arrayType, 0);
}

void vboSetFaceCount(vbo_t* vbo, unsigned int count)
{
	assert(vbo);
	vbo->count = count;
}

void vboDrawArrays(vbo_t* vbo)
{
	assert(vbo);
	assert(vbo->count > 0);

	glDrawArrays(GL_TRIANGLES, 0, vbo->count);
}

void vboDraw(vbo_t* vbo)
{
	assert(vbo);

	vboEnablePointer(vbo, GL_VERTEX_ARRAY);
	vboEnablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
	vboEnablePointer(vbo, GL_NORMAL_ARRAY);

	vboDrawArrays(vbo);
	
	vboDisablePointer(vbo, GL_VERTEX_ARRAY);
	vboDisablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
	vboDisablePointer(vbo, GL_NORMAL_ARRAY);
}
