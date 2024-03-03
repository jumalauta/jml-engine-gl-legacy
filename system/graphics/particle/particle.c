#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/timer/timer.h"
#include "system/debug/debug.h"
#include "system/ui/window/window.h"
#include "system/datatypes/memory.h"
#include "system/datatypes/datatypes.h"
#include "system/datatypes/datatypes.h"
#include "system/math/general/general.h"
#include "particle.h"

/**
 * @defgroup particle Particle handling
 */

/**
 * Deinitialize particle container and free memory
 * @param particleContainer [in] Pointer to particle container.
 * @ingroup particle
 */
void deinitParticleContainer(void *particleContainerPointer)
{
	assert(particleContainerPointer);
	particleContainer_t *particleContainer = (particleContainer_t*)particleContainerPointer;
	
	free(particleContainer->particleList);
	free(particleContainer->particleDefaultTextureList);
}

/**
 * Initialize particle container
 * @param particleContainer [in] Pointer to particle container. NULL creates a new particle container.
 * @return pointer to particle container
 * @ingroup particle
 * @ref JSAPI
 */
particleContainer_t* initParticleContainer(particleContainer_t *particleContainer)
{
	if (particleContainer == NULL)
	{
		particleContainer = memoryAllocateGeneral((void*)particleContainer, sizeof(particleContainer_t), deinitParticleContainer);
		assert(particleContainer);
	}
	
	particleContainer->particleList = NULL;
	particleContainer->particleCount = 0;
	particleContainer->particleDefaultTextureList = NULL;
	particleContainer->particleDefaultTextureCount = 0;
	particleContainer->particleDurationMin = 1.0;
	particleContainer->particleDurationMax = 5.0;
	particleContainer->particleFadeInTime = 0.15;
	particleContainer->particleFadeOutTime = 0.15;

	particleContainer->previousParticleInit = -1.0;
	particleContainer->particleInitDelay = 0.0;
	particleContainer->particleInitCountMax = -1;

	setPoint3d(&particleContainer->positionMin, -1.0, -1.0, -1.0);
	setPoint3d(&particleContainer->positionMax,  1.0, 1.0, 1.0);
	setPoint3d(&particleContainer->position, 0.0, 0.0, 0.0);
	setPoint3d(&particleContainer->particleScaleMin, 1.0, 1.0, 1.0);
	setPoint3d(&particleContainer->particleScaleMax, 1.0, 1.0, 1.0);
	setPoint3d(&particleContainer->particleAngleMin, 0.0, 0.0, 0.0);
	setPoint3d(&particleContainer->particleAngleMax, 0.0, 0.0, 0.0);
	setPoint3d(&particleContainer->particlePivot, 0.0, 0.0, 0.0);
	particleContainer->particleColor.r = 1.0;
	particleContainer->particleColor.g = 1.0;
	particleContainer->particleColor.b = 1.0;
	particleContainer->particleColor.a = 1.0;
	particleContainer->perspective3d = 1;
	particleContainer->startTime = 0.0;
	particleContainer->duration = -1.0;

	particleContainer->initParticle = NULL;
	particleContainer->initParticleClientData = NULL;
	particleContainer->updateParticle = NULL;
	particleContainer->updateParticleClientData = NULL;
	particleContainer->updateParticleContainer = NULL;
	particleContainer->updateParticleContainerClientData = NULL;

	return particleContainer;
}

unsigned int getParticleContainerParticleCount(particleContainer_t *particleContainer)
{
	assert(particleContainer);
	return particleContainer->particleCount;
}

void setParticleContainerPerspective3d(particleContainer_t *particleContainer, int perspective3d)
{
	assert(particleContainer);
	particleContainer->perspective3d = perspective3d;
}

void setParticleContainerDefaultTextureList(particleContainer_t *particleContainer, texture_t **particleDefaultTextureList, unsigned int particleDefaultTextureCount)
{
	assert(particleContainer);
	particleContainer->particleDefaultTextureCount = particleDefaultTextureCount;
	if (particleContainer->particleDefaultTextureList)
	{
		free(particleContainer->particleDefaultTextureList);
	}
	particleContainer->particleDefaultTextureList = (texture_t**)malloc(particleContainer->particleDefaultTextureCount * sizeof(texture_t*));
	assert(particleContainer->particleDefaultTextureList);
	
	unsigned int i;
	for(i = 0; i < particleContainer->particleDefaultTextureCount; i++)
	{
		particleContainer->particleDefaultTextureList[i] = particleDefaultTextureList[i];
	}
}

void setParticleContainerTime(particleContainer_t *particleContainer, float startTime, float duration)
{
	assert(particleContainer);
	particleContainer->startTime = startTime;
	particleContainer->duration = duration;
}
void setParticleContainerParticleDurationRange(particleContainer_t *particleContainer, float particleDurationMin, float particleDurationMax)
{
	assert(particleContainer);
	particleContainer->particleDurationMin = particleDurationMin;
	particleContainer->particleDurationMax = particleDurationMax;
}
void setParticleContainerParticleFadeTimeRange(particleContainer_t *particleContainer, float particleFadeInTime, float particleFadeOutTime)
{
	assert(particleContainer);
	particleContainer->particleFadeInTime = particleFadeInTime;
	particleContainer->particleFadeOutTime = particleFadeOutTime;
}

void setParticleContainerParticleInitDelay(particleContainer_t *particleContainer, float particleInitDelay)
{
	assert(particleContainer);
	particleContainer->particleInitDelay = particleInitDelay;
}

void setParticleContainerParticleInitCountMax(particleContainer_t *particleContainer, int particleInitCountMax)
{
	assert(particleContainer);
	particleContainer->particleInitCountMax = particleInitCountMax;
}

void setParticleContainerPosition(particleContainer_t *particleContainer, point3d_t position)
{
	assert(particleContainer);
	setPoint3d(&particleContainer->position, position.x, position.y, position.z);
}
void setParticleContainerPositionRange(particleContainer_t *particleContainer, point3d_t positionMin, point3d_t positionMax)
{
	assert(particleContainer);
	setPoint3d(&particleContainer->positionMin, positionMin.x, positionMin.y, positionMin.z);
	setPoint3d(&particleContainer->positionMax, positionMax.x, positionMax.y, positionMax.z);
}
void setParticleContainerParticleScaleRange(particleContainer_t *particleContainer, point3d_t particleScaleMin, point3d_t particleScaleMax)
{
	assert(particleContainer);
	setPoint3d(&particleContainer->particleScaleMin, particleScaleMin.x, particleScaleMin.y, particleScaleMin.z);
	setPoint3d(&particleContainer->particleScaleMax, particleScaleMax.x, particleScaleMax.y, particleScaleMax.z);
}
void setParticleContainerParticleAngleRange(particleContainer_t *particleContainer, point3d_t particleAngleMin, point3d_t particleAngleMax)
{
	assert(particleContainer);
	setPoint3d(&particleContainer->particleAngleMin, particleAngleMin.x, particleAngleMin.y, particleAngleMin.z);
	setPoint3d(&particleContainer->particleAngleMax, particleAngleMax.x, particleAngleMax.y, particleAngleMax.z);
}
void setParticleContainerParticleColor(particleContainer_t *particleContainer, color_t particleColor)
{
	assert(particleContainer);
	particleContainer->particleColor.r = particleColor.r;
	particleContainer->particleColor.g = particleColor.g;
	particleContainer->particleColor.b = particleColor.b;
	particleContainer->particleColor.a = particleColor.a;
}
void setParticleContainerParticlePivot(particleContainer_t *particleContainer, point3d_t particlePivot)
{
	assert(particleContainer);
	setPoint3d(&particleContainer->particlePivot, particlePivot.x, particlePivot.y, particlePivot.z);
}
void setParticleContainerDirection(particleContainer_t *particleContainer, point3d_t direction)
{
	assert(particleContainer);
	setPoint3d(&particleContainer->direction, direction.x, direction.y, direction.z);
}

static void setPointRandomValue(point3d_t *dest, point3d_t *base, point3d_t *min, point3d_t *max)
{
	float range = ((rand()%10000)/10000.);
	float x = (max->x - min->x) * range + min->x;

	range = ((rand()%10000)/10000.);
	float y = (max->y - min->y) * range + min->y;

	range = ((rand()%10000)/10000.);
	float z = (max->z - min->z) * range + min->z;

	setPoint3d(dest, base->x+x, base->y+y, base->z+z);
}

static void setParticleDefaultValues(particleContainer_t *particleContainer, particle_t *particle)
{
	assert(particleContainer);
	assert(particle);
	if (particleContainer->particleDefaultTextureCount > 0)
	{
		particle->texture = particleContainer->particleDefaultTextureList[rand()%particleContainer->particleDefaultTextureCount];
	}
	else
	{
		particle->texture = NULL;
	}

	setPoint3d(&particle->startPosition, particleContainer->position.x, particleContainer->position.y, particleContainer->position.z);
	setPointRandomValue(&particle->endPosition, &particle->startPosition, &particleContainer->positionMin, &particleContainer->positionMax);
	setPoint3d(&particle->position, particle->startPosition.x, particle->startPosition.y, particle->startPosition.z);

	setPoint3d(&particle->startScale, particleContainer->particleScaleMin.x, particleContainer->particleScaleMin.y, particleContainer->particleScaleMin.z);
	setPointRandomValue(&particle->endScale, &particle->startScale, &particleContainer->particleScaleMin, &particleContainer->particleScaleMax);
	setPoint3d(&particle->scale, particle->startScale.x, particle->startScale.y, particle->startScale.z);

	setPoint3d(&particle->startAngle, particleContainer->particleAngleMin.x, particleContainer->particleAngleMin.y, particleContainer->particleAngleMin.z);
	setPointRandomValue(&particle->endAngle, &particle->startAngle, &particleContainer->particleAngleMin, &particleContainer->particleAngleMax);
	setPoint3d(&particle->angle, particle->startAngle.x, particle->startAngle.y, particle->startAngle.z);

	setPoint3d(&particle->pivot, particleContainer->particlePivot.x, particleContainer->particlePivot.y, particleContainer->particlePivot.z);

	particle->color.r = particleContainer->particleColor.r;
	particle->color.g = particleContainer->particleColor.g;
	particle->color.b = particleContainer->particleColor.b;
	particle->color.a = particleContainer->particleColor.a;

	particle->active = 0;
	particle->progress = 0.0;
	particle->alpha = 0.0;
	particle->initTime = particle->startTime = timerGetTime();
	
	float particleDurationMin = particleContainer->particleDurationMin;
	float particleDurationMax = particleContainer->particleDurationMax;
	
	float range = ((rand()%10000)/10000.0);
	particle->duration = particleDurationMin+(particleDurationMax-particleDurationMin)*range;
	
	if (particleContainer->initParticle)
	{
		particleContainer->initParticle(particleContainer, particle);
	}
	
	if (particleContainer->duration >= 0.0)
	{
		if (particle->startTime+particle->duration > particleContainer->startTime+particleContainer->duration)
		{
			particle->active = -1;
		}
	}
}

void setParticleTexture(particle_t *particle, texture_t *texture)
{
	assert(particle);
	particle->texture = texture;
}
void setParticleActive(particle_t *particle, int active)
{
	assert(particle);
	particle->active = active;
}
float getParticleProgress(particle_t *particle)
{
	assert(particle);
	return particle->progress;
}
void setParticleTime(particle_t *particle, float startTime, float duration)
{
	assert(particle);
	particle->startTime = startTime;
	particle->duration = duration;
}
void setParticlePosition(particle_t *particle, point3d_t position)
{
	assert(particle);
	setPoint3d(&particle->position, position.x, position.y, position.z);
}
void setParticlePositionRange(particle_t *particle, point3d_t startPosition, point3d_t endPosition)
{
	assert(particle);
	setPoint3d(&particle->startPosition, startPosition.x, startPosition.y, startPosition.z);
	setPoint3d(&particle->endPosition, endPosition.x, endPosition.y, endPosition.z);
}
void setParticleScale(particle_t *particle, point3d_t scale)
{
	assert(particle);
	setPoint3d(&particle->scale, scale.x, scale.y, scale.z);
}
void setParticleScaleRange(particle_t *particle, point3d_t startScale, point3d_t endScale)
{
	assert(particle);
	setPoint3d(&particle->startScale, startScale.x, startScale.y, startScale.z);
	setPoint3d(&particle->endScale, endScale.x, endScale.y, endScale.z);
}
void setParticleAngle(particle_t *particle, point3d_t angle)
{
	assert(particle);
	setPoint3d(&particle->angle, angle.x, angle.y, angle.z);
}
void setParticleAngleRange(particle_t *particle, point3d_t startAngle, point3d_t endAngle)
{
	assert(particle);
	setPoint3d(&particle->startAngle, startAngle.x, startAngle.y, startAngle.z);
	setPoint3d(&particle->endAngle, endAngle.x, endAngle.y, endAngle.z);
}
void setParticlePivot(particle_t *particle, point3d_t pivot)
{
	assert(particle);
	setPoint3d(&particle->pivot, pivot.x, pivot.y, pivot.z);
}
void setParticleColor(particle_t *particle, color_t color)
{
	assert(particle);
	particle->color.r = color.r;
	particle->color.g = color.g;
	particle->color.b = color.b;
	particle->color.a = color.a;
}

/**
 * Initialize particle container
 * @param particleContainer [in] Pointer to particle container. NULL creates a new particle container.
 * @param particleI particle initialize index
 * @param count number of particles
 * @return pointer to particle container
 * @ingroup particle
 * @ref JSAPI
 */
void initParticleContainerParticles(particleContainer_t *particleContainer, unsigned int particleI, unsigned int count)
{
	if (particleContainer->particleCount < particleI+count)
	{
		particleContainer->particleList = (particle_t*)realloc(particleContainer->particleList, sizeof(particle_t)*(particleI+count));
		assert(particleContainer->particleList);
		particleContainer->particleCount = particleI+count;
	}

	int initializedParticles = 0;
	unsigned int i;
	for(i = particleI; i < count; i++)
	{
		
		if (initializedParticles <= particleContainer->particleInitCountMax
				|| particleContainer->particleInitCountMax == -1)
		{
			initializedParticles++;
			setParticleDefaultValues(particleContainer, &particleContainer->particleList[i]);
		}
	}
}

static void interpolatePoint3d(float percentage, point3d_t *dest, point3d_t *a, point3d_t *b)
{
	dest->x = interpolateLinear(percentage, a->x, b->x);
	dest->y = interpolateLinear(percentage, a->y, b->y);
	dest->z = interpolateLinear(percentage, a->z, b->z);
}

static void loadSphericalBillboardMatrix()
{
	float modelViewMatrix44[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix44);

	int i,j;
	for(i=0; i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			if (i==j)
			{
				modelViewMatrix44[i*4+j] = 1.0;
			}
			else
			{
				modelViewMatrix44[i*4+j] = 0.0;
			}
		}
	}

	glLoadMatrixf(modelViewMatrix44);
}

/**
 * Update and draw particle container
 * @param particleContainer [in] Pointer to particle container. NULL creates a new particle container.
 * @ingroup particle
 * @ref JSAPI
 */
void drawParticleContainer(particleContainer_t *particleContainer)
{
	if (particleContainer->updateParticleContainer)
	{
		particleContainer->updateParticleContainer(particleContainer);
	}

	float time = timerGetTime();
	if (particleContainer->duration >= 0.0
		&& time > particleContainer->startTime+particleContainer->duration)
	{
		return;
	}

	glPushMatrix();

	glEnable(GL_BLEND);
	unsigned int primitiveType = GL_POINTS;
	if (particleContainer->particleDefaultTextureCount > 0)
	{
		primitiveType = GL_QUADS;
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		//glBlendFunc(texture->srcBlend, texture->dstBlend);
	}
	else
	{
		glEnable(GL_POINT_SMOOTH);
	}

	if (particleContainer->perspective3d)
	{
		if (primitiveType == GL_QUADS)
		{
			loadSphericalBillboardMatrix();
		}
	}
	else
	{
		perspective2dBegin(getScreenWidth(),getScreenHeight());
	}

	int initializedParticles = 0;
	int particleInitializeAllowed = 0;
	if (time > particleContainer->previousParticleInit + particleContainer->particleInitDelay
		|| particleContainer->previousParticleInit < 0.0)
	{
		particleInitializeAllowed = 1;
		particleContainer->previousParticleInit = time;
	}

	unsigned int i;
	for(i = 0; i < particleContainer->particleCount; i++)
	{
		//do particle update operations
		particle_t *particle = &particleContainer->particleList[i];

		if (particle->initTime > time)
		{
			//this is mainly to allow some decent rewinding capabilities in demo editor
			if ((initializedParticles <= particleContainer->particleInitCountMax)
				|| particleContainer->particleInitCountMax == -1)
			{
				initializedParticles++;
				setParticleDefaultValues(particleContainer, particle);				
			}
		}

		if (particle->active == -1)
		{
			continue;
		}

		float timeFromStart = time-particle->startTime;
		float percentage = timeFromStart/particle->duration;
		if (percentage > 1.0)
		{
			if ((particleInitializeAllowed && initializedParticles <= particleContainer->particleInitCountMax)
				|| particleContainer->particleInitCountMax == -1)
			{
				initializedParticles++;
				setParticleDefaultValues(particleContainer, particle);				
			}
			timeFromStart = time-particle->startTime;
			percentage = timeFromStart/particle->duration;
		}

		if (particle->active == -1)
		{
			continue;
		}

		if (timeFromStart < 0.0)
		{
			continue;
		}
		particle->active = 1;
		particle->progress = percentage;

		float alpha = 1.0;
		if (timeFromStart < particleContainer->particleFadeInTime)
		{
			alpha = timeFromStart/particleContainer->particleFadeInTime;
		}
		else if (timeFromStart >= particle->duration-particleContainer->particleFadeOutTime)
		{
			alpha = 1.0 - (timeFromStart-(particle->duration-particleContainer->particleFadeOutTime))/particleContainer->particleFadeOutTime;
		}
		if (alpha > 1.0) { alpha = 1.0; }
		particle->alpha = alpha;
		
		interpolatePoint3d(percentage, &particle->position, &particle->startPosition, &particle->endPosition);
		interpolatePoint3d(percentage, &particle->scale, &particle->startScale, &particle->endScale);
		interpolatePoint3d(percentage, &particle->angle, &particle->startAngle, &particle->endAngle);
		
		if (particleContainer->updateParticle)
		{
			particleContainer->updateParticle(particleContainer, particle);
		}

		//do drawing operations
		glColor4f(particle->color.r, particle->color.g, particle->color.b, particle->color.a*particle->alpha);
		
		if (primitiveType == GL_POINTS)
		{
			glPointSize((particle->scale.x+particle->scale.y+particle->scale.z)/3.0);
			glBegin(primitiveType);
			glVertex3f(
				particleContainer->position.x+particle->position.x,
				particleContainer->position.y+particle->position.y,
				particleContainer->position.z+particle->position.z);
			glEnd();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, particle->texture->id);
			
			glPushMatrix();
			float w = particle->texture->w;
			float h = particle->texture->h;
			
			if (particleContainer->perspective3d)
			{
				w /= h;
				h = 1.0f;
			}
			
			w *= particle->scale.x;
			h *= particle->scale.y;
			
			double dx = -(w/2.0)+particle->pivot.x;
			double dy = -(h/2.0)+particle->pivot.y;
			float uMin = 0.0;
			float uMax = 1.0;
			float vMin = 0.0;
			float vMax = 1.0;
			glTranslated(particle->position.x-dx, particle->position.y-dy, 0.0);
			glRotated(-particle->angle.z, 0,0,1);
			glBegin(primitiveType);
			glMultiTexCoord2f(GL_TEXTURE0, uMax,vMax);
			glVertex2d(dx+w,dy+h);
			glMultiTexCoord2f(GL_TEXTURE0, uMin,vMax);
			glVertex2d(dx,dy+h);
			glMultiTexCoord2f(GL_TEXTURE0, uMin,vMin);
			glVertex2d(dx, dy);
			glMultiTexCoord2f(GL_TEXTURE0, uMax,vMin);
			glVertex2d(dx+w,dy);
			glEnd();
			glPopMatrix();
		}
	}
	
	if (primitiveType == GL_QUADS)
	{
		glDisable(GL_TEXTURE_2D);
	}
	
	glDisable(GL_BLEND);

	if (!particleContainer->perspective3d)
	{
		perspective2dEnd();
	}

	glPopMatrix();
}
