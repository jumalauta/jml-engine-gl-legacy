#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <assert.h>
#include <graphicsIncludes.h>

#include "image.h"
#include "system/debug/debug.h"
#include "system/player/player.h"
#include "system/io/io.h"
#include "system/datatypes/memory.h"
#include "system/thread/thread.h"
#include "system/graphics/video/video.h"
#include "system/ui/window/window.h"

#ifdef PNG

#ifdef LIBPNG
#include <libpng/png.h>

static void debugPrintPngInfo(const char *extra_info, const char *filename,
	png_uint_32 width, png_uint_32 height, int bit_depth,
	int color_type, int interlace_method, int compression_method, int filter_method)
{
	char interlace_method_string[32] = {'\0'};
	switch (interlace_method)
	{
		case PNG_INTERLACE_NONE:
			sprintf(interlace_method_string, "NONE");
			break;
		case PNG_INTERLACE_ADAM7:
			sprintf(interlace_method_string, "ADAM7");
			break;
		default:
			sprintf(interlace_method_string, "UNKNOWN(%d)", interlace_method);
			break;
	}

	char compression_method_string[32] = {'\0'};
	switch (compression_method)
	{
		case PNG_COMPRESSION_TYPE_BASE:
			sprintf(compression_method_string, "COMPRESSION");
			break;
		default:
			sprintf(compression_method_string, "UNKNOWN(%d)", compression_method);
			break;
	}

	char filter_method_string[32] = {'\0'};
	switch (filter_method)
	{
		case PNG_FILTER_TYPE_BASE:
			sprintf(filter_method_string, "FILTER");
			break;
		case PNG_INTRAPIXEL_DIFFERENCING:
			sprintf(filter_method_string, "INTRAPIXEL_DIFFERENCING");
			break;
		default:
			sprintf(filter_method_string, "UNKNOWN(%d)", filter_method);
			break;
	}

	char color_type_string[32] = {'\0'};
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			sprintf(color_type_string, "GRAY");
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			sprintf(color_type_string, "GRAY_ALPHA");
			break;
		case PNG_COLOR_TYPE_PALETTE:
			sprintf(color_type_string, "PALETTE");
			break;
		case PNG_COLOR_TYPE_RGB:
			sprintf(color_type_string, "RGB");
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			sprintf(color_type_string, "RGB_ALPHA");
			break;
		case PNG_COLOR_MASK_PALETTE:
			sprintf(color_type_string, "MASK_PALETTE");
			break;
		default:
			sprintf(color_type_string, "UNKNOWN(%d)", color_type);
			break;
	}

	debugPrintf("%s'%s': %dx%d %dbit %s %s %s %s", extra_info, filename, width, height, bit_depth,
		color_type_string, interlace_method_string, compression_method_string, filter_method_string);
}


#define CHANGE_LOG_SIZE 512

static void strcat_png_change(char *png_change_log, const char *str)
{
	if (png_change_log[0] != '\0')
	{
		strncat(png_change_log, ", ", CHANGE_LOG_SIZE-strlen(png_change_log)-1);
	}
	else
	{
		strncat(png_change_log, "PNG expanded: ", CHANGE_LOG_SIZE-strlen(png_change_log)-1);
	}
	
	strncat(png_change_log, str, CHANGE_LOG_SIZE-strlen(png_change_log)-1);
}

imageData_t* imageLoadPNG(const char* filename)
{
	char png_change_log[CHANGE_LOG_SIZE] = {'\0'};
	png_change_log[0] = '\0';

//#ifdef PNG_THREAD_UNSAFE_OK
	//debugWarningPrintf("libpng not thread safe!");
//#endif
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		debugErrorPrintf("Unable to read file '%s'!", filename);

		return NULL;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		debugErrorPrintf("Unable to create info for file '%s'!", filename);

		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);

		return NULL;
	}

	png_infop end_info_ptr = png_create_info_struct(png_ptr);
	if (!end_info_ptr)
	{
		debugErrorPrintf("Unable to create end-info for file '%s'!", filename);

		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

		return NULL;
	}
	
	FILE *f = fopen(filename, "rb");
	if (f==NULL)
	{
		return NULL;
	}

	png_init_io(png_ptr, f);
	png_read_info(png_ptr, info_ptr);

	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	int interlace_method;
	int compression_method;
	int filter_method;
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method);

	if (bit_depth == 16)
	{
#if PNG_LIBPNG_VER >= 10504
		png_set_scale_16(png_ptr);
#else
		png_set_strip_16(png_ptr);
#endif
		
		strcat_png_change(png_change_log, "16 bit depth to 8 bit");
	}

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
#if PNG_LIBPNG_VER >= 10209
		png_set_expand_gray_1_2_4_to_8(png_ptr);
#else
		png_set_gray_1_2_4_to_8(png_ptr);
#endif

		char gray[32];
		snprintf(gray, 32, "gray %d bit depth to 8 bit, ", bit_depth);
		strcat_png_change(png_change_log, gray);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
		
		strcat_png_change(png_change_log, "gray to RGB");
	}

	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
		
		strcat_png_change(png_change_log, "palette to RGB");
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		
		strcat_png_change(png_change_log, "tRNS to alpha");
	}
	else if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
	{
		//if alpha channel does not exist then add a filler
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		
		strcat_png_change(png_change_log, "filler to alpha");
	}

	/*
	//TODO: implement gamma correction
	double  gamma;
	if (png_get_gAMA(png_ptr, info_ptr, &gamma))
	{
		png_set_gamma(png_ptr, display_exponent, gamma);
		strcat_png_change(png_change_log, "gamma corrected");
	}*/

	if (strlen(png_change_log) > 0)
	{
		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method);

		strcat_png_change(png_change_log, "New details: ");
		debugPrintPngInfo(png_change_log, filename, width, height, bit_depth, color_type, interlace_method, compression_method, filter_method);
	}


	imageData_t* _imageData = (imageData_t*)malloc(sizeof(imageData_t));
	assert(_imageData);
	_imageData->filename = strdup(filename);
	assert(_imageData->filename);
	_imageData->name = strdup(filename);
	assert(_imageData->name);
	
	_imageData->w = width;
	_imageData->h = height;
	_imageData->channels = png_get_channels(png_ptr, info_ptr);
		
	_imageData->pixels = (void*)calloc((_imageData->h * _imageData->w + _imageData->w), sizeof(unsigned int));
	assert(_imageData->pixels);

	if (_imageData->channels == 4)
	{
		threadGlobalMutexLock();

		unsigned int i,j,k;
		png_bytep rowPtr[_imageData->h];
		for (i = 0; i < _imageData->h; i++)
		{
			rowPtr[i] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
		}
		png_read_image(png_ptr, rowPtr);
		threadGlobalMutexUnlock();
 

		for(i=0;i<_imageData->h;i++)
		{
			for(j=0,k=0;j<_imageData->w;j++,k+=_imageData->channels)
			{
#ifdef LITTLE_ENDIAN
				((unsigned int*)_imageData->pixels)[(_imageData->h-(i+1))*_imageData->w+j] =
					rowPtr[i][k] +
					rowPtr[i][k+1] * 0x00000100 +
					rowPtr[i][k+2] * 0x00010000 +
					rowPtr[i][k+3] * 0x01000000;
#else
				((unsigned int*)_imageData->pixels)[(_imageData->h-(i+1))*_imageData->w+j] =
					rowPtr[i][k+3] +
					rowPtr[i][k+2] * 0x00000100 +
					rowPtr[i][k+1] * 0x00010000 +
					rowPtr[i][k]   * 0x01000000;
#endif
			}
			
			png_free(png_ptr, rowPtr[i]);
		}
	}
	else
	{
		debugErrorPrintf("Image '%s' channel '%d' amount is not supported! Please use RGBA.",filename,_imageData->channels);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);

	fclose(f);

	return _imageData;
}

int imageWritePNG(imageData_t *imageData)
{
	assert(imageData);
	assert(imageData->filename);
	assert(imageData->pixels);

	FILE *f = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	
	f = fopen(imageData->filename, "wb");
	if (f == NULL)
	{
		debugErrorPrintf("Unable to open file '%s' for writing!", imageData->filename);
		return 0;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		debugErrorPrintf("Unable to create write struct for file '%s'!", imageData->filename);
		if (f != NULL)
		{
			fclose(f);
		}

		return 0;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		debugErrorPrintf("Unable to create info for file '%s'!", imageData->filename);
		if (f != NULL)
		{
			fclose(f);
		}
		if (png_ptr != NULL)
		{
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		}
		return 0;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) 
	{
		debugErrorPrintf("Erroring in creating file '%s'!", imageData->filename);
		if (f != NULL)
		{
			fclose(f);
		}
		if (info_ptr != NULL)
		{
			png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		}
		if (png_ptr != NULL)
		{
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		}
		return 0;
	}

	const int bit_depth = 8;
	int color_type = 0;
	switch(imageData->channels)
	{
		case 3:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case 4:
			color_type = PNG_COLOR_TYPE_RGBA;
			break;
		default:
			debugErrorPrintf("Invalid channel amount %d! Expected 3 (RGB) or 4 (RGBA). filename:'%s'", imageData->channels, imageData->filename);
			return 0;
	}

	png_init_io(png_ptr, f);

	png_set_IHDR(png_ptr, info_ptr, imageData->w, imageData->h,
		bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if (imageData->name != NULL)
	{
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = imageData->name;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	png_bytep row = (png_bytep)malloc(imageData->channels * imageData->w * sizeof(png_byte));

	unsigned int x, y;
	for (y=0; y<imageData->h; y++)
	{
		for (x=0; x<imageData->w; x++)
		{
			unsigned int color = imageData->pixels[(imageData->h-y-1)*imageData->w + x];
			unsigned char r = (color>>0)&0xFF; 
			unsigned char g = (color>>8)&0xFF;
			unsigned char b = (color>>16)&0xFF;
			unsigned char a = 0xFF; //disable alpha - no transparency

			switch(imageData->channels)
			{
				case 3:
					row[x*imageData->channels+0] = r;
					row[x*imageData->channels+1] = g;
					row[x*imageData->channels+2] = b;
					break;
				case 4:
					row[x*imageData->channels+0] = r;
					row[x*imageData->channels+1] = g;
					row[x*imageData->channels+2] = b;
					row[x*imageData->channels+3] = a;
					break;
				default:
					assert(2==1 && "this should never end up in here!");
					break;
			}
		}

		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, NULL);

	if (f != NULL)
	{
		fclose(f);
	}
	if (info_ptr != NULL)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	}
	if (png_ptr != NULL)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	}
	if (row != NULL)
	{
		free(row);
	}

	return 1;
}

#else

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

imageData_t* imageLoadPNG(const char* filename)
{
	int f = open(filename, O_RDONLY);
	int count = lseek(f, 0, SEEK_END);
	close(f);
	
	FILE *fp = fopen(filename,"rb");
	if (fp == NULL) {
		return NULL;
	}
	
	char *content = (char *)malloc(sizeof(char) * (count + 1));
	int readCount = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
	assert(count == readCount);

	imageData_t* _imageData = (imageData_t*)malloc(sizeof(imageData_t));
	assert(_imageData);
	_imageData->filename = strdup(filename);
	assert(_imageData->filename);
	_imageData->name = strdup(filename);
	assert(_imageData->name);

    //flip image to meet OpenGL's expectations
    stbi_set_flip_vertically_on_load(1);

    int width = 0;
    int height = 0;
    int channels = 0;
    const int FORCE_RGBA = 4;
    _imageData->pixels = stbi_load_from_memory(content, count, &width, &height, &channels, FORCE_RGBA);
	assert(_imageData->pixels);
	
	_imageData->w = width;
	_imageData->h = height;
	_imageData->channels = channels;

	free(content);

	return _imageData;
}

int imageWritePNG(imageData_t *imageData)
{
	assert(imageData);
	assert(imageData->filename);
	assert(imageData->pixels);

	return 1;
}

#endif

int imageTakeScreenshot(const char *filename)
{
	if (!endsWithIgnoreCase(filename, ".png"))
	{
		debugErrorPrintf("Screenshot failed! Only PNG screenshots are supported. filename:'%s'", filename);
		return 0;
	}

	imageData_t *imageData = (imageData_t*)malloc(sizeof(imageData_t)); 
	assert(imageData);

	imageData->filename = strdup(filename);
	assert(imageData->filename);
	imageData->name = strdup(filename);
	assert(imageData->name);

	imageData->w = getWindowWidth();
	imageData->h = getWindowHeight();

	imageData->channels = 4;
	imageData->pixels = (void*)calloc((imageData->h * imageData->w + imageData->w), sizeof(unsigned int));
	assert(imageData->pixels);

	glReadPixels(0, 0, imageData->w, imageData->h, GL_RGBA, GL_UNSIGNED_BYTE, imageData->pixels);

	int ok = imageWritePNG(imageData);

	freeImageData(imageData);

	return ok;
}

void freeImageData(imageData_t *img)
{
	free(img->name);
	free(img->filename);
	if (img->pixels) {
		free(img->pixels);
		img->pixels = NULL;
	}
	free(img);
}

static texture_t* imageProcessImageData(const char *filename)
{
	const char *file = getFilePath(filename);
	texture_t *tex = NULL;
	
	if (endsWithIgnoreCase(filename, ".png"))
	{
		imageData_t *img = imageLoadPNG(file);
		if (img != NULL)
		{
			tex = imageCreateTextureByImageData(img);
			debugPrintf("Loaded image '%s' (%p, %dx%d)", file, tex, tex->w, tex->h);

			freeImageData(img);
		}
		else
		{
			debugWarningPrintf("Couldn't load image '%s'!", file);
		}
	}
#ifdef SUPPORT_VIDEO
	else if (endsWithIgnoreCase(filename, ".ogv") || endsWithIgnoreCase(filename, ".ogg"))
	{
		video_t *video = videoLoad(filename);
		if (video != NULL && video->frameTexture != NULL)
		{
			debugPrintf("Video loaded! '%s'", filename);
			return video->frameTexture;
		}
		else
		{
			debugWarningPrintf("Couldn't load video '%s'!", file);
		}
	}
#endif
	else
	{
		debugErrorPrintf("Unrecognized image format! file:'%s'", filename);
	}

	return tex;
}

static void imageLoadImageThread(void* data)
{
	assert(data);

	imageLoadImage((const char*)data);
	notifyResourceLoaded();
}

void imageLoadImageAsync(const char *filename)
{
	if (threadIsEnabled())
	{
		threadAsyncCall(imageLoadImageThread, (void*)getFilePath(filename));
	}
}

//abstraction in case if some other format besides PNG is used
texture_t* imageLoadImage(const char* filename)
{
	const char *file = filename;
	texture_t *tex = getTextureFromMemory(file);
	if (tex == NULL)
	{
		file = getFilePath(filename);
		tex = getTextureFromMemory(file);
	}
	
	if (tex == NULL)
	{
		tex = imageProcessImageData(file);
	}

	return tex;
}

#endif

texture_t* imageCreateTextureByImageData(imageData_t* _imageData)
{

	texture_t *_texture = textureInit(NULL);
	
	assert(_imageData->filename && strlen(_imageData->filename) > 0);

	_texture->name = strdup(_imageData->filename);

	threadGlobalMutexLock();

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 

	switch(_imageData->channels)
	{
		case 1:
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, _imageData->w, _imageData->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, (const void *)_imageData->pixels);
			//break;
		case 3:
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _imageData->w, _imageData->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *)_imageData->pixels);
			_texture->hasAlpha = 0;
			break;

		case 2:
		case 4:
			gluBuild2DMipmaps(
				GL_TEXTURE_2D,
				4,
				_imageData->w,
				_imageData->h,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				(const void *)_imageData->pixels
			);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _imageData->w, _imageData->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *)_imageData->pixels);
			_texture->hasAlpha = 1;
			break;
	}

	_texture->customWidth  = _texture->w = _imageData->w;
	_texture->customHeight = _texture->h = _imageData->h;
	_texture->multiTextureId[0] = _texture->id = id;

	glBindTexture(GL_TEXTURE_2D, 0);

	threadGlobalMutexUnlock();

	return _texture;
}

texture_t* imageCreateTexture(const char *name, int filter, int format, int width, int height)
{
	texture_t *_texture = textureInit(NULL);

	_texture->name = strdup(name);

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
	if (filter == 0)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
	if (filter == 1)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}
	
#ifdef SUPPORT_GL_FBO
	if (format == GL_DEPTH_COMPONENT)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	}
#endif
	_texture->customWidth = _texture->w = width;
	_texture->customHeight = _texture->h = height;
	_texture->multiTextureId[0] = _texture->id = id;

	glBindTexture(GL_TEXTURE_2D, 0);

	debugPrintf("Created texture '%s' (%p, %dx%d)", name, _texture, width, height);

	return _texture;
}
