#include <assert.h>

#include "graphicsIncludes.h"
#include "system/ui/window/window.h"
#include "system/datatypes/memory.h"

#include "texture.h"
 
/**
 * @defgroup texture Texture functionality
 */

/**
 * Initialize texture and set to default values
 * @param[in] texture pointer to texture, if NULL then new texture will be created
 * @return pointer to texture
 * @ingroup texture
 */
texture_t* textureInit(texture_t *texture)
{
	int init = 0;
	if (texture == NULL)
	{
		texture = (texture_t*)malloc(sizeof(texture_t));
		init = 1;
	}
	
	texture->name = "";
	texture->w = texture->h = texture->customWidth = texture->customHeight = texture->center = texture->id = texture->hasAlpha = texture->hasCustomDimensions = 0;
	texture->x = texture->y = texture->z
		= texture->pivotX = texture->pivotY = texture->pivotZ
		= texture->scaleW = texture->scaleH = 0.0;
	texture->angleX = texture->angleY = texture->angleZ
		= texture->degreesX = texture->degreesY = texture->degreesZ = 0.0;
	texture->srcBlend = GL_SRC_ALPHA;
	texture->dstBlend = GL_ONE_MINUS_SRC_ALPHA;
	
	texture->canvasWidth = getScreenWidth();
	texture->canvasHeight = getScreenHeight();
	
	texture->uMin = texture->vMin = 0.0;
	texture->uMax = texture->vMax = 1.0;
	
	texture->perspective3d = 0;

	int i;
	texture->multiTextureId[0] = texture->id;
	for(i=1; i < MAX_TEXTURE_UNITS; i++)
	{
		texture->multiTextureId[i] = 0;
	}
	
	setTextureDefaults(texture);

	if (init)
	{
		memoryAllocateTexture(texture);
	}

	return texture;
}

/**
 * Deinitialize texture and free memory
 * @param[in] texture pointer to texture
 * @ingroup texture
 */
void textureDeinit(texture_t *texture)
{
	assert(texture);
	if (texture->id == 0) {
		return;
	}

	if (texture->name)
	{
		debugPrintf("Cleaning '%s' (%p)", texture->name, texture);
	}

	if (texture->name)
	{
		free(texture->name);
	}

	glDeleteTextures(1, &texture->id);
	texture->id = 0;
}


/**
 * Set texture perspective to 2D or 3D
 * @param texture [in] pointer to texture
 * @param perspective3d 1 if image is displayed in 3D space, 0 if 2D
 * @ingroup texture
 * @ref JSAPI
 */
void setTexturePerspective3d(texture_t *texture, int perspective3d)
{
	assert(texture != NULL);

	texture->perspective3d = perspective3d;
}

/**
 * Set default blending mode for the texture
 * @param[in] texture pointer to texture
 * @param srcBlend source blending factor, default is GL_SRC_ALPHA
 * @param dstBlend destination blending factor, default is GL_ONE_MINUS_SRC_ALPHA
 * @ingroup texture
 * @see <a href="https://www.opengl.org/sdk/docs/man2/xhtml/glBlendFunc.xml">glBlendFunc OpenGL documentation</a>
 * @ref JSAPI
 */
void setTextureBlendFunc(texture_t *texture, unsigned int srcBlend, unsigned int dstBlend)
{
	assert(texture != NULL);

	texture->srcBlend = srcBlend;
	texture->dstBlend = dstBlend;
}

/**
 * Set custom dimensions to the texture
 * @param[in] texture pointer to texture
 * @param w new width
 * @param w new height
 * @ingroup texture
 * @deprecated This might be removed in the future
 */
void setCustomDimensionToTexture(texture_t *texture, int w, int h)
{
	assert(texture != NULL);

	texture->hasCustomDimensions = 1;
	if (0 == w)
	{
		texture->customWidth = texture->w;
	}
	else
	{
		texture->customWidth = w;
	}

	if (0 == h)
	{
		texture->customHeight = texture->h;
	}
	else
	{
		texture->customHeight = h;
	}
	
	if (0 == w && 0 == h)
	{
		texture->hasCustomDimensions = 0;
	}
}

/**
 * Set texture's canvas dimensions
 * @param texture [in] pointer to texture
 * @param w width, default is getScreenWidth()
 * @param w height, default is getScreenHeight()
 * @ingroup texture
 */
void setTextureCanvasDimensions(texture_t *texture, int w, int h)
{
	assert(texture != NULL);

	texture->canvasWidth = w;
	texture->canvasHeight = h;
}

/**
 * Set texture UV dimensions
 * @param[in] texture pointer to texture
 * @param uMin U minimum
 * @param vMin V minimum
 * @param uMax U maximum
 * @param vMax V maximum
 * @ingroup texture
 * @ref JSAPI
 */
void setTextureUvDimensions(texture_t *texture, double uMin, double vMin, double uMax, double vMax)
{
	assert(texture != NULL);
	texture->uMin = uMin;
	texture->vMin = vMin;
	texture->uMax = uMax;
	texture->vMax = vMax;
}

/**
 * Set texture dimensions to screen dimensions
 * @param[in] texture pointer to texture
 * @ingroup texture
 * @ref JSAPI
 */
void setTextureSizeToScreenSize(texture_t *texture)
{
	assert(texture != NULL);

	setCustomDimensionToTexture(texture, getScreenWidth(), getScreenHeight());
	//texture->w = getScreenWidth();
	//texture->h = getScreenHeight();
}

/**
 * Align texture by centering, horizontally or vertically
 * @param[in] texture pointer to texture
 * @param center alignment mode: 1 = centered, 2 = horizontal, 3 = vertical
 * @ingroup texture
 * @ref JSAPI
 */
void setTextureCenterAlignment(texture_t *texture, int center)
{
	assert(texture != NULL);

	texture->center = center;
}

/**
 * Set texture position
 * @param texture [in] pointer to texture
 * @param x position X
 * @param y position Y
 * @param z position Z
 * @ingroup texture
 * @ref JSAPI
 */
void setTexturePosition(texture_t *texture, double x, double y, double z)
{
	assert(texture != NULL);

	texture->x = x;
	texture->y = y;
	texture->z = z;

	if (texture->perspective3d == 0)
	{
		texture->z = 0;
	}
}

/**
 * Set texture pivot in pixels
 * @param[in] texture pointer to texture
 * @param x pivot position X
 * @param y pivot position Y
 * @param z pivot position Z
 * @ingroup texture
 * @ref JSAPI
 */
void setTexturePivot(texture_t *texture, double x, double y, double z)
{
	assert(texture != NULL);

	texture->pivotX = x;
	texture->pivotY = y;
	texture->pivotZ = z;

	if (texture->perspective3d == 0)
	{
		texture->pivotZ = 0;
	}
}

/**
 * Scale texture size
 * @param[in] texture pointer to texture
 * @param scaleW width, 1.0 is default
 * @param scaleH height, 1.0 is default
 * @ingroup texture
 * @ref JSAPI
 * @warning Name refactoring pending
 */
void setTextureScale(texture_t *texture, double scaleW, double scaleH)
{
	assert(texture != NULL);

	texture->scaleW = scaleW;
	texture->scaleH = scaleH;
}

/**
 * Set texture rotation angle
 * @param texture [in] pointer to texture
 * @param degreesX X in degrees
 * @param degreesY Y in degrees
 * @param degreesZ Z in degrees
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 * @ingroup texture
 * @ref JSAPI
 */
void setTextureRotation(texture_t* texture, double degreesX, double degreesY, double degreesZ, double x, double y, double z)
{
	assert(texture != NULL);

	texture->degreesX = degreesX;
	texture->degreesY = degreesY;
	texture->degreesZ = degreesZ;

	texture->angleX = x;
	texture->angleY = y;
	texture->angleZ = z;
	
	if (!texture->perspective3d)
	{
		texture->degreesX = 0;
		texture->degreesY = 0;
		texture->angleX = 0;
		texture->angleY = 0;
	}
}

/**
 * Sets texture units to enable multitexturing
 * @param texture [in] pointer to source texture
 * @param unitIndex Texture unit index number, value should be in range 0 ... MAX_TEXTURE_UNITS-1
 * @param textureDst [in] pointer to destination texture which will be assigned to the texture unit
 * @ingroup texture
 * @ref JSAPI
 */
void setTextureUnitTexture(texture_t *texture, unsigned int unitIndex, texture_t *textureDst)
{
	assert(texture != NULL);
	assert(textureDst != NULL);
	assert(unitIndex < MAX_TEXTURE_UNITS);

	texture->multiTextureId[unitIndex] = textureDst->id;
}

/**
 * Set texture to default values
 * @param[in] texture pointer to texture
 * @ingroup texture
 * @ref JSAPI
 */
void setTextureDefaults(texture_t *texture)
{
	assert(texture != NULL);

	setTextureBlendFunc(texture, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setCustomDimensionToTexture(texture, 0, 0);
	setTextureCenterAlignment(texture, 0);
	setTexturePosition(texture, 0.0, 0.0, 0.0);
	setTexturePivot(texture, 0.0, 0.0, 0.0);
	setTextureScale(texture, 1.0, 1.0);
	setTextureRotation(texture, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

/**
 * Display image in 2D perspective
 * @param texture [in] pointer to texture
 * @ingroup texture
 * @ref JSAPI
 * @warning Name refactoring pending
 */
void drawTexture(texture_t *texture)
{
	//glBlendFuncSeparate(texture->srcBlend, texture->dstBlend, GL_ONE, GL_SRC_ALPHA);
	glBlendFunc(texture->srcBlend, texture->dstBlend);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	int i;
	for(i=MAX_TEXTURE_UNITS-1; i >= 0; i--)
	{
		unsigned int id = texture->multiTextureId[i];
		if (id == 0)
		{
			continue;
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture->multiTextureId[i]);
	}

	//glBindTexture(GL_TEXTURE_2D, texture->id);
	if (!texture->perspective3d)
	{
		perspective2dBegin((int)texture->canvasWidth,(int)texture->canvasHeight);
	}

	double w = texture->customWidth;
	double h = texture->customHeight;
	if (texture->perspective3d)
	{
		w = w/h;
		h = 1.0;
		
		/*x -= w/2.0;
		y -= h/2.0;*/
	}

	double xFixed = -w/2.0*texture->scaleW;
	double yFixed = -h/2.0*texture->scaleH;

	double x = xFixed + texture->x;
	double y = yFixed + texture->y;
	double z = texture->z;



	if (!texture->perspective3d)
	{
		switch (texture->center)
		{
			case 1:
				x += (texture->canvasWidth/2.0);
				y += (texture->canvasHeight/2.0);
				break;
			case 2:
				x += (texture->canvasWidth/2.0);
				break;
			case 3:
				y += (texture->canvasHeight/2.0);
				break;
			case 4:
				x -= xFixed;
				break;
			case 5:
				x += texture->canvasWidth+xFixed;
				break;
			default:
				break;
		}
	}
	
	glPushMatrix();
	double pivotX = (w/2.0+texture->pivotX/texture->scaleW);
	double pivotY = (h/2.0+texture->pivotY/texture->scaleH);
	double pivotZ = texture->pivotZ;

	glTranslated(x, y, z);

	glScalef(texture->scaleW, texture->scaleH, 1.0);

	glTranslated(pivotX, pivotY, pivotZ);

	glRotated(texture->degreesX, -texture->angleX,                0,                0);
	glRotated(texture->degreesY,                0, -texture->angleY,                0);
	glRotated(texture->degreesZ,                0,                0, -texture->angleZ);

	glTranslated(-pivotX, -pivotY, -pivotZ);

	glBegin(GL_QUADS);
	glMultiTexCoord2f(GL_TEXTURE0, texture->uMax,texture->vMax);
	glVertex3d(w,h,0);
	glMultiTexCoord2f(GL_TEXTURE0, texture->uMin,texture->vMax);
	glVertex3d(0,h,0);
	glMultiTexCoord2f(GL_TEXTURE0, texture->uMin,texture->vMin);
	glVertex3d(0,0,0);
	glMultiTexCoord2f(GL_TEXTURE0, texture->uMax,texture->vMin);
	glVertex3d(w,0,0);
	glEnd();
	glPopMatrix();

	if (!texture->perspective3d)
	{
		perspective2dEnd();
	}
	//glBindTexture(GL_TEXTURE_2D,0);
	for(i=MAX_TEXTURE_UNITS-1; i >= 0; i--)
	{
		unsigned int id = texture->multiTextureId[i];
		if (id == 0)
		{
			continue;
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}
