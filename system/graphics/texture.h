#ifndef EXH_SYSTEM_GRAPHICS_TEXTURE_H_
#define EXH_SYSTEM_GRAPHICS_TEXTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "graphicsIncludes.h"

#include "system/math/general/general.h"

#define MAX_TEXTURE_UNITS 4
typedef struct {
	char* name;
	unsigned int w, h, customWidth, customHeight, center, id, hasAlpha, hasCustomDimensions, canvasWidth, canvasHeight;
	double x, y, z, pivotX, pivotY, pivotZ, scaleW, scaleH;
	double angleX, angleY, angleZ, degreesX, degreesY, degreesZ;
	double uMin, vMin, uMax, vMax;
	int perspective3d;
	unsigned int srcBlend, dstBlend;
	unsigned int multiTextureId[MAX_TEXTURE_UNITS];
} texture_t;

extern texture_t* textureInit(texture_t *texture);
extern void textureDeinit(texture_t *texture);
extern void setTexturePerspective3d(texture_t *texture, int perspective3d);
extern void setTextureBlendFunc(texture_t *texture, unsigned int srcBlend, unsigned int dstBlend);
extern void setCustomDimensionToTexture(texture_t *texture, int w, int h);
extern void setTextureCanvasDimensions(texture_t *texture, int w, int h);
extern void setTextureUvDimensions(texture_t *texture, double uMin, double vMin, double uMax, double vMax);
extern void setTextureCenterAlignment(texture_t *texture, int center);
extern void setTexturePosition(texture_t *texture, double x, double y, double z);
extern void setTexturePivot(texture_t *texture, double x, double y, double z);
extern void setTextureScale(texture_t *texture, double scaleW, double scaleH);
extern void setTextureSizeToScreenSize(texture_t *texture);
extern void setTextureRotation(texture_t* texture, double degreesX, double degreesY, double degreesZ, double x, double y, double z);
extern void setTextureUnitTexture(texture_t *texture, unsigned int unitIndex, texture_t *textureDst);
extern void setTextureDefaults(texture_t *texture);
extern void drawTexture(texture_t *texture);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*EXH_SYSTEM_GRAPHICS_TEXTURE_H_*/
