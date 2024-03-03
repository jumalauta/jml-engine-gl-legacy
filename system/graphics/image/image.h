#ifndef SYSTEM_GRAPHICS_IMAGE_H_
#define SYSTEM_GRAPHICS_IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system/graphics/graphics.h"

typedef struct {
	char *name, *filename;
	unsigned int w, h, channels, *pixels;
} imageData_t;

extern void freeImageData(imageData_t *img);

#ifdef PNG
extern imageData_t* imageLoadPNG(const char* filename);
extern int imageWritePNG(imageData_t *imageData);
extern int imageTakeScreenshot(const char *filename);
extern void imageLoadImageAsync(const char *filename);
extern texture_t* imageLoadImage(const char* filename);
#endif

extern texture_t* imageCreateTextureByImageData(imageData_t* _imageData);
extern texture_t* imageCreateTexture(const char *name, int filter, int channels, int width, int height);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif
