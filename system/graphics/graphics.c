#include <assert.h>

#include "graphics.h"
#include "graphicsIncludes.h"
#include "system/ui/window/window.h"

static color_t clearColor;
 
/**
 * @defgroup screen Screen functionality
 */

/**
 * Set the clear color of the screen. Values should be in float range 0.0 - 1.0.
 * @param r red color
 * @param g green color
 * @param b blue color
 * @param a alpha color
 * @ref JSAPI
 */
void setClearColor(float r, float g, float b, float a)
{
	clearColor.r = r;
	clearColor.g = g;
	clearColor.b = b;
	clearColor.a = a;
}

/**
 * Get the current screen clear color.
 * @return pointer to clear color
 * @see color_t
 */
color_t* getClearColor()
{
	return &clearColor;
}

/**
 * Setup projection and model matrices according to camera data.
 * @see camera_t
 * @ingroup screen
 * @ref JSAPI
 */
void viewReset(void)
{
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	
	camera_t *camera = getCamera();
	gluPerspective(camera->fovy, camera->aspect, camera->zNear, camera->zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	float positionX = camera->position.x;
	float positionY = camera->position.y;
	float positionZ = camera->position.z;
	float targetX = camera->lookAt.x;
	float targetY = camera->lookAt.y;
	float targetZ = camera->lookAt.z;
	if (camera->positionObject)
	{
		positionX += camera->positionObject->position.x;
		positionY += camera->positionObject->position.y;
		positionZ += camera->positionObject->position.z;
	}
	if (camera->targetObject)
	{
		targetX += camera->targetObject->position.x;
		targetY += camera->targetObject->position.y;
		targetZ += camera->targetObject->position.z;
	}
	
	gluLookAt(
		positionX, positionY, positionZ,
		targetX, targetY, targetZ,
		camera->up.x, camera->up.y, camera->up.z
	);
	
	/*printf("CAMERA SETUP\n\tPOSITION\tx:%f, y:%f, z:%f\n\tLOOK\tx:%f, y:%f, z:%f\n\tUP\tx:%f, y:%f, z:%f\n",
		positionX, positionY, positionZ,
		targetX, targetY, targetZ,
		camera->up.x, camera->up.y, camera->up.z);*/
}

static int perspective2dCount = 0;

/**
 * Change perspective to 2D
 * @param w width of the 2D screen
 * @param h height of the 2D screen
 * @see perspective2dEnd
 * @ingroup screen
 * @ref JSAPI
 */
void perspective2dBegin(int w, int h)
{
	if (perspective2dCount == 0)
	{
		glDisable(GL_DEPTH_TEST);

		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glPushMatrix();
		glOrtho(0, w, 0, h, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	
	perspective2dCount++;
}

/**
 * Change perspective from 2D to 3D.
 * @see perspective2dBegin
 * @ingroup screen
 * @ref JSAPI
 */
void perspective2dEnd(void)
{
	if (perspective2dCount == 1)
	{
		glPopMatrix();
		glPopMatrix();

		glEnable(GL_DEPTH_TEST);

		viewReset();
	}
	
	perspective2dCount--;
	
	if (perspective2dCount < 0)
	{
		debugErrorPrintf("perspective2dBegin call missing! perspective2dCount:'%d'", perspective2dCount);
	}
}
