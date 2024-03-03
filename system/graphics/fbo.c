#include <assert.h>
#include "graphicsIncludes.h"
#include "graphics.h"
#include "system/debug/debug.h"
#include "system/ui/window/window.h"
#include "system/datatypes/memory.h"

#ifdef SUPPORT_GL_FBO

/**
 * @defgroup fbo Frame Buffer Object (FBO)
 */

/**
 * Bind framebuffer to FBO.
 * @param[in] fbo Pointer to fbo. NULL binds to default framebuffer.
 * @ingroup fbo
 * @ref JSAPI
 */
void fboBind(fbo_t* fbo)
{
	GLuint fb = 0;
	if (fbo)
	{
		fb = fbo->id;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
}

static void fboUpdateTextureUvDimensions(fbo_t* fbo)
{
	assert(fbo);

	if (fbo->color)
	{
		setTextureUvDimensions(fbo->color, 0.0, 0.0, fbo->renderWidthPercent, fbo->renderHeightPercent);
	}
	if (fbo->depth)
	{
		setTextureUvDimensions(fbo->depth, 0.0, 0.0, fbo->renderWidthPercent, fbo->renderHeightPercent);
	}
}

/**
 * Initialize new FBO or retrieve existing FBO.
 * @param name [in] Name of the FBO. If FBO is not found with the given name then new FBO is created.
 * @return Pointer to fbo.
 * @ingroup fbo
 * @ref JSAPI
 */
fbo_t* fboInit(const char *name)
{
	fbo_t *fbo = getFboFromMemory(name);
	if (fbo)
	{
		return fbo;
	}

	fbo = memoryAllocateFbo(NULL);

	fbo->name = strdup(name);
	assert(fbo->name);

	fbo->id = 0;
	fbo->color = NULL;
	fbo->colorTextureType = GL_RGBA;
	fbo->depth = NULL;
	fbo->depthTextureType = GL_DEPTH_COMPONENT;
	fbo->storeDepth = 0;
	fbo->depthBuffer = 0;
	
	fboSetDimensions(fbo, getScreenWidth(), getScreenHeight());
	fboSetRenderDimensions(fbo, 1.0, 1.0);
	
	return fbo;
}

static void fboCreateTexture(fbo_t* fbo, texture_t** texture, unsigned int textureType, const char *type)
{
	assert(fbo);

	if (*texture)
	{
		textureDeinit(*texture);
	}

	size_t stringLength = strlen(fbo->name)+strlen(type)+6;
	char *fboTextureName = (char*)malloc(stringLength*sizeof(char));
	snprintf(fboTextureName, stringLength, "%s.%s.fbo", fbo->name, type);

	*texture = imageCreateTexture((const char*)fboTextureName, 0, textureType, fboGetWidth(fbo), fboGetHeight(fbo));
	setTextureCenterAlignment(*texture, 1);

	free(fboTextureName);
}

/**
 * Create FBO textures and the framebuffer object
 * @param fbo [in] Pointer to fbo.
 * @return 1 if OK, 0 if not OK
 * @ingroup fbo
 * @ref JSAPI
 */
int fboGenerateFramebuffer(fbo_t* fbo)
{
	assert(fbo);

	debugPrintf("Initializing FBO '%s' (%dx%d)", fbo->name, fbo->width, fbo->height);

	fboCreateTexture(fbo, &fbo->color, fbo->colorTextureType, "color");
	
	if (fbo->storeDepth)
	{
		fboCreateTexture(fbo, &fbo->depth, fbo->depthTextureType, "depth");
	}

	fboUpdateTextureUvDimensions(fbo);
	
	if (fbo->id != 0)
	{
		glDeleteFramebuffers(1, &fbo->id);
	}
	glGenFramebuffers(1, &fbo->id);

	fboBind(fbo);

	if (fbo->color)
	{
		//depth buffer is needed so that depth testing will work with 3D objects
		glGenRenderbuffers(1, &fbo->depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo->depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fbo->width, fbo->height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->depthBuffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color->id, 0);
	}
	if (fbo->depth)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, fbo->depth->id, 0);
	}

	GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	fboBind(NULL);

	if (e != GL_FRAMEBUFFER_COMPLETE)
	{
		debugErrorPrintf("Issue with FBO creation! '%X'", e);
		return 0;
	}

	return 1;
}

/**
 * Set render dimension percent relative to whole FBO dimensions
 * @param fbo [in] Pointer to fbo.
 * @param width FBO texture width
 * @param height FBO texture height
 * @ingroup fbo
 * @ref JSAPI
 */
void fboSetDimensions(fbo_t* fbo, unsigned int width, unsigned int height)
{
	assert(fbo);
	fbo->width = width;
	fbo->height = height;
}

/**
 * Set render dimension percent relative to whole FBO dimensions
 * @param fbo [in] Pointer to fbo.
 * @param widthPercent Percentual width
 * @param heightPercent Percentual height
 * @ingroup fbo
 * @ref JSAPI
 */
void fboSetRenderDimensions(fbo_t* fbo, double widthPercent, double heightPercent)
{
	assert(fbo);
	assert(widthPercent > 0.0 && widthPercent <= 1.0);
	assert(heightPercent > 0.0 && heightPercent <= 1.0);

	fbo->renderWidthPercent = widthPercent;
	fbo->renderHeightPercent = heightPercent;
	
	fboUpdateTextureUvDimensions(fbo);
}

/**
 * Get FBO render width
 * @param fbo [in] Pointer to fbo.
 * @ingroup fbo
 * @ref JSAPI
 */
int fboGetWidth(fbo_t* fbo)
{
	assert(fbo);
	return fbo->width * fbo->renderWidthPercent;
}

/**
 * Get FBO render width
 * @param fbo [in] Pointer to fbo.
 * @ingroup fbo
 * @ref JSAPI
 */
int fboGetHeight(fbo_t* fbo)
{
	assert(fbo);
	return fbo->height * fbo->renderHeightPercent;
}

/**
 * Updates viewport by setting projection and model matrices according to the FBO details.
 * @param fbo [in] Pointer to fbo. NULL binds to default framebuffer.
 * @ingroup fbo
 * @ref JSAPI
 */
void fboUpdateViewport(fbo_t* fbo)
{
	setWindowFbo(fbo);
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	viewReset();
}

/**
 * Deinitialize and clean FBO
 * @param fbo [in] Pointer to fbo.
 * @ingroup fbo
 * @ref JSAPI
 */
void fboDeinit(fbo_t* fbo)
{
	assert(fbo);

	debugPrintf("Deinitializing FBO '%s'", fbo->name);

	fboBind(NULL);

	free(fbo->name);
	glDeleteRenderbuffers(1, &fbo->depthBuffer);
	glDeleteFramebuffers(1, &fbo->id);
}

/**
 * Store depth data in FBO
 * @param fbo [in] Pointer to fbo.
 * @param _storeDepth 1 if depth is stored, 0 if depth is not stored. Default is 0 in FBOs.
 * @ingroup fbo
 * @ref JSAPI
 */
void fboStoreDepth(fbo_t* fbo, int _storeDepth)
{
	assert(fbo);
	
	fbo->storeDepth = _storeDepth;
}


/**
 * Binds FBO textures to texture units. Texture unit 0 is the FBO color data. Texture unit 1 is the FBO depth data.
 * @param[in] fbo Pointer to fbo. NULL resets texture unit bindings.
 * @ingroup fbo
 * @ref JSAPI
 */
void fboBindTextures(fbo_t* fbo)
{
	if (fbo)
	{
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		unsigned int depthId = 0;
		if (fbo->depth)
		{
			depthId = fbo->depth->id;
		}
		glBindTexture(GL_TEXTURE_2D, depthId);

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, fbo->color->id);
		unsigned int colorId = 0;
		if (fbo->color)
		{
			colorId = fbo->color->id;
		}
		glBindTexture(GL_TEXTURE_2D, colorId);
	}
	else //unbind textures incase NULL was given as the argument
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

#endif

