#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"
#include "system/javascript/javascript.h"

#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/graphics/shader/shader.h"
#include "system/graphics/font/font.h"
#include "system/graphics/image/image.h"
#include "system/graphics/object/object3d.h"
#include "system/graphics/object/basic3dshapes.h"
#include "system/graphics/object/lighting.h"
#include "system/graphics/particle/particle.h"
#include "system/datatypes/memory.h"
#include "system/math/splines/spline.h"
#include "system/ui/window/window.h"

#include "bindings.h"

static int duk_setClearColor(duk_context *ctx)
{
	float r = (double)duk_get_number(ctx, 0);
	float g = (double)duk_get_number(ctx, 1);
	float b = (double)duk_get_number(ctx, 2);
	float a = (double)duk_get_number(ctx, 3);

	setClearColor(r, g, b, a);
	
	return 0;
}

static int duk_getObjectFromMemory(duk_context *ctx)
{
	const char *name = (const char*)duk_get_string(ctx, 0);
	
	void *object_ptr = (void*)getObjectFromMemory(name);
	if (object_ptr == NULL)
	{
		debugErrorPrintf("Could not find object by name '%s'", name);
		assert(object_ptr);
	}
	
	duk_idx_t obj = duk_push_object(ctx);
	duk_push_pointer(ctx, object_ptr);
	duk_put_prop_string(ctx, obj, "ptr");
	duk_push_string(ctx, ((object3d_t*)object_ptr)->filename);
	duk_put_prop_string(ctx, obj, "filename");

	return 1;
}

static int duk_useObjectLighting(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	int lighting = (int)duk_get_int(ctx, 1);
	
	useObjectLighting(object, lighting);

	return 0;
}

static int duk_useObjectCamera(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	int camera = (int)duk_get_int(ctx, 1);
	
	useObjectCamera(object, camera);

	return 0;
}

static int duk_useObjectNormals(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	int normals = (int)duk_get_int(ctx, 1);
	
	useObjectNormals(object, normals);

	return 0;
}

static int duk_useObjectTextureCoordinates(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	int coordinates = (int)duk_get_int(ctx, 1);
	
	useObjectTextureCoordinates(object, coordinates);

	return 0;
}

static int duk_useSimpleColors(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	int simpleColors = (int)duk_get_int(ctx, 1);
	
	useSimpleColors(object, simpleColors);

	return 0;
}

static int duk_setObjectScale(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	float x = (float)duk_get_number(ctx, 1);
	float y = (float)duk_get_number(ctx, 2);
	float z = (float)duk_get_number(ctx, 3);
	
	setObjectScale(object, x, y, z);

	return 0;
}

static int duk_setObjectPosition(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	float x = (float)duk_get_number(ctx, 1);
	float y = (float)duk_get_number(ctx, 2);
	float z = (float)duk_get_number(ctx, 3);
	
	setObjectPosition(object, x, y, z);

	return 0;
}

static int duk_setObjectPivot(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	float x = (float)duk_get_number(ctx, 1);
	float y = (float)duk_get_number(ctx, 2);
	float z = (float)duk_get_number(ctx, 3);
	
	setObjectPivot(object, x, y, z);

	return 0;
}

static int duk_setObjectRotation(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	float degreesX = (float)duk_get_number(ctx, 1);
	float degreesY = (float)duk_get_number(ctx, 2);
	float degreesZ = (float)duk_get_number(ctx, 3);
	float x = (float)duk_get_number(ctx, 4);
	float y = (float)duk_get_number(ctx, 5);
	float z = (float)duk_get_number(ctx, 6);
	
	setObjectRotation(object, degreesX, degreesY, degreesZ, x, y, z);

	return 0;
}


static int duk_setObjectColor(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	float r = (float)duk_get_number(ctx, 1);
	float g = (float)duk_get_number(ctx, 2);
	float b = (float)duk_get_number(ctx, 3);
	float a = (float)duk_get_number(ctx, 4);
	
	setObjectColor(object, r, g, b, a);

	return 0;
}

static int duk_loadObjectBasicShape(duk_context *ctx)
{
	const char *name = duk_get_string(ctx, 0);
	const char *objectTypeString = duk_get_string(ctx, 1);
	
	int objectType = BASIC_3D_SHAPE_COMPLEX;
	if (!strcmp(objectTypeString, "CYLINDER"))
	{
		objectType = BASIC_3D_SHAPE_CYLINDER;
	}
	else if (!strcmp(objectTypeString, "DISK"))
	{
		objectType = BASIC_3D_SHAPE_DISK;
	}
	else if (!strcmp(objectTypeString, "SPHERE"))
	{
		objectType = BASIC_3D_SHAPE_SPHERE;
	}
	else if (!strcmp(objectTypeString, "CUBE"))
	{
		objectType = BASIC_3D_SHAPE_CUBE;
	}
	else if ((!strcmp(objectTypeString, "MATRIX")) || (!strcmp(objectTypeString, "CUSTOM")))
	{
		objectType = BASIC_3D_MATRIX;
	}
	else
	{
		debugErrorPrintf("Unknown type object type! name:'%s', type:'%s'", name, objectTypeString);
	}

	void *object_ptr = loadObjectBasicShape(name, objectType);
	
	duk_idx_t fbo_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, object_ptr);
	duk_put_prop_string(ctx, fbo_obj, "ptr");
	duk_push_string(ctx, name);
	duk_put_prop_string(ctx, fbo_obj, "filename");
	duk_push_string(ctx, objectTypeString);
	duk_put_prop_string(ctx, fbo_obj, "objectTypeString");

	return 1;
}

static int duk_loadObject(duk_context *ctx)
{
	const char *filename = duk_get_string(ctx, 0);
	void *object_ptr = loadObject(filename);
	
	duk_idx_t fbo_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, object_ptr);
	duk_put_prop_string(ctx, fbo_obj, "ptr");
	duk_push_string(ctx, filename);
	duk_put_prop_string(ctx, fbo_obj, "filename");	
	return 1;
}

static int duk_replaceObjectTexture(duk_context *ctx)
{
	object3d_t *object = (object3d_t*)duk_get_pointer(ctx, 0);
	const char *findTextureName = (const char*)duk_get_string(ctx, 1);
	const char *replaceTextureName = (const char*)duk_get_string(ctx, 2);

	duk_push_int(ctx, replaceObjectTexture(object, findTextureName, replaceTextureName));

	return 1;
}

static int duk_drawObject(duk_context *ctx)
{
	int argc = duk_get_top(ctx);
	assert(argc > 0);
	
	void *object_ptr = (void*)duk_get_pointer(ctx, 0);
	const char *displayCamera = "Camera01";
	double displayFrame = 0.0;
	int clear = 0;

	if (argc > 1)
	{
		displayCamera = (const char*)duk_get_string(ctx, 1);
	}
	if (argc > 2)
	{
		displayFrame = (double)duk_get_number(ctx, 2);
	}
	if (argc > 3)
	{
		clear = (unsigned int)duk_get_int(ctx, 3);
	}
	
	drawObject(object_ptr, displayCamera, displayFrame, clear);

	return 0;
}

static duk_idx_t duk_push_texture_object(duk_context *ctx, texture_t *tex)
{
	assert(ctx != NULL);

	duk_idx_t tex_obj = duk_push_object(ctx);
	
	if (tex == NULL)
	{
		return tex_obj;
	}

	duk_push_pointer(ctx, (void*)tex);
	duk_put_prop_string(ctx, tex_obj, "ptr");
	duk_push_string(ctx, (const char*)tex->name);
	duk_put_prop_string(ctx, tex_obj, "name");
	duk_push_uint(ctx, tex->w);
	duk_put_prop_string(ctx, tex_obj, "w");
	duk_push_uint(ctx, tex->h);
	duk_put_prop_string(ctx, tex_obj, "h");
	duk_push_number(ctx, tex->x);
	duk_put_prop_string(ctx, tex_obj, "x");
	duk_push_number(ctx, tex->y);
	duk_put_prop_string(ctx, tex_obj, "y");	
	duk_push_number(ctx, tex->z);
	duk_put_prop_string(ctx, tex_obj, "z");	
	duk_push_uint(ctx, tex->id);
	duk_put_prop_string(ctx, tex_obj, "id");
	duk_push_int(ctx, tex->perspective3d);
	duk_put_prop_string(ctx, tex_obj, "perspective3d");

	return tex_obj;
}

#ifdef SUPPORT_GL_FBO
static duk_idx_t duk_push_fbo_object(duk_context *ctx, fbo_t *fbo)
{
	assert(ctx != NULL);

	duk_idx_t fbo_obj = duk_push_object(ctx);

	if (fbo == NULL)
	{
		return fbo_obj;
	}

	duk_push_pointer(ctx, fbo);
	duk_put_prop_string(ctx, fbo_obj, "ptr");
	duk_push_int(ctx, fbo->width);
	duk_put_prop_string(ctx, fbo_obj, "width");
	duk_push_int(ctx, fbo->height);
	duk_put_prop_string(ctx, fbo_obj, "height");
	duk_push_uint(ctx, fbo->id);
	duk_put_prop_string(ctx, fbo_obj, "id");
	if (fbo->color)
	{
		duk_push_texture_object(ctx, fbo->color);
		duk_put_prop_string(ctx, fbo_obj, "color");
	}
	if (fbo->depth)
	{
		duk_push_texture_object(ctx, fbo->depth);
		duk_put_prop_string(ctx, fbo_obj, "depth");
	}

	return fbo_obj;
}
#endif

static int duk_setCameraPositionObject(duk_context *ctx)
{
	object3d_t *object = NULL;
	if (duk_get_top(ctx) > 0)
	{
		object = (object3d_t*)duk_get_pointer(ctx, 0);
	}
	
	setCameraPositionObject(object);
	
	return 0;
}

static int duk_setCameraTargetObject(duk_context *ctx)
{
	object3d_t *object = NULL;
	if (duk_get_top(ctx) > 0)
	{
		object = (object3d_t*)duk_get_pointer(ctx, 0);
	}

	setCameraTargetObject(object);
	
	return 0;
}

static int duk_setCameraPerspective(duk_context *ctx)
{
	double fovy = (double)duk_get_number(ctx, 0);
	double aspect = (double)duk_get_number(ctx, 1);
	double zNear = (double)duk_get_number(ctx, 2);
	double zFar = (double)duk_get_number(ctx, 3);

	setCameraPerspective(fovy, aspect, zNear, zFar);
	
	return 0;
}
static int duk_setCameraPosition(duk_context *ctx)
{
	double x = (double)duk_get_number(ctx, 0);
	double y = (double)duk_get_number(ctx, 1);
	double z = (double)duk_get_number(ctx, 2);

	setCameraPosition(x, y, z);
	
	return 0;
}
static int duk_setCameraLookAt(duk_context *ctx)
{
	double x = (double)duk_get_number(ctx, 0);
	double y = (double)duk_get_number(ctx, 1);
	double z = (double)duk_get_number(ctx, 2);

	setCameraLookAt(x, y, z);
	
	return 0;
}
static int duk_setCameraUpVector(duk_context *ctx)
{
	double x = (double)duk_get_number(ctx, 0);
	double y = (double)duk_get_number(ctx, 1);
	double z = (double)duk_get_number(ctx, 2);

	setCameraUpVector(x, y, z);
	
	return 0;
}
static int duk_getCamera(duk_context *ctx)
{
	camera_t *camera = getCamera();
	assert(camera != NULL);

	duk_idx_t camera_obj = duk_push_object(ctx);
	
	duk_push_pointer(ctx, camera);
	duk_put_prop_string(ctx, camera_obj, "ptr");
	
	duk_idx_t position_obj = duk_push_object(ctx);
	{
		duk_push_number(ctx, camera->position.x);
		duk_put_prop_string(ctx, position_obj, "x");
		duk_push_number(ctx, camera->position.y);
		duk_put_prop_string(ctx, position_obj, "y");
		duk_push_number(ctx, camera->position.z);
		duk_put_prop_string(ctx, position_obj, "z");
	}
	duk_put_prop_string(ctx, camera_obj, "position");

	duk_idx_t lookAt_obj = duk_push_object(ctx);
	{
		duk_push_number(ctx, camera->lookAt.x);
		duk_put_prop_string(ctx, lookAt_obj, "x");
		duk_push_number(ctx, camera->lookAt.y);
		duk_put_prop_string(ctx, lookAt_obj, "y");
		duk_push_number(ctx, camera->lookAt.z);
		duk_put_prop_string(ctx, lookAt_obj, "z");
	}
	duk_put_prop_string(ctx, camera_obj, "lookAt");

	duk_idx_t up_obj = duk_push_object(ctx);
	{
		duk_push_number(ctx, camera->up.x);
		duk_put_prop_string(ctx, up_obj, "x");
		duk_push_number(ctx, camera->up.y);
		duk_put_prop_string(ctx, up_obj, "y");
		duk_push_number(ctx, camera->up.z);
		duk_put_prop_string(ctx, up_obj, "z");
	}
	duk_put_prop_string(ctx, camera_obj, "up");

	return 1;
}

#ifdef SUPPORT_GL_FBO
static int duk_fboInit(duk_context *ctx)
{
	const char *name = duk_get_string(ctx, 0);
	fbo_t* fbo = fboInit(name);

	duk_push_fbo_object(ctx, fbo);
	
	return 1;
}
static int duk_fboBind(duk_context *ctx)
{
	fbo_t* fbo = NULL;
	int argc = duk_get_top(ctx);
	if (argc > 0)
	{
		fbo = (fbo_t*)duk_get_pointer(ctx, 0);
	}

	fboBind(fbo);

	return 0;
}
static int duk_fboDeinit(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);

	fboDeinit(fbo);

	return 0;
}

static int duk_fboStoreDepth(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);
	int storeDepth = (int)duk_get_int(ctx, 1);

	fboStoreDepth(fbo, storeDepth);

	return 0;
}

static int duk_fboSetDimensions(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);
	unsigned int width = (unsigned int)duk_get_uint(ctx, 1);
	unsigned int height = (unsigned int)duk_get_uint(ctx, 2);

	fboSetDimensions(fbo, width, height);

	return 0;
}

static int duk_fboGenerateFramebuffer(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);

	duk_push_uint(ctx, fboGenerateFramebuffer(fbo));

	return 1;
}

static int duk_fboSetRenderDimensions(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);
	double widthPercent = (double)duk_get_number(ctx, 1);
	double heightPercent = (double)duk_get_number(ctx, 2);

	fboSetRenderDimensions(fbo, widthPercent, heightPercent);

	return 0;
}
static int duk_fboGetWidth(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);

	duk_push_int(ctx, fboGetWidth(fbo));

	return 1;
}
static int duk_fboGetHeight(duk_context *ctx)
{
	fbo_t* fbo = (fbo_t*)duk_get_pointer(ctx, 0);

	duk_push_int(ctx, fboGetHeight(fbo));

	return 1;
}

static int duk_fboUpdateViewport(duk_context *ctx)
{
	fbo_t* fbo = NULL;
	int argc = duk_get_top(ctx);
	if (argc > 0)
	{
		fbo = (fbo_t*)duk_get_pointer(ctx, 0);
	}

	fboUpdateViewport(fbo);

	return 0;
}
static int duk_fboBindTextures(duk_context *ctx)
{
	fbo_t* fbo = NULL;
	int argc = duk_get_top(ctx);
	if (argc > 0)
	{
		fbo = (fbo_t*)duk_get_pointer(ctx, 0);
	}

	fboBindTextures(fbo);

	return 0;
}
#endif

static int duk_imageLoadImageAsync(duk_context *ctx)
{
	const char *filename = duk_get_string(ctx, 0);
	
	imageLoadImageAsync(filename);

	return 0;
}

static int duk_imageLoadImage(duk_context *ctx)
{
	const char *filename = duk_get_string(ctx, 0);
	
	texture_t *tex = imageLoadImage(filename);

	duk_push_texture_object(ctx, tex);
	
	return 1;
}

static int duk_setTextureDefaults(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);

	setTextureDefaults(tex);

	return 0;
}

static int duk_setTexturePerspective3d(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	unsigned int perspective3d = (int)duk_get_int(ctx, 1);

	setTexturePerspective3d(tex, perspective3d);

	return 0;
}

static int duk_setTextureBlendFunc(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	unsigned int src = (unsigned int)duk_get_uint(ctx, 1);
	unsigned int dst = (unsigned int)duk_get_uint(ctx, 2);

	setTextureBlendFunc(tex, src, dst);

	return 0;
}

static int duk_setCustomDimensionToTexture(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	int w = (int)duk_get_int(ctx, 1);
	int h = (int)duk_get_int(ctx, 2);

	setCustomDimensionToTexture(tex, w, h);

	return 0;
}

static int duk_setTextureCanvasDimensions(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	int w = (int)duk_get_int(ctx, 1);
	int h = (int)duk_get_int(ctx, 2);

	setTextureCanvasDimensions(tex, w, h);

	return 0;
}

static int duk_setTextureUvDimensions(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	double uMin = (double)duk_get_number(ctx, 1);
	double vMin = (double)duk_get_number(ctx, 2);
	double uMax = (double)duk_get_number(ctx, 3);
	double vMax = (double)duk_get_number(ctx, 4);

	setTextureUvDimensions(tex, uMin, vMin, uMax, vMax);

	return 0;
}

static int duk_setTextureSizeToScreenSize(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);

	setTextureSizeToScreenSize(tex);

	return 0;
}

static int duk_setTexturePivot(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	double x = (double)duk_get_number(ctx, 1);
	double y = (double)duk_get_number(ctx, 2);
	double z = (double)duk_get_number(ctx, 3);

	setTexturePivot(tex, x, y, z);

	return 0;
}

static int duk_setTextureCenterAlignment(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	int alignment = (int)duk_get_int(ctx, 1);
	
	setTextureCenterAlignment(tex, alignment);

	return 0;
}

static int duk_drawTexture(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	
	drawTexture(tex);

	return 0;
}

static int duk_setTextureRotation(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	double degreesX = (double)duk_get_number(ctx, 1);
	double degreesY = (double)duk_get_number(ctx, 2);
	double degreesZ = (double)duk_get_number(ctx, 3);
	double x = (double)duk_get_number(ctx, 4);
	double y = (double)duk_get_number(ctx, 5);
	double z = (double)duk_get_number(ctx, 6);

	setTextureRotation(tex, degreesX, degreesY, degreesZ, x, y, z);

	return 0;
}

static int duk_setTextureUnitTexture(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	unsigned int unit = (unsigned int)duk_get_uint(ctx, 1);
	texture_t *texDst = (texture_t*)duk_get_pointer(ctx, 2);

	setTextureUnitTexture(tex, unit, texDst);

	return 0;
}


static int duk_setTextureScale(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	double w = (double)duk_get_number(ctx, 1);
	double h = (double)duk_get_number(ctx, 2);

	setTextureScale(tex, w, h);

	return 0;
}

static int duk_setTexturePosition(duk_context *ctx)
{
	texture_t *tex = (texture_t*)duk_get_pointer(ctx, 0);
	double x = (double)duk_get_number(ctx, 1);
	double y = (double)duk_get_number(ctx, 2);
	double z = (double)duk_get_number(ctx, 3);

	setTexturePosition(tex, x, y, z);

	return 0;
}

#ifdef SUPPORT_VIDEO
static duk_idx_t duk_push_video_object(duk_context *ctx, video_t *video)
{
	assert(ctx != NULL);

	duk_idx_t video_obj = duk_push_object(ctx);
	
	if (video == NULL)
	{
		return video_obj;
	}

	duk_push_pointer(ctx, (void*)video);
	duk_put_prop_string(ctx, video_obj, "ptr");
	duk_push_string(ctx, (const char*)video->filename);
	duk_put_prop_string(ctx, video_obj, "filename");
	duk_push_uint(ctx, video->w);
	duk_put_prop_string(ctx, video_obj, "w");
	duk_push_uint(ctx, video->h);
	duk_put_prop_string(ctx, video_obj, "h");
	duk_push_number(ctx, video->length);
	duk_put_prop_string(ctx, video_obj, "length");
	duk_push_number(ctx, video->fps);
	duk_put_prop_string(ctx, video_obj, "fps");

	duk_push_texture_object(ctx, video->frameTexture);
	duk_put_prop_string(ctx, video_obj, "frameTexture");

	return video_obj;
}

static int duk_videoLoad(duk_context *ctx)
{
	const char *filename = duk_get_string(ctx, 0);
	
	video_t *video = videoLoad(filename);

	duk_push_video_object(ctx, video);
	
	return 1;
}

static int duk_videoSetSpeed(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);
	double speed = (double)duk_get_number(ctx, 1);

	videoSetSpeed(video, speed);

	return 0;
}

static int duk_videoSetFps(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);
	double fps = (double)duk_get_number(ctx, 1);

	videoSetFps(video, fps);

	return 0;
}

static int duk_videoSetLoop(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);
	int loop = (int)duk_get_int(ctx, 1);

	videoSetLoop(video, loop);

	return 0;
}

static int duk_videoPlay(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);

	videoPlay(video);

	return 0;
}

static int duk_videoSetStartTime(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);
	float startTime = (float)duk_get_number(ctx, 1);

	videoSetStartTime(video, startTime);

	return 0;
}

static int duk_videoSetTime(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);
	float time = (float)duk_get_number(ctx, 1);

	videoSetTime(video, time);

	return 0;
}

static int duk_videoStop(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);

	videoStop(video);

	return 0;
}

static int duk_videoPause(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);

	videoPause(video);

	return 0;
}

static int duk_videoDraw(duk_context *ctx)
{
	video_t *video = (video_t*)duk_get_pointer(ctx, 0);

	videoDraw(video);

	return 0;
}
#endif

static int duk_perspective2dBegin(duk_context *ctx)
{
	int w = (int)duk_get_int(ctx, 0);
	int h = (int)duk_get_int(ctx, 1);
	perspective2dBegin(w,h);
	
	return 0;
}

static int duk_perspective2dEnd(duk_context *ctx)
{
	perspective2dEnd();
	
	return 0;
}

static int duk_setLight4ub(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	unsigned int type = (unsigned int)duk_get_uint(ctx, 1);
	unsigned int f1 = (unsigned int)duk_get_uint(ctx, 2);
	unsigned int f2 = (unsigned int)duk_get_uint(ctx, 3);
	unsigned int f3 = (unsigned int)duk_get_uint(ctx, 4);
	unsigned int f4 = (unsigned int)duk_get_uint(ctx, 5);
	
	setLight4ub(light, type, f1, f2, f3, f4);
	return 0;
}

static int duk_setLight4f(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	unsigned int type = (unsigned int)duk_get_uint(ctx, 1);
	float f1 = (float)duk_get_number(ctx, 2);
	float f2 = (float)duk_get_number(ctx, 3);
	float f3 = (float)duk_get_number(ctx, 4);
	float f4 = (float)duk_get_number(ctx, 5);
	
	setLight4f(light, type, f1, f2, f3, f4);
	return 0;
}

static int duk_lightSetAmbientColor(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	float r = (float)duk_get_number(ctx, 1);
	float g = (float)duk_get_number(ctx, 2);
	float b = (float)duk_get_number(ctx, 3);
	float a = (float)duk_get_number(ctx, 4);
	
	lightSetAmbientColor(light, r, g, b, a);

	return 0;
}

static int duk_lightSetDiffuseColor(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	float r = (float)duk_get_number(ctx, 1);
	float g = (float)duk_get_number(ctx, 2);
	float b = (float)duk_get_number(ctx, 3);
	float a = (float)duk_get_number(ctx, 4);
	
	lightSetDiffuseColor(light, r, g, b, a);

	return 0;
}

static int duk_lightSetSpecularColor(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	float r = (float)duk_get_number(ctx, 1);
	float g = (float)duk_get_number(ctx, 2);
	float b = (float)duk_get_number(ctx, 3);
	float a = (float)duk_get_number(ctx, 4);
	
	lightSetSpecularColor(light, r, g, b, a);

	return 0;
}

static int duk_lightInit(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	
	lightInit(light);

	return 0;
}

static int duk_lightSetOn(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	
	lightSetOn(light);

	return 0;
}

static int duk_lightSetOff(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	
	lightSetOff(light);

	return 0;
}

static int duk_lightSetPosition(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);
	float x = (float)duk_get_number(ctx, 1);
	float y = (float)duk_get_number(ctx, 2);
	float z = (float)duk_get_number(ctx, 3);
	
	lightSetPosition(light, x, y, z);

	return 0;
}

static int duk_lightSetPositionObject(duk_context *ctx)
{
	unsigned int light = (unsigned int)duk_get_uint(ctx, 0);

	object3d_t *object = NULL;
	if (duk_get_top(ctx) > 1)
	{
		object = (object3d_t*)duk_get_pointer(ctx, 1);
	}
	
	lightSetPositionObject(light, object);
	
	return 0;
}

static int duk_isLightingEnabled(duk_context *ctx)
{
	duk_push_int(ctx, (int)isLightingEnabled());

	return 1;
}

static int duk_setTextPivot(duk_context *ctx)
{
	double x = (double)duk_get_number(ctx, 0);
	double y = (double)duk_get_number(ctx, 1);
	double z = (double)duk_get_number(ctx, 2);

	setTextPivot(x, y, z);

	return 0;
}

static int duk_setTextRotation(duk_context *ctx)
{
	double x = (double)duk_get_number(ctx, 0);
	double y = (double)duk_get_number(ctx, 1);
	double z = (double)duk_get_number(ctx, 2);

	setTextRotation(x, y, z);

	return 0;
}

static int duk_setTextSize(duk_context *ctx)
{
	double w = (double)duk_get_number(ctx, 0);
	double h = (double)duk_get_number(ctx, 1);

	setTextSize(w, h);

	return 0;
}

static int duk_setTextDefaults(duk_context *ctx)
{
	setTextDefaults();

	return 0;
}

static int duk_setTextCenterAlignment(duk_context *ctx)
{
	int center = (int)duk_get_int(ctx, 0);

	setTextCenterAlignment(center);

	return 0;
}

static int duk_setTextFont(duk_context *ctx)
{
	const char* font = (const char*)duk_get_string(ctx, 0);

	setTextFont(font);

	return 0;
}

static int duk_setTextPosition(duk_context *ctx)
{
	double x = (double)duk_get_number(ctx, 0);
	double y = (double)duk_get_number(ctx, 1);
	double z = (double)duk_get_number(ctx, 2);

	setTextPosition(x, y, z);

	return 0;
}

static int duk_setDrawTextString(duk_context *ctx)
{
	const char *text = (const char*)duk_get_string(ctx, 0);

	setDrawTextString(text);

	return 0;
}

static int duk_getTextStringHeight(duk_context *ctx)
{
	duk_push_int(ctx, (int)getTextStringHeight());

	return 1;
}

static int duk_getTextStringWidth(duk_context *ctx)
{
	duk_push_int(ctx, (int)getTextStringWidth());

	return 1;
}

static int duk_drawText2d(duk_context *ctx)
{
	drawText2d();

	return 0;
}

static int duk_drawText3d(duk_context *ctx)
{
	drawText3d();

	return 0;
}

static int duk_loadSplineContainerFromFile(duk_context *ctx)
{
	const char *filename = (const char*)duk_get_string(ctx, 0);
	void *ptr = (void*)loadSplineContainerFromFile(filename);

	duk_idx_t spline_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, ptr);
	duk_put_prop_string(ctx, spline_obj, "ptr");
	duk_push_string(ctx, filename);
	duk_put_prop_string(ctx, spline_obj, "name");
	
	return 1;
}

static int duk_drawSplineContainer(duk_context *ctx)
{
	splineContainer *spline = (splineContainer*)duk_get_pointer(ctx, 0);
	float start = (float)duk_get_number(ctx, 1);
	float end = (float)duk_get_number(ctx, 2);

	drawSplineContainer(spline, start, end);

	return 0;
}

static int duk_initParticleContainer(duk_context *ctx)
{
	particleContainer_t *particleContainer = initParticleContainer(NULL);
	
	duk_idx_t particleContainer_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, particleContainer);
	duk_put_prop_string(ctx, particleContainer_obj, "ptr");

	return 1;
}

static int duk_initParticleContainerParticles(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	unsigned int particleI = (unsigned int)duk_get_uint(ctx, 1);
	unsigned int count = (unsigned int)duk_get_uint(ctx, 2);

	initParticleContainerParticles(particleContainer, particleI, count);

	return 0;
}

static int duk_drawParticleContainer(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);

	drawParticleContainer(particleContainer);

	return 0;
}

static void particleCallback(particleContainer_t *particleContainer, particle_t *particle, void *clientData)
{
	const char *functionName = (const char*)clientData;
	
	duk_context *ctx = (duk_context*)jsGetDuktapeContext();

	duk_get_global_string(ctx, functionName);

	duk_idx_t particleContainer_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, particleContainer);
	duk_put_prop_string(ctx, particleContainer_obj, "ptr");

	duk_idx_t particle_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, particle);
	duk_put_prop_string(ctx, particle_obj, "ptr");

	duk_int_t rc = duk_pcall(ctx, 2);
	if (rc != DUK_EXEC_SUCCESS)
	{
		windowSetTitle("JS ERROR");
		debugErrorPrintf("Call failure! particleContainer:'%p', particle:'%p', jsFunction:'%s', error:'%s'", particleContainer, particle, functionName, duk_to_string(ctx, -1));
	}
	duk_pop(ctx);
}

static void initParticleCallback(particleContainer_t *particleContainer, particle_t *particle)
{
	particleCallback(particleContainer, particle, particleContainer->initParticleClientData);
}

static int duk_bindParticleContainerInitParticleFunction(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	const char *functionName = (const char*)duk_get_string(ctx, 1);

	particleContainer->initParticle = initParticleCallback;
	particleContainer->initParticleClientData = (void*)functionName;

	return 0;
}

static void updateParticleCallback(particleContainer_t *particleContainer, particle_t *particle)
{
	particleCallback(particleContainer, particle, particleContainer->updateParticleClientData);
}

static int duk_bindParticleContainerUpdateParticleFunction(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	const char *functionName = (const char*)duk_get_string(ctx, 1);

	particleContainer->updateParticle = updateParticleCallback;
	particleContainer->updateParticleClientData = (void*)functionName;

	return 0;
}

static void updateParticleContainerCallback(particleContainer_t *particleContainer)
{
	const char *functionName = (const char*)particleContainer->updateParticleContainerClientData;

	duk_context *ctx = (duk_context*)jsGetDuktapeContext();

	duk_get_global_string(ctx, functionName);

	duk_idx_t particleContainer_obj = duk_push_object(ctx);
	duk_push_pointer(ctx, particleContainer);
	duk_put_prop_string(ctx, particleContainer_obj, "ptr");

	duk_int_t rc = duk_pcall(ctx, 1);
	if (rc != DUK_EXEC_SUCCESS)
	{
		windowSetTitle("JS ERROR");
		debugErrorPrintf("Call failure! particleContainer:'%p', jsFunction:'%s', error:'%s'", particleContainer, functionName, duk_to_string(ctx, -1));
	}
	duk_pop(ctx);
}

static int duk_bindParticleContainerUpdateParticleContainerFunction(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	const char *functionName = (const char*)duk_get_string(ctx, 1);

	particleContainer->updateParticleContainer = updateParticleContainerCallback;
	particleContainer->updateParticleContainerClientData = (void*)functionName;

	return 0;
}

static int duk_setParticleContainerPerspective3d(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	int perspective3d = (int)duk_get_int(ctx, 1);

	setParticleContainerPerspective3d(particleContainer, perspective3d);

	return 0;
}

static int duk_setParticleContainerDefaultTextureList(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);

	texture_t **particleDefaultTextureList = NULL;
	unsigned int particleDefaultTextureCount = 0;
	duk_idx_t array_obj_index = 1;
	if (duk_is_array(ctx, array_obj_index))
	{
		duk_get_prop_string(ctx, array_obj_index, "length");
		particleDefaultTextureCount = (unsigned int)duk_get_uint(ctx, -1);
		duk_pop(ctx);

		particleDefaultTextureList = (texture_t**)memoryAllocateGeneral((void*)particleContainer->particleDefaultTextureList, sizeof(texture_t*) * particleDefaultTextureCount, NULL);
		assert(particleDefaultTextureList);

		unsigned int i = 0;
		for(i=0; i<particleDefaultTextureCount; i++)
		{
			duk_get_prop_index(ctx, array_obj_index, (duk_uarridx_t)i);
			particleDefaultTextureList[i] = (texture_t*)duk_get_pointer(ctx, -1);
			if (particleDefaultTextureList[i] == NULL)
			{
				debugErrorPrintf("%d/%d texture pointer is not valid. Are you passing texture pointer correctly?",i+1,particleDefaultTextureCount);
				assert(particleDefaultTextureList[i]);
			}
			duk_pop(ctx);
		}
	}

	setParticleContainerDefaultTextureList(particleContainer, particleDefaultTextureList, particleDefaultTextureCount);

	return 0;
}
static int duk_setParticleContainerTime(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	float startTime = (float)duk_get_number(ctx, 1);
	float duration = (float)duk_get_number(ctx, 2);

	setParticleContainerTime(particleContainer, startTime, duration);

	return 0;
}
static int duk_setParticleContainerParticleDurationRange(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	float particleDurationMin = (float)duk_get_number(ctx, 1);
	float particleDurationMax = (float)duk_get_number(ctx, 2);

	setParticleContainerParticleDurationRange(particleContainer, particleDurationMin, particleDurationMax);

	return 0;
}
static int duk_setParticleContainerParticleFadeTimeRange(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	float particleFadeInTime = (float)duk_get_number(ctx, 1);
	float particleFadeOutTime = (float)duk_get_number(ctx, 2);

	setParticleContainerParticleFadeTimeRange(particleContainer, particleFadeInTime, particleFadeOutTime);

	return 0;
}
static int duk_setParticleContainerParticleInitDelay(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	float particleInitDelay = (float)duk_get_number(ctx, 1);

	setParticleContainerParticleInitDelay(particleContainer, particleInitDelay);

	return 0;
}
static int duk_setParticleContainerParticleInitCountMax(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	int particleInitCountMax = (int)duk_get_int(ctx, 1);

	setParticleContainerParticleInitCountMax(particleContainer, particleInitCountMax);

	return 0;
}

static int duk_setParticleContainerPosition(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	point3d_t position;
	position.x = (float)duk_get_number(ctx, 1);
	position.y = (float)duk_get_number(ctx, 2);
	position.z = (float)duk_get_number(ctx, 3);

	setParticleContainerPosition(particleContainer, position);

	return 0;
}
static int duk_setParticleContainerPositionRange(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	point3d_t positionMin;
	positionMin.x = (float)duk_get_number(ctx, 1);
	positionMin.y = (float)duk_get_number(ctx, 2);
	positionMin.z = (float)duk_get_number(ctx, 3);
	point3d_t positionMax;
	positionMax.x = (float)duk_get_number(ctx, 4);
	positionMax.y = (float)duk_get_number(ctx, 5);
	positionMax.z = (float)duk_get_number(ctx, 6);

	setParticleContainerPositionRange(particleContainer, positionMin, positionMax);

	return 0;
}
static int duk_setParticleContainerParticleScaleRange(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	point3d_t particleScaleMin;
	particleScaleMin.x = (float)duk_get_number(ctx, 1);
	particleScaleMin.y = (float)duk_get_number(ctx, 2);
	particleScaleMin.z = (float)duk_get_number(ctx, 3);
	point3d_t particleScaleMax;
	particleScaleMax.x = (float)duk_get_number(ctx, 4);
	particleScaleMax.y = (float)duk_get_number(ctx, 5);
	particleScaleMax.z = (float)duk_get_number(ctx, 6);

	setParticleContainerParticleScaleRange(particleContainer, particleScaleMin, particleScaleMax);

	return 0;
}
static int duk_setParticleContainerParticleAngleRange(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	point3d_t particleAngleMin;
	particleAngleMin.x = (float)duk_get_number(ctx, 1);
	particleAngleMin.y = (float)duk_get_number(ctx, 2);
	particleAngleMin.z = (float)duk_get_number(ctx, 3);
	point3d_t particleAngleMax;
	particleAngleMax.x = (float)duk_get_number(ctx, 4);
	particleAngleMax.y = (float)duk_get_number(ctx, 5);
	particleAngleMax.z = (float)duk_get_number(ctx, 6);

	setParticleContainerParticleAngleRange(particleContainer, particleAngleMin, particleAngleMax);

	return 0;
}
static int duk_setParticleContainerParticleColor(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	color_t particleColor;
	particleColor.r = (float)duk_get_number(ctx, 1);
	particleColor.g = (float)duk_get_number(ctx, 2);
	particleColor.b = (float)duk_get_number(ctx, 3);
	particleColor.a = (float)duk_get_number(ctx, 4);

	setParticleContainerParticleColor(particleContainer, particleColor);

	return 0;
}

static int duk_setParticleContainerParticlePivot(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	point3d_t particlePivot;
	particlePivot.x = (float)duk_get_number(ctx, 1);
	particlePivot.y = (float)duk_get_number(ctx, 2);
	particlePivot.z = (float)duk_get_number(ctx, 3);

	setParticleContainerParticlePivot(particleContainer, particlePivot);

	return 0;
}
static int duk_setParticleContainerDirection(duk_context *ctx)
{
	particleContainer_t *particleContainer = (particleContainer_t*)duk_get_pointer(ctx, 0);
	point3d_t direction;
	direction.x = (float)duk_get_number(ctx, 1);
	direction.y = (float)duk_get_number(ctx, 2);
	direction.z = (float)duk_get_number(ctx, 3);

	setParticleContainerDirection(particleContainer, direction);

	return 0;
}

static int duk_setParticleTexture(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	texture_t *texture = (texture_t*)duk_get_pointer(ctx, 1);

	setParticleTexture(particle, texture);

	return 0;
}
static int duk_setParticleActive(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	int active = (int)duk_get_int(ctx, 1);

	setParticleActive(particle, active);

	return 0;
}
static int duk_setParticleTime(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	float startTime = (float)duk_get_number(ctx, 1);
	float duration = (float)duk_get_number(ctx, 2);

	setParticleTime(particle, startTime, duration);

	return 0;
}
static int duk_setParticlePosition(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t position;
	position.x = (float)duk_get_number(ctx, 1);
	position.y = (float)duk_get_number(ctx, 2);
	position.z = (float)duk_get_number(ctx, 3);

	setParticlePosition(particle, position);

	return 0;
}
static int duk_setParticlePositionRange(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t startPosition;
	startPosition.x = (float)duk_get_number(ctx, 1);
	startPosition.y = (float)duk_get_number(ctx, 2);
	startPosition.z = (float)duk_get_number(ctx, 3);
	point3d_t endPosition;
	endPosition.x = (float)duk_get_number(ctx, 4);
	endPosition.y = (float)duk_get_number(ctx, 5);
	endPosition.z = (float)duk_get_number(ctx, 6);

	setParticlePositionRange(particle, startPosition, endPosition);

	return 0;
}
static int duk_setParticleScale(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t scale;
	scale.x = (float)duk_get_number(ctx, 1);
	scale.y = (float)duk_get_number(ctx, 2);
	scale.z = (float)duk_get_number(ctx, 3);

	setParticleScale(particle, scale);

	return 0;
}
static int duk_setParticleScaleRange(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t startScale;
	startScale.x = (float)duk_get_number(ctx, 1);
	startScale.y = (float)duk_get_number(ctx, 2);
	startScale.z = (float)duk_get_number(ctx, 3);
	point3d_t endScale;
	endScale.x = (float)duk_get_number(ctx, 4);
	endScale.y = (float)duk_get_number(ctx, 5);
	endScale.z = (float)duk_get_number(ctx, 6);

	setParticleScaleRange(particle, startScale, endScale);

	return 0;
}
static int duk_setParticleAngle(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t angle;
	angle.x = (float)duk_get_number(ctx, 1);
	angle.y = (float)duk_get_number(ctx, 2);
	angle.z = (float)duk_get_number(ctx, 3);

	setParticleAngle(particle, angle);

	return 0;
}
static int duk_setParticleAngleRange(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t startAngle;
	startAngle.x = (float)duk_get_number(ctx, 1);
	startAngle.y = (float)duk_get_number(ctx, 2);
	startAngle.z = (float)duk_get_number(ctx, 3);
	point3d_t endAngle;
	endAngle.x = (float)duk_get_number(ctx, 4);
	endAngle.y = (float)duk_get_number(ctx, 5);
	endAngle.z = (float)duk_get_number(ctx, 6);

	setParticleAngleRange(particle, startAngle, endAngle);

	return 0;
}
static int duk_setParticleColor(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	color_t color;
	color.r = (float)duk_get_number(ctx, 1);
	color.g = (float)duk_get_number(ctx, 2);
	color.b = (float)duk_get_number(ctx, 3);
	color.a = (float)duk_get_number(ctx, 4);

	setParticleColor(particle, color);

	return 0;
}

static int duk_setParticlePivot(duk_context *ctx)
{
	particle_t *particle = (particle_t*)duk_get_pointer(ctx, 0);
	point3d_t pivot;
	pivot.x = (float)duk_get_number(ctx, 1);
	pivot.y = (float)duk_get_number(ctx, 2);
	pivot.z = (float)duk_get_number(ctx, 3);

	setParticlePivot(particle, pivot);

	return 0;
}

void bindJsGraphicsFunctions(duk_context *ctx)
{
	bindCFunctionToJs(setClearColor, 4);

	bindCFunctionToJs(getObjectFromMemory, 1);
	bindCFunctionToJs(useObjectLighting, 2);
	bindCFunctionToJs(useObjectCamera, 2);
	bindCFunctionToJs(useObjectNormals, 2);
	bindCFunctionToJs(useObjectTextureCoordinates, 2);
	bindCFunctionToJs(useSimpleColors, 2);
	bindCFunctionToJs(setObjectScale, 4);
	bindCFunctionToJs(setObjectPosition, 4);
	bindCFunctionToJs(setObjectPivot, 4);
	bindCFunctionToJs(setObjectRotation, 7);
	bindCFunctionToJs(setObjectColor, 5);

	bindCFunctionToJs(loadObjectBasicShape, 2);
	bindCFunctionToJs(loadObject, 1);
	bindCFunctionToJs(replaceObjectTexture, 3);
	bindCFunctionToJs(drawObject, DUK_VARARGS);

	bindCFunctionToJs(setCameraPositionObject, DUK_VARARGS);
	bindCFunctionToJs(setCameraTargetObject, DUK_VARARGS);
	bindCFunctionToJs(setCameraPerspective, 4);
	bindCFunctionToJs(setCameraPosition, 3);
	bindCFunctionToJs(setCameraLookAt, 3);
	bindCFunctionToJs(setCameraUpVector, 3);
	bindCFunctionToJs(getCamera, 0);

#ifdef SUPPORT_GL_FBO
	bindCFunctionToJs(fboInit, 1);
	bindCFunctionToJs(fboBind, DUK_VARARGS);
	bindCFunctionToJs(fboDeinit, 1);
	bindCFunctionToJs(fboStoreDepth, 2);
	bindCFunctionToJs(fboSetDimensions, 3);
	bindCFunctionToJs(fboGenerateFramebuffer, 1);
	bindCFunctionToJs(fboSetRenderDimensions, 3);
	bindCFunctionToJs(fboGetWidth, 1);
	bindCFunctionToJs(fboGetHeight, 1);
	bindCFunctionToJs(fboUpdateViewport, DUK_VARARGS);
	bindCFunctionToJs(fboBindTextures, DUK_VARARGS);
#endif

	bindCFunctionToJs(imageLoadImageAsync, 1);
	bindCFunctionToJs(imageLoadImage, 1);
	bindCFunctionToJs(drawTexture, 1);
	bindCFunctionToJs(setTextureCenterAlignment, 2);
	bindCFunctionToJs(setTextureDefaults, 1);
	bindCFunctionToJs(setTexturePerspective3d, 2);
	bindCFunctionToJs(setTextureBlendFunc, 3);
	bindCFunctionToJs(setCustomDimensionToTexture, 3);
	bindCFunctionToJs(setTextureCanvasDimensions, 3);
	bindCFunctionToJs(setTextureUvDimensions, 5);
	bindCFunctionToJs(setTextureSizeToScreenSize, 1);
	bindCFunctionToJs(setTexturePivot, 4);
	bindCFunctionToJs(setTextureUnitTexture, 3);
	bindCFunctionToJs(setTextureRotation, 7);
	bindCFunctionToJs(setTextureScale, 3);
	bindCFunctionToJs(setTexturePosition, 4);

#ifdef SUPPORT_VIDEO
	bindCFunctionToJs(videoLoad, 1);
	bindCFunctionToJs(videoSetSpeed, 2);
	bindCFunctionToJs(videoSetFps, 2);
	bindCFunctionToJs(videoSetLoop, 2);
	bindCFunctionToJs(videoPlay, 1);
	bindCFunctionToJs(videoSetStartTime, 2);
	bindCFunctionToJs(videoSetTime, 2);
	bindCFunctionToJs(videoStop, 1);
	bindCFunctionToJs(videoPause, 1);
	bindCFunctionToJs(videoDraw, 1);
#endif

	bindCFunctionToJs(perspective2dBegin, 2);
	bindCFunctionToJs(perspective2dEnd, 0);

	bindCFunctionToJs(setLight4f, 6);
	bindCFunctionToJs(setLight4ub, 6);
	bindCFunctionToJs(lightInit, 1);
	bindCFunctionToJs(lightSetOn, 1);
	bindCFunctionToJs(lightSetOff, 1);
	bindCFunctionToJs(lightSetAmbientColor, 5);
	bindCFunctionToJs(lightSetDiffuseColor, 5);
	bindCFunctionToJs(lightSetSpecularColor, 5);
	bindCFunctionToJs(lightSetPosition, 4);
	bindCFunctionToJs(lightSetPositionObject, DUK_VARARGS);
	bindCFunctionToJs(isLightingEnabled, 0);
	
	bindCFunctionToJs(setTextPivot, 3);
	bindCFunctionToJs(setTextRotation, 3);
	bindCFunctionToJs(setTextSize, 2);
	bindCFunctionToJs(setTextDefaults, 0);
	bindCFunctionToJs(setTextFont, 1);
	bindCFunctionToJs(setTextPosition, 3);
	bindCFunctionToJs(setTextCenterAlignment, 1);
	bindCFunctionToJs(setDrawTextString, 1);
	bindCFunctionToJs(getTextStringWidth, 0);
	bindCFunctionToJs(getTextStringHeight, 0);
	bindCFunctionToJs(drawText2d, 0);
	bindCFunctionToJs(drawText3d, 0);

	bindCFunctionToJs(loadSplineContainerFromFile, 1);
	bindCFunctionToJs(drawSplineContainer, 3);

	bindCFunctionToJs(initParticleContainer, DUK_VARARGS);
	bindCFunctionToJs(initParticleContainerParticles, 3);
	bindCFunctionToJs(drawParticleContainer, 1);
	bindCFunctionToJs(bindParticleContainerInitParticleFunction, 2);
	bindCFunctionToJs(bindParticleContainerUpdateParticleFunction, 2);
	bindCFunctionToJs(bindParticleContainerUpdateParticleContainerFunction, 2);

	bindCFunctionToJs(setParticleContainerPerspective3d, 2);
	bindCFunctionToJs(setParticleContainerDefaultTextureList, 2);
	bindCFunctionToJs(setParticleContainerTime, 3);
	bindCFunctionToJs(setParticleContainerParticleDurationRange, 3);
	bindCFunctionToJs(setParticleContainerParticleFadeTimeRange, 3);
	bindCFunctionToJs(setParticleContainerParticleInitDelay, 2);
	bindCFunctionToJs(setParticleContainerParticleInitCountMax, 2);

	bindCFunctionToJs(setParticleContainerPosition, 4);
	bindCFunctionToJs(setParticleContainerPositionRange, 7);
	bindCFunctionToJs(setParticleContainerParticleScaleRange, 7);
	bindCFunctionToJs(setParticleContainerParticleAngleRange, 7);
	bindCFunctionToJs(setParticleContainerParticleColor, 5);
	bindCFunctionToJs(setParticleContainerParticlePivot, 4);
	bindCFunctionToJs(setParticleContainerDirection, 4);
	bindCFunctionToJs(setParticleTexture, 2);
	bindCFunctionToJs(setParticleActive, 2);
	bindCFunctionToJs(setParticleTime, 3);
	bindCFunctionToJs(setParticlePosition, 4);
	bindCFunctionToJs(setParticlePositionRange, 7);
	bindCFunctionToJs(setParticleScale, 4);
	bindCFunctionToJs(setParticleScaleRange, 7);
	bindCFunctionToJs(setParticleAngle, 4);
	bindCFunctionToJs(setParticleAngleRange, 7);
	bindCFunctionToJs(setParticlePivot, 4);
	bindCFunctionToJs(setParticleColor, 5);
}
