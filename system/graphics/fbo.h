#ifndef EXH_SYSTEM_GRAPHICS_FBO_H_
#define EXH_SYSTEM_GRAPHICS_FBO_H_

/**
 * Frame Buffer Object (FBO) information
 */
typedef struct {
	/**
	 * FBO color data texture pointer
	 */
	texture_t *color;
	/**
	 * Color texture's type - GL_RGB or GL_RGBA.
	 */
	unsigned int colorTextureType;
	/**
	 * FBO depth data texture pointer
	 */
	texture_t *depth;
	/**
	 * Depth texture's type - GL_DEPTH_COMPONENT
	 */
	unsigned int depthTextureType;
	/**
	 * Depth buffer thing
	 */
	GLuint depthBuffer;
	/**
	 * Store depth when rendering to FBO
	 */
	int storeDepth;
	/**
	 * FBO width
	 */
	int width;
	/**
	 * FBO height
	 */
	int height;
	/**
	 * FBO render Width percent
	 */
	double renderWidthPercent;
	/**
	 * FBO height
	 */
	double renderHeightPercent;
	/**
	 * FBO ID
	 */
	GLuint id;
	/**
	 * FBO logical name
	 */
	char *name;
} fbo_t;

extern void fboBind(fbo_t* fbo);
extern fbo_t* fboInit(const char *name);
extern void fboDeinit(fbo_t* fbo);
extern void fboStoreDepth(fbo_t* fbo, int _storeDepth);
extern int fboGenerateFramebuffer(fbo_t* fbo);
extern void fboSetDimensions(fbo_t* fbo, unsigned int width, unsigned int height);
extern void fboSetRenderDimensions(fbo_t* fbo, double widthPercent, double heightPercent);
extern int fboGetWidth(fbo_t* fbo);
extern int fboGetHeight(fbo_t* fbo);
extern void fboUpdateViewport(fbo_t* fbo);
extern void fboBindTextures(fbo_t* fbo);

#endif /*EXH_SYSTEM_GRAPHICS_FBO_H_*/
