#ifndef EXH_SYSTEM_GRAPHICS_OBJECT_LIGHTING_H_
#define EXH_SYSTEM_GRAPHICS_OBJECT_LIGHTING_H_

#include "system/graphics/object/object3d.h"

typedef struct {
	unsigned int id;
	int enabled;
	point3d_t position;
	point3d_t direction;
	float constantAttenuation;
	float linearAttenuation;
	float quadricAttenuation;
	float spotExponent;
	float spotCutOff;
	color_t ambient;
	color_t diffuse;
	color_t specular;
	object3d_t *positionObject;
} light_t;

extern void setLight4f(unsigned int light, unsigned int type, float f1, float f2, float f3, float f4);
extern void setLight4ub(unsigned int light, unsigned int type, unsigned int f1, unsigned int f2, unsigned int f3, unsigned int f4);

extern void lightSetDefaults(light_t *light);
extern void lightCalculate(light_t *light);
extern void lightingInit();
extern void lightSetAmbientColor(unsigned int i, float r, float g, float b, float a);
extern void lightSetDiffuseColor(unsigned int i, float r, float g, float b, float a);
extern void lightSetSpecularColor(unsigned int i, float r, float g, float b, float a);
extern void lightSetPosition(unsigned int i, float x, float y, float z);
extern void lightSetPositionObject(unsigned int i, object3d_t *positionObject);
extern void lightInit(unsigned int i);
extern void lightSetOn(unsigned int i);
extern void lightSetOff(unsigned int i);
extern int isLightingEnabled();

#endif /*EXH_SYSTEM_GRAPHICS_OBJECT_LIGHTING_H_*/
