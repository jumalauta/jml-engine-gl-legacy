#ifndef EXH_SYSTEM_GRAPHICS_PARTICLE_PARTICLE_H_
#define EXH_SYSTEM_GRAPHICS_PARTICLE_PARTICLE_H_

typedef struct {
	texture_t *texture;
	int active;
	float startTime;
	float duration;
	float progress;
	float initTime;
	float alpha;
	point3d_t position;
	point3d_t startPosition;
	point3d_t endPosition;
	point3d_t scale;
	point3d_t startScale;
	point3d_t endScale;
	point3d_t angle;
	point3d_t startAngle;
	point3d_t endAngle;
	point3d_t pivot;
	color_t color;
} particle_t;

typedef struct particleContainer_t particleContainer_t;
struct particleContainer_t {
	particle_t *particleList;
	unsigned int particleCount;
	texture_t **particleDefaultTextureList;
	unsigned int particleDefaultTextureCount;
	float particleDurationMin;
	float particleDurationMax;
	float particleFadeInTime;
	float particleFadeOutTime;
	float particleInitDelay;
	float previousParticleInit;
	int particleInitCountMax;
	point3d_t positionMin;
	point3d_t positionMax;
	point3d_t position;
	point3d_t particleScaleMin;
	point3d_t particleScaleMax;
	point3d_t particleAngleMin;
	point3d_t particleAngleMax;
	point3d_t particlePivot;
	point3d_t direction;
	color_t particleColor;
	float startTime;
	float duration;
	int perspective3d;
	void (*initParticle)(particleContainer_t*, particle_t*);
	void *initParticleClientData;
	void (*updateParticle)(particleContainer_t*, particle_t*);
	void *updateParticleClientData;
	void (*updateParticleContainer)(particleContainer_t*);
	void *updateParticleContainerClientData;
};

extern void deinitParticleContainer(void *particleContainerPointer);
extern particleContainer_t* initParticleContainer(particleContainer_t *particleContainer);
extern void initParticleContainerParticles(particleContainer_t *particleContainer, unsigned int particleI, unsigned int count);
extern void drawParticleContainer(particleContainer_t *particleContainer);

extern unsigned int getParticleContainerParticleCount(particleContainer_t *particleContainer);
extern void setParticleContainerPerspective3d(particleContainer_t *particleContainer, int perspective3d);
extern void setParticleContainerDefaultTextureList(particleContainer_t *particleContainer, texture_t **particleDefaultTextureList, unsigned int particleDefaultTextureCount);
extern void setParticleContainerTime(particleContainer_t *particleContainer, float startTime, float duration);
extern void setParticleContainerParticleDurationRange(particleContainer_t *particleContainer, float particleDurationMin, float particleDurationMax);
extern void setParticleContainerParticleFadeTimeRange(particleContainer_t *particleContainer, float particleFadeInTime, float particleFadeOutTime);
extern void setParticleContainerParticleInitDelay(particleContainer_t *particleContainer, float particleInitDelay);
extern void setParticleContainerParticleInitCountMax(particleContainer_t *particleContainer, int particleInitCountMax);

extern void setParticleContainerPosition(particleContainer_t *particleContainer, point3d_t position);
extern void setParticleContainerPositionRange(particleContainer_t *particleContainer, point3d_t positionMin, point3d_t positionMax);
extern void setParticleContainerParticleScaleRange(particleContainer_t *particleContainer, point3d_t particleScaleMin, point3d_t particleScaleMax);
extern void setParticleContainerParticleAngleRange(particleContainer_t *particleContainer, point3d_t particleAngleMin, point3d_t particleAngleMax);
extern void setParticleContainerParticleColor(particleContainer_t *particleContainer, color_t particleColor);
extern void setParticleContainerParticlePivot(particleContainer_t *particleContainer, point3d_t particlePivot);
extern void setParticleContainerDirection(particleContainer_t *particleContainer, point3d_t direction);

extern void setParticleTexture(particle_t *particle, texture_t *texture);
extern void setParticleActive(particle_t *particle, int active);
extern float getParticleProgress(particle_t *particle);
extern void setParticleTime(particle_t *particle, float startTime, float duration);
extern void setParticlePosition(particle_t *particle, point3d_t position);
extern void setParticlePositionRange(particle_t *particle, point3d_t startPosition, point3d_t endPosition);
extern void setParticleScale(particle_t *particle, point3d_t scale);
extern void setParticleScaleRange(particle_t *particle, point3d_t startScale, point3d_t endScale);
extern void setParticleAngle(particle_t *particle, point3d_t angle);
extern void setParticleAngleRange(particle_t *particle, point3d_t startAngle, point3d_t endAngle);
extern void setParticlePivot(particle_t *particle, point3d_t pivot);
extern void setParticleColor(particle_t *particle, color_t color);

#endif /*EXH_SYSTEM_GRAPHICS_PARTICLE_PARTICLE_H_*/
