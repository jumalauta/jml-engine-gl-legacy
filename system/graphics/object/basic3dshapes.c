#include <assert.h>

#include "graphicsIncludes.h"
#include "system/debug/debug.h"
#include "basic3dshapes.h"

void drawObjectCube(object3d_t *object)
{
	assert(object);

	if (object == NULL)
	{
		return;
	}

	glBegin(GL_QUADS);
	//top
	glNormal3f( 0, 1, 0 );
	glTexCoord2d(1.0,1.0);
	glVertex3f( 0.5f, 0.5f, -0.5f);
	glNormal3f( 0, 1, 0 );
	glTexCoord2d(0.0,1.0);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glNormal3f( 0, 1, 0 );
	glTexCoord2d(0.0,0.0);
	glVertex3f(-0.5f, 0.5f,  0.5f);
	glNormal3f( 0, 1, 0 );
	glTexCoord2d(1.0,0.0);
	glVertex3f( 0.5f, 0.5f,  0.5f);

	//bottom
	glNormal3f( 0, -1, 0 );
	glTexCoord2d(1.0,1.0);
	glVertex3f( 0.5f, -0.5f,  0.5f);
	glNormal3f( 0, -1, 0 );
	glTexCoord2d(0.0,1.0);
	glVertex3f(-0.5f, -0.5f,  0.5f);
	glNormal3f( 0, -1, 0 );
	glTexCoord2d(0.0,0.0);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glNormal3f( 0, -1, 0 );
	glTexCoord2d(1.0,0.0);
	glVertex3f( 0.5f, -0.5f, -0.5f);

	//front
	glNormal3f( 0, 0, 1 );
	glTexCoord2d(1.0,1.0);
	glVertex3f( 0.5f,  0.5f, 0.5f);
	glNormal3f( 0, 0, 1 );
	glTexCoord2d(0.0,1.0);
	glVertex3f(-0.5f,  0.5f, 0.5f);
	glNormal3f( 0, 0, 1 );
	glTexCoord2d(0.0,0.0);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glNormal3f( 0, 0, 1 );
	glTexCoord2d(1.0,0.0);
	glVertex3f( 0.5f, -0.5f, 0.5f);

	//back
	glNormal3f( 0, 0, -1 );
	glTexCoord2d(1.0,1.0);
	glVertex3f( 0.5f, -0.5f, -0.5f);
	glNormal3f( 0, 0, -1 );
	glTexCoord2d(0.0,1.0);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glNormal3f( 0, 0, -1 );
	glTexCoord2d(0.0,0.0);
	glVertex3f(-0.5f,  0.5f, -0.5f);
	glNormal3f( 0, 0, -1 );
	glTexCoord2d(1.0,0.0);
	glVertex3f( 0.5f,  0.5f, -0.5f);

	//left
	glNormal3f( -1, 0, 0 );
	glTexCoord2d(1.0,1.0);
	glVertex3f(-0.5f,  0.5f,  0.5f);
	glNormal3f( -1, 0, 0 );
	glTexCoord2d(0.0,1.0);
	glVertex3f(-0.5f,  0.5f, -0.5f);
	glNormal3f( -1, 0, 0 );
	glTexCoord2d(0.0,0.0);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glNormal3f( -1, 0, 0 );
	glTexCoord2d(1.0,0.0);
	glVertex3f(-0.5f, -0.5f,  0.5f);

	//right
	glNormal3f( 1, 0, 0 );
	glTexCoord2d(1.0,1.0);
	glVertex3f(0.5f,  0.5f, -0.5f);
	glNormal3f( 1, 0, 0 );
	glTexCoord2d(0.0,1.0);
	glVertex3f(0.5f,  0.5f,  0.5f);
	glNormal3f( 1, 0, 0 );
	glTexCoord2d(0.0,0.0);
	glVertex3f(0.5f, -0.5f,  0.5f);
	glNormal3f( 1, 0, 0 );
	glTexCoord2d(1.0,0.0);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glEnd();
}

void drawObjectPyramid(object3d_t *object)
{
	assert(object);

	if (object == NULL)
	{
		return;
	}

	glBegin(GL_TRIANGLES);
	//front
	glVertex3f( 0.0f,  0.5f, 0.0f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f( 0.5f, -0.5f, 0.5f);

	//right
	glVertex3f(0.0f,  0.5f,  0.0f);
	glVertex3f(0.5f, -0.5f,  0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	//back
	glVertex3f( 0.0f,  0.5f,  0.0f);
	glVertex3f( 0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	//left
	glVertex3f( 0.0f,  0.5f,  0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f,  0.5f);
	glEnd();
}
