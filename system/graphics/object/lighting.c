#include <assert.h>

#include "graphicsIncludes.h"
#include "system/debug/debug.h"
#include "lighting.h"

#define MAX_LIGHTS 8
static light_t lighting[MAX_LIGHTS];

void setLight4f(unsigned int light, unsigned int type, float f1, float f2, float f3, float f4)
{
	GLfloat floats[] = { f1, f2, f3, f4 };
	glLightfv(light, type, floats); 
}

void setLight4ub(unsigned int light, unsigned int type, unsigned int f1, unsigned int f2, unsigned int f3, unsigned int f4)
{
	setLight4f(light, type, f1/255.0f, f2/255.0f, f3/255.0f, f4/255.0f);
}

void lightCalculate(light_t *light)
{
	assert(light);

	float posX = light->position.x;
	float posY = light->position.y;
	float posZ = light->position.z;
	if (light->positionObject)
	{
		posX += light->positionObject->position.x;
		posY += light->positionObject->position.y;
		posZ += light->positionObject->position.z;
	}
	setLight4f(light->id, GL_POSITION, posX, posY, posZ, 0.0f);
	setLight4f(light->id, GL_SPOT_DIRECTION, light->direction.x, light->direction.y, light->direction.z, 0.0f);

	setLight4f(light->id, GL_AMBIENT, light->ambient.r, light->ambient.g, light->ambient.b, light->ambient.a);
	setLight4f(light->id, GL_DIFFUSE, light->diffuse.r, light->diffuse.g, light->diffuse.b, light->diffuse.a);
	setLight4f(light->id, GL_SPECULAR, light->specular.r, light->specular.g, light->specular.b, light->specular.a);

	glLightf(light->id, GL_CONSTANT_ATTENUATION, light->constantAttenuation);
	glLightf(light->id, GL_LINEAR_ATTENUATION, light->linearAttenuation);
	glLightf(light->id, GL_QUADRATIC_ATTENUATION, light->quadricAttenuation);
	
	glLightf(light->id, GL_SPOT_EXPONENT, light->spotExponent);
	glLightf(light->id, GL_SPOT_CUTOFF, light->spotCutOff);
}

//Default values based on https://www.opengl.org/sdk/docs/man2/xhtml/glLight.xml
void lightSetDefaults(light_t *light)
{
	assert(light);

	light->enabled = 0;
	
	light->positionObject = NULL;

	light->position.x = 0;
	light->position.y = 0;
	light->position.z = 1;
	
	light->direction.x = 0;
	light->direction.y = 0;
	light->direction.z = -1;
	
	light->constantAttenuation = 1.0f;
	light->linearAttenuation = 0.0f;
	light->quadricAttenuation = 0.0f;
	
	light->spotExponent = 0.0f;
	light->spotCutOff = 180.0f;

	float defaultValue = 0.0f;
	const float defaultAlphaValue = 1.0f;

	light->ambient.r = defaultValue;
	light->ambient.g = defaultValue;
	light->ambient.b = defaultValue;
	light->ambient.a = defaultAlphaValue;
	
	if (light->id == GL_LIGHT0)
	{
		defaultValue = 1.0f;
	}
	
	light->diffuse.r = defaultValue;
	light->diffuse.g = defaultValue;
	light->diffuse.b = defaultValue;
	light->diffuse.a = defaultAlphaValue;
	
	light->specular.r = defaultValue;
	light->specular.g = defaultValue;
	light->specular.b = defaultValue;
	light->specular.a = defaultAlphaValue;
}

static light_t* getLightPtr(unsigned int i)
{
	if (i >= MAX_LIGHTS)
	{
		debugErrorPrintf("Incorrect light id '%d'! Maximum lights:'%d'", i, MAX_LIGHTS);
		return NULL;
	}

	return &lighting[i];
}

static void lightSetColor(color_t *color, float r, float g, float b, float a)
{
	assert(color);

	color->r = r;
	color->g = g;
	color->b = b;
	color->a = a;
}

void lightSetAmbientColor(unsigned int i, float r, float g, float b, float a)
{
	light_t *light = getLightPtr(i);
	assert(light);
	
	lightSetColor(&light->ambient, r, g, b, a);
}

void lightSetDiffuseColor(unsigned int i, float r, float g, float b, float a)
{
	light_t *light = getLightPtr(i);
	assert(light);
	
	lightSetColor(&light->diffuse, r, g, b, a);
}

void lightSetSpecularColor(unsigned int i, float r, float g, float b, float a)
{
	light_t *light = getLightPtr(i);
	assert(light);
	
	lightSetColor(&light->specular, r, g, b, a);
}

void lightSetPosition(unsigned int i, float x, float y, float z)
{
	light_t *light = getLightPtr(i);
	assert(light);
	
	light->position.x = x;
	light->position.y = y;
	light->position.z = z;
}

void lightSetPositionObject(unsigned int i, object3d_t *positionObject)
{
	light_t *light = getLightPtr(i);
	assert(light);
	
	light->positionObject = positionObject;
}

void lightInit(unsigned int i)
{
	light_t *light = getLightPtr(i);
	assert(light);

	light->id = GL_LIGHT0 + i;

	lightSetDefaults(light);
}

void lightSetOn(unsigned int i)
{
	light_t *light = getLightPtr(i);
	assert(light);

	light->enabled = 1;

	glEnable(GL_LIGHTING);
	
	glEnable(light->id);
	lightCalculate(light);

	/*debugPrintf("Light id:'0x%X', enabled:'%d', positionObj:'%p'\n" \
		"\tx:%.2f, y:%.2f, z:%.2f\n" \
		"\tambient  r:%.2f, g:%.2f, b:%.2f, a:%.2f\n" \
		"\tdiffuse  r:%.2f, g:%.2f, b:%.2f, a:%.2f\n" \
		"\tspecular r:%.2f, g:%.2f, b:%.2f, a:%.2f",
		light->id, light->enabled, light->positionObject,
		light->position.x, light->position.y, light->position.z,
		light->ambient.r, light->ambient.g, light->ambient.b, light->ambient.a,
		light->diffuse.r, light->diffuse.g, light->diffuse.b, light->diffuse.a,
		light->specular.r, light->specular.g, light->specular.b, light->specular.a
	);*/
}

void lightSetOff(unsigned int i)
{
	light_t *light = getLightPtr(i);
	assert(light);
	light->enabled = 0;

	glDisable(light->id);
	

	light_t lightDefault;
	lightDefault.id = light->id;
	lightSetDefaults(&lightDefault);
	lightCalculate(&lightDefault);
	
	if (!isLightingEnabled())
	{
		glDisable(GL_LIGHTING);
	}
}

void lightingInit()
{
	debugPrintf("Initializing lighting");

	int i;
	for(i = 0; i < MAX_LIGHTS; i++)
	{
		lightInit(i);
	}
}

int isLightingEnabled()
{
	int i;
	for(i = 0; i < MAX_LIGHTS; i++)
	{
		if (lighting[i].enabled)
		{
			return 1;
		}
	}

	return 0;
}
