#include <assert.h>

#include "graphicsIncludes.h"
#include "system/ui/window/window.h"

#include "camera.h"

static camera_t camera;
 
/**
 * @defgroup camera Camera functionality
 */

/**
 * Set camera's position to track a specific 3d object. Camera's position value will be relative to the 3d object. Default is NULL.
 * @param object [in] pointer to the object that camera's position will track
 * @ingroup camera
 * @ref JSAPI
 */
void setCameraPositionObject(object3d_t *object)
{
	camera.positionObject = object;
}

/**
 * Set camera's target to track a specific 3d object.  Camera's target value will be relative to the 3d object. Default is NULL.
 * @param object [in] pointer to the object that camera's target will track
 * @ingroup camera
 * @ref JSAPI
 */
void setCameraTargetObject(object3d_t *object)
{
	camera.targetObject = object;
}

/**
 * Set camera's perspective.
 * @param fovy Specifies the field of view angle, in degrees, in the y direction. Default is 45.
 * @param aspect Specifies the aspect ratio that determines the field of view in the x direction. The aspect ratio is the ratio of x (width) to y (height). Default is relative screen width / screen height.
 * @param zNear Specifies the distance from the viewer to the near clipping plane (always positive). Default is 1.0.
 * @param zFar Specifies the distance from the viewer to the far clipping plane (always positive). Default is 1000.0.
 * @ingroup camera
 * @ref JSAPI
 */
void setCameraPerspective(double fovy, double aspect, double zNear, double zFar)
{
	camera.fovy = fovy;
	camera.aspect = aspect;
	camera.zNear = zNear;
	camera.zFar = zFar;
}

/**
 * Set camera's position. Default is (0,0,2).
 * @param x Camera's X position
 * @param y Camera's Y position
 * @param z Camera's Z position
 * @ingroup camera
 * @ref JSAPI
 */
void setCameraPosition(float x, float y, float z)
{
	setPoint3d(&camera.position, x, y, z);
}

/**
 * Set camera's target (look at). Default is (0,0,0).
 * @param x Camera's X target
 * @param y Camera's Y target
 * @param z Camera's Z target
 * @ingroup camera
 * @ref JSAPI
 */
void setCameraLookAt(float x, float y, float z)
{
	setPoint3d(&camera.lookAt, x, y, z);
}

/**
 * Set camera's up vector. Default is (0,1,0).
 * @param x Camera's X up vector
 * @param y Camera's Y up vector
 * @param z Camera's Z up vector
 * @ingroup camera
 * @ref JSAPI
 */
void setCameraUpVector(float x, float y, float z)
{
	setPoint3d(&camera.up, x, y, z);
}

/**
 * Get camera's data.
 * @return pointer to camera data
 * @see camera_t
 * @ingroup camera
 * @ref JSAPI
 */
camera_t* getCamera()
{
	return &camera;
}

/**
 * Initialize camera's default settings.
 * @ingroup camera
 */
void cameraInit()
{
	setCameraPerspective(45.0, getWindowScreenAreaAspectRatio(), 1.0, 1000.0);
	setCameraPosition(0,0,2);
	setCameraLookAt(0,0,0);
	setCameraUpVector(0,1,0);
	setCameraPositionObject(NULL);
	setCameraTargetObject(NULL);
}
