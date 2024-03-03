#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system/ui/window/window.h"
#include "system/timer/timer.h"
#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/graphics/image/image.h"
#include "system/debug/debug.h"
#include "system/datatypes/memory.h"
#include "system/datatypes/string.h"
#include "system/io/io.h"
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

#include "font.h"

#define ASCII_CONTROL_CHARACTERS 32
#define TOTAL_NORMAL_CHARACTERS 127
#define EXTRA_CHARACTERS 0
#define TOTAL_CHARACTERS (TOTAL_NORMAL_CHARACTERS + EXTRA_CHARACTERS - ASCII_CONTROL_CHARACTERS)
#define CHARACTER_ROWS 10
#define CHARACTER_COLS 10

//used for the menu and in demo rendering in case data/font.png does not exist
unsigned char font_rasters[TOTAL_CHARACTERS][13] = {
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36},
{0x00, 0x00, 0x00, 0x66, 0x66, 0xff, 0x66, 0x66, 0xff, 0x66, 0x66, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x7e, 0xff, 0x1b, 0x1f, 0x7e, 0xf8, 0xd8, 0xff, 0x7e, 0x18},
{0x00, 0x00, 0x0e, 0x1b, 0xdb, 0x6e, 0x30, 0x18, 0x0c, 0x76, 0xdb, 0xd8, 0x70},
{0x00, 0x00, 0x7f, 0xc6, 0xcf, 0xd8, 0x70, 0x70, 0xd8, 0xcc, 0xcc, 0x6c, 0x38},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x0c, 0x0e},
{0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c},
{0x00, 0x00, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30},
{0x00, 0x00, 0x00, 0x00, 0x99, 0x5a, 0x3c, 0xff, 0x3c, 0x5a, 0x99, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03},
//0
//0 with slash {0x00, 0x00, 0x3c, 0x66, 0xc3, 0xe3, 0xf3, 0xdb, 0xcf, 0xc7, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x38, 0x18},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0x07, 0x03, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xff, 0xcc, 0x6c, 0x3c, 0x1c, 0x0c},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x03, 0x7f, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06},
{0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60},
{0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x0c, 0x06, 0x03, 0xc3, 0xc3, 0x7e},
{0x00, 0x00, 0x3f, 0x60, 0xcf, 0xdb, 0xd3, 0xdd, 0xc3, 0x7e, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e},
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3},
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c},
{0x00, 0x03, 0x03, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60},
{0x00, 0x00, 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18},
{0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38, 0x30, 0x70},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0x7f, 0x03, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0xc3, 0xc0, 0xc0, 0xc0, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x03, 0x03, 0x03, 0x03, 0x03},
{0x00, 0x00, 0x7f, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x33, 0x1e},
{0x7e, 0xc3, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
{0x38, 0x6c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x00},
{0x00, 0x00, 0xc6, 0xcc, 0xf8, 0xf0, 0xd8, 0xcc, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78},
{0x00, 0x00, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfc, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x03, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0x03, 0x03, 0x7e, 0xc0, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x1c, 0x36, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x00},
{0x00, 0x00, 0x7e, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xdb, 0xc3, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0x60, 0x60, 0x30, 0x18, 0x3c, 0x66, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xff, 0x60, 0x30, 0x18, 0x0c, 0x06, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x0f, 0x18, 0x18, 0x18, 0x38, 0xf0, 0x38, 0x18, 0x18, 0x18, 0x0f},
{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0xf0, 0x18, 0x18, 0x18, 0x1c, 0x0f, 0x1c, 0x18, 0x18, 0x18, 0xf0},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x8f, 0xf1, 0x60, 0x00, 0x00, 0x00}
};


#ifdef SDL_TTF

#include <SDL/SDL_ttf.h>
static TTF_Font* font = NULL;
//static char fontpath[] = "data/gfx/SpecialElite.ttf"; //MetalShow.ttf";
static char fontpath[] = "data/Arial.ttf";

/*static int round(double x)
{
	return (int)(x + 0.5);
}*/

static int nextpoweroftwo(int x)
{
	double logbase2 = log(x) / log(2);
	return round(pow(2,ceil(logbase2)));
}

static SDL_Color color;
static SDL_Rect position;

void setTextDefaults()
{
	color.r = 255;
	color.g = 255;
	color.b = 255;
	
	setTextWrap(0);
}

void fontInit(void)
{
	debugPrintf("Initializing fonts");

	setTextDefaults();

	if (TTF_Init())
	{
		debugErrorPrintf("Could not initialize SDL TTF: '%s'", TTF_GetError());
		return;
	}
	
	font = TTF_OpenFont(fontpath, 30);
	if(font == NULL)
	{
		debugWarningPrintf("Could not load font: %s", TTF_GetError());
		return;
	}
	
	debugPrintf("Loaded font: '%p' '%s'", font, fontpath);
}

void fontDeinit(void)
{
	debugPrintf("Deinitializing fonts");
	
	if (font != NULL)
	{
		TTF_CloseFont(font);
	}
	
	TTF_Quit();
}

static double textWidth  = 1.0;
static double textHeight = 1.0;

void setTextSize(double w, double h)
{
	textWidth  = w;
	textHeight = h;
}

void drawText3d(const char *txt)
{
	glPushMatrix();
	
	SDL_Rect location;
	location.x = 0;
	location.y = 0;
	
	SDL_Color color;
	color.r = 255;
	color.g = 255;
	color.b = 255;

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	//SDL_Rect rect;
	int w,h;
	unsigned int texture;

	initial = TTF_RenderText_Blended(font, txt, color);

	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);

	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); 
			glVertex2f(location.x    , location.y);
		glTexCoord2f(1.0f, 1.0f); 
			glVertex2f(location.x + w, location.y);
		glTexCoord2f(1.0f, 0.0f); 
			glVertex2f(location.x + w, location.y + h);
		glTexCoord2f(0.0f, 0.0f); 
			glVertex2f(location.x    , location.y + h);
	glEnd();

	glFinish();

	location.w = initial->w;
	location.h = initial->h;

	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
	glDeleteTextures(1, &texture);
	
	glPopMatrix();
}

static void __drawText3d(int x, int y, const char *txt)
{
	glPushMatrix();
	
	SDL_Rect location;
	location.x = x;
	location.y = y;
	
	SDL_Color color;
	color.r = 255;
	color.g = 255;
	color.b = 255;

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	//SDL_Rect rect;
	int w,h;
	unsigned int texture;

	initial = TTF_RenderText_Blended(font, txt, color);

	w = nextpoweroftwo(initial->w);
	h = nextpoweroftwo(initial->h);

	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); 
			glVertex2f(location.x    , location.y);
		glTexCoord2f(1.0f, 1.0f); 
			glVertex2f(location.x + w, location.y);
		glTexCoord2f(1.0f, 0.0f); 
			glVertex2f(location.x + w, location.y + h);
		glTexCoord2f(0.0f, 0.0f); 
			glVertex2f(location.x    , location.y + h);
	glEnd();

	glFinish();

	location.w = initial->w;
	location.h = initial->h;

	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
	glDeleteTextures(1, &texture);
	
	glPopMatrix();
}

static void SDL_GL_RenderText(const char *text, 
                      TTF_Font *fontti,
                      SDL_Color color,
                      SDL_Rect *location)
{
color.r = 255;
color.g = 255;
color.b = 255;
	SDL_Surface *initial;

	initial = TTF_RenderText_Blended(fontti, text, color);
	if(initial == NULL)
	{
		debugPrintf("Rendertext error: %p '%s'", initial, TTF_GetError());
		return;
	}

	int i=0;
	glBegin(GL_POINTS);
			for(i=0;i<initial->h;i++)
			{
				int j;
				for(j=0;j<initial->w;j++)
				{
					unsigned int color_data = (((unsigned int*)initial->pixels)[(initial->h-(i+1))*initial->w+j]>>24&0xFF)>0?255:0;
					if (color_data>0)
					{
						//glColor4ub(0xFF, 0xFF, 0x, 0xFF);
						glVertex2f(location->x+j,location->y+i);		
					}
				}
			}
glEnd();
	location->w = initial->w;
	location->h = initial->h;

	SDL_FreeSurface(initial);
}

void drawText2d(double ox,double oy,const char *txt)
{
	position.x = ox;
	position.y = oy;
	SDL_GL_RenderText(txt, font, color, &position);
}

#else // BUILTIN font

static texture_t font[TOTAL_CHARACTERS];
static imageData_t *fontData=NULL, characterData;

static double textWidth  = 1.0;
static double textHeight = 1.0;

static double textPivotX = 0.0;
static double textPivotY = 0.0;
static double textPivotZ = 0.0;

static double textAngleX = 0.0;
static double textAngleY = 0.0;
static double textAngleZ = 0.0;

static int textCenter = 0;
static double textX = 0.0;
static double textY = 0.0;
static double textZ = 0.0;

static const char *textString = NULL;
static int textStringWidth = 0;
static int textStringHeight = 0;

//unsigned char ttf_buffer[1<<20];
#define FONT_BITMAP_SIZE 1024
//unsigned char temp_bitmap[FONT_BITMAP_SIZE*FONT_BITMAP_SIZE];
static font_t *currentFont = NULL;
static font_t *defaultFont = NULL;

void deinitFont(font_t *font)
{
	assert(font);

	if (font == currentFont)
	{
		currentFont = NULL;
	}
	if (font == defaultFont)
	{
		defaultFont = NULL;
	}

	if (font->name)
	{
		free(font->name);
		font->name = NULL;
	}

	if (font->characterData)
	{
		free(font->characterData);
		font->characterData = NULL;
	}
}

//static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
//static GLuint ftex;
static int ascent, descent, lineGap, baseline;

static font_t* my_stbtt_initfont(const char *ttfFilePath)
{
	font_t* font = memoryAllocateFont(NULL);
	font->name = strdup(ttfFilePath);

   unsigned int count = 0;
   unsigned char *ttf_buffer = (unsigned char *)ioReadFileToBuffer(ttfFilePath, &count);

   imageData_t fontCharacterData;
	fontCharacterData.w = FONT_BITMAP_SIZE;
	fontCharacterData.h = FONT_BITMAP_SIZE;
	fontCharacterData.channels = 1;
	fontCharacterData.pixels = NULL;
	unsigned char *ucPixels = (unsigned char*)calloc((fontCharacterData.w*fontCharacterData.h), sizeof(unsigned char));
	assert(ucPixels);
	fontCharacterData.filename = strdup(ttfFilePath);
	assert(fontCharacterData.filename);
	fontCharacterData.name = strdup(ttfFilePath);
	assert(fontCharacterData.name);

//fseek (pFile, 0, SEEK_END);   // non-portable
//size=ftell (pFile);
	font->fontTexture = imageCreateTexture(fontCharacterData.name, 0, GL_ALPHA, fontCharacterData.w,fontCharacterData.h);
	font->characterData = (void*)malloc(sizeof(stbtt_bakedchar)*96);

   //fread(ttf_buffer, 1, 1<<20, fopen(ttfFilePath, "rb"));
   float pixelSize = 90.0; //seems that some fonts are not visible with too large pixel sizes (i.e. 100px) - maybe dynamic setup needed?
   //unsigned char *temp_bitmap = (unsigned char *)malloc(FONT_BITMAP_SIZE*FONT_BITMAP_SIZE*sizeof(unsigned char));
   //stbtt_BakeFontBitmap(ttf_buffer,0, pixelSize, ucPixels,fontCharacterData.w,fontCharacterData.h, 32,96, cdata); // no guarantee this fits!
   stbtt_BakeFontBitmap(ttf_buffer,0, pixelSize, ucPixels,fontCharacterData.w,fontCharacterData.h, 32,96, (stbtt_bakedchar*)font->characterData); // no guarantee this fits!
   
   stbtt_fontinfo fontinfo;
   stbtt_InitFont(&fontinfo, ttf_buffer, 0);
   float scale = stbtt_ScaleForPixelHeight(&fontinfo, pixelSize);
   stbtt_GetFontVMetrics(&fontinfo, &ascent,&descent,&lineGap);
   baseline = (int) (ascent*scale);
   lineGap = (int) (lineGap*scale+0.5);
   //debugPrintf("PASKA: baseline:%d,scale:%.2f,ascent:%.2f,descent:%.2f,lineGap:%d",baseline,scale,ascent*scale,descent*scale,lineGap);

   // can free ttf_buffer at this point
   //glGenTextures(1, &ftex);
   glBindTexture(GL_TEXTURE_2D, font->fontTexture->id);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, ucPixels);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glBindTexture(GL_TEXTURE_2D, 0);

	free(fontCharacterData.filename);
	free(fontCharacterData.name);
	free(ucPixels);
   free(ttf_buffer);

   debugPrintf("Loaded font '%s'", font->name);

   //free(temp_bitmap);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   return font;
}

static void my_stbtt_print(float x, float y, const char *txt)
{
   assert(currentFont);

   signed char *text = (signed char*)txt;
   double lineCharacterHeight = 0.0;
   double height = 0.0;
   double width = 0.0;
   double lineWidth = 0.0;
   // assume orthographic projection with units = screen pixels, origin at top left
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, currentFont->fontTexture->id);
   glBegin(GL_QUADS);
   stbtt_bakedchar *cdata = (stbtt_bakedchar*)currentFont->characterData;
   assert(cdata);
   while (*text) {
      stbtt_aligned_quad q;
   	  if (*text == '\n') {
         stbtt_GetBakedQuad(cdata, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, 0, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         double characterHeight = q.y1 - q.y0 + lineGap;
         lineCharacterHeight = (lineCharacterHeight<characterHeight)?characterHeight:lineCharacterHeight;
         height += lineCharacterHeight;
         width = lineWidth;
         //lineWidth = 0;
   	  }
      else if (*text >= ASCII_CONTROL_CHARACTERS) {
         stbtt_GetBakedQuad(cdata, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, *text-ASCII_CONTROL_CHARACTERS, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         //double characterWidth = q.x1 - q.x0;
         double characterHeight = q.y1 - q.y0 + lineGap;
         //debugPrintf("Printing '%c': characterHeight:%.2f,y0:%.2f,y1:%.2f",*text,characterHeight,q.y0,q.y1);
         lineWidth = x;
         lineCharacterHeight = (lineCharacterHeight<characterHeight)?characterHeight:lineCharacterHeight;
         glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0-width,q.y0-height-q.y1-q.y0);
         glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1-width,q.y0-height-q.y1-q.y0);
         glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1-width,q.y1-height-q.y1-q.y0);
         glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0-width,q.y1-height-q.y1-q.y0);
      }
      ++text;
   }
   glEnd();
}

static double textCharacterAverageWidth = 0.0;
static double textCharacterAverageHeight = 0.0;

static void my_stbtt_text_length(const char *txt)
{
   signed char *text = (signed char*)txt;
   float x = 0.0f;
   float y = 0.0f;
   //double height = 0.0;
   double lineWidth = 0.0;
   double maxLineWidth = 0.0;
   double lineCharacterHeight = 0.0;
   int includedCharacters = 0;
   textCharacterAverageWidth = 0.0;
   textCharacterAverageHeight = 0.0;
   stbtt_bakedchar *cdata = (stbtt_bakedchar*)currentFont->characterData;
   int lines = -1;
   while (*text) {
      stbtt_aligned_quad q;
   	  if (*text == '\n') {
         stbtt_GetBakedQuad(cdata, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, 0, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         double characterHeight = q.y1 - q.y0 + lineGap;
         lineCharacterHeight = (lineCharacterHeight<characterHeight)?characterHeight:lineCharacterHeight;
         lines++;
         lineWidth = 0.0;
   	  }
      else if (*text >= ASCII_CONTROL_CHARACTERS) {
         stbtt_GetBakedQuad(cdata, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, *text-ASCII_CONTROL_CHARACTERS, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         double characterWidth = q.x1 - q.x0;
         double characterHeight = q.y1 - q.y0 + lineGap;
         lineWidth += characterWidth;
         maxLineWidth = (maxLineWidth<lineWidth)?lineWidth:maxLineWidth;
         lineCharacterHeight = (lineCharacterHeight<characterHeight)?characterHeight:lineCharacterHeight;
         if (*text > ASCII_CONTROL_CHARACTERS) {
         	includedCharacters++;
            textCharacterAverageWidth += characterWidth;
            textCharacterAverageHeight += characterHeight;
         }
      }
      ++text;
   }

   textCharacterAverageWidth /= (double)includedCharacters;
   textCharacterAverageHeight /= (double)includedCharacters;
   textStringWidth = (int)(maxLineWidth+0.5);
   textStringHeight = (int)((textCharacterAverageHeight*lines)+0.5);
}

void setTextDefaults()
{
	currentFont = defaultFont;
	//debugPrintf("Setting font '%s' %p vs. %p",currentFont->name,currentFont,defaultFont);

	fontData   = NULL;
	textString = NULL;

	textStringWidth  = 0;
	textStringHeight = 0;

	textWidth  = 1.0;
	textHeight = 1.0;

	textPivotX = 0.0;
	textPivotY = 0.0;
	textPivotZ = 0.0;

	textAngleX = 0.0;
	textAngleY = 0.0;
	textAngleZ = 0.0;

	textCenter = 0;
	textX = 0.0;
	textY = 0.0;
	textZ = 0.0;
}

static font_t* loadTextFont(const char* filename)
{
	const char *file = filename;
	font_t *font = getFontFromMemory(file);
	if (font == NULL)
	{
		file = getFilePath(filename);
		font = getFontFromMemory(file);
	}
	
	if (font == NULL)
	{
		font = my_stbtt_initfont(file);
	}

	if (font != NULL && defaultFont == NULL)
	{
		defaultFont = font;
		currentFont = defaultFont;
		debugPrintf("Default font '%s'", defaultFont->name);
	}

	return font;
}

void setTextFont(const char *font)
{
	//debugPrintf("Setting font '%s'",font);
	currentFont = loadTextFont(font);
	if (currentFont == NULL)
	{
		currentFont = defaultFont;
	}
}

void fontInit(void)
{
	setTextDefaults();

#ifdef PNG
	const char *customFont = getFilePath("data/font.png");
	if (fileExists(customFont))
	{
		fontData = imageLoadPNG(customFont);
		debugPrintf("Initializing custom font '%s'", customFont);
		characterData.w = fontData->w/CHARACTER_COLS;
		characterData.h = fontData->h/CHARACTER_ROWS;
		characterData.channels = fontData->channels;
	}
	else
#endif
	{
		customFont = getFilePath("data/font.ttf");
		if (fileExists(customFont))
		{
			currentFont = loadTextFont(customFont);
			return;
		}
		else
		{
			debugPrintf("Initializing default 9x13 font");
			characterData.w = 8+1;
			characterData.h = 13;
			characterData.channels = 4;
		}
	}

	//127-32 = 95
	unsigned int i,j,character;
	
	characterData.pixels = (void*)calloc((characterData.h * characterData.w + characterData.w), sizeof(unsigned int));
	characterData.filename = (char*)malloc(32*sizeof(char));
	characterData.name = (char*)malloc(32*sizeof(char));

	int row = -1;
	for(character = 0; character < TOTAL_CHARACTERS; character++)
	{
		sprintf(characterData.filename, "Font character '%c'", (char)(character+32));
		sprintf(characterData.name, "Font character '%c'", (char)(character+32));

		//printf("\n// %c\n",(char)(character+ASCII_CONTROL_CHARACTERS));
		if (fontData)
		{
			if (character%CHARACTER_ROWS == 0)
			{
				row++;
			}

			int offsetX = (character%CHARACTER_COLS)*characterData.w;
			int offsetY = row*characterData.h;
			
			//printf("%02d: '%c' (x:%d y:%d, w:%d, h:%d)\n",character,(char)(character+ASCII_CONTROL_CHARACTERS), offsetX, offsetY, characterData.w, characterData.h);

			for(i=0;i<characterData.h;i++)
			{
				for(j=0;j<characterData.w;j++)
				{
					((unsigned int*)characterData.pixels)[((characterData.h)-(i+1))*characterData.w+j]
						= ((unsigned int*)fontData->pixels)[((fontData->h)-(i+1+offsetY))*fontData->w+j+offsetX];
				}
			}
			font[character] = *imageCreateTextureByImageData(&characterData);
		}
		else
		{
#ifndef NO_FONT
			for(i=0;i<characterData.h;i++)
			{
				for(j = 0; j < 8; j++)
				{
					unsigned int value = 0x00;//(j&1==1)?0xFFFFFFFF:0x0000000;
					if (((font_rasters[character][characterData.h-i-1] >> (7-j)) & 1) == 1)
					{
						value = 0xFFFFFFFF;
						//printf("1,");
					}
					else
					{
						//printf("0,");
					}
					((unsigned int*)characterData.pixels)[((characterData.h)-(i+1))*characterData.w+j] = value;
				}
				//printf("\n");
				//populateRasterBit(font_rasters[character][characterData.h-i-1], ((unsigned int*)characterData.pixels)[((characterData.h)-(i+1))*characterData.w]);
			}
			font[character] = *imageCreateTextureByImageData(&characterData);
#endif
		}
	}

	if (fontData)
	{
		freeImageData(fontData);
	}

	free(characterData.filename);
	free(characterData.name);
	free(characterData.pixels);
}

void fontDeinit(void)
{
}

void setTextSize(double w, double h)
{
	textWidth  = w;
	textHeight = h;
}

void setTextPivot(double x, double y, double z)
{
	textPivotX = x;
	textPivotY = y;
	textPivotZ = z;
}

void setTextRotation(double x, double y, double z)
{
	textAngleX = x;
	textAngleY = y;
	textAngleZ = z;
}

void setTextCenterAlignment(int center)
{
	textCenter = center;
}

void setTextPosition(double x, double y, double z)
{
	textX = x;
	textY = y;
	textZ = z;
}


double getTextStringWidth()
{
	return textStringWidth;
}

double getTextStringHeight()
{
	return textStringHeight;
}


double getTextCharacterHeight()
{
	if (textCharacterAverageHeight > 0)
	{
		return textCharacterAverageHeight;
	}

	return font[0].h*textHeight*25;
}

double getTextCharacterWidth()
{
	if (textCharacterAverageWidth > 0)
	{
		return textCharacterAverageWidth;
	}

	return font[0].w*textWidth*25;
}

void setDrawTextString(const char *txt)
{
	textString = txt;
	if (defaultFont != NULL)
	{
		my_stbtt_text_length(textString);
		return;
	}

	//calculate width and height dimensions of the text string
	int length = strlen(textString);
	int i = 0;
	int offsetY = 0;
	int offsetX = 0;
	textStringWidth = 0;
	textStringHeight = 0;
	for(i = 0; i < length; i++)
	{
		if (textString[i] == '\n')
		{
			offsetY += getTextCharacterHeight();
			offsetX = (i+1)*getTextCharacterWidth();
			continue;
		}

		texture_t *texture = &font[(textString[i]-ASCII_CONTROL_CHARACTERS)];
		
		double w = getTextCharacterWidth();
		double h = getTextCharacterHeight();
		int x = (i*texture->w)-offsetX*25;
		int y = offsetY;
		
		if (textStringWidth < x+w)
		{
			textStringWidth = x+w;
		}
		if (textStringHeight < y+h)
		{
			textStringHeight = y+h;
		}
	}
}

static int textWrap = 0;
void setTextWrap(int wrap)
{
	textWrap = wrap;
}

static void drawText(int is2d)
{
	float canvasWidth = getScreenWidth();
	float canvasHeight = getScreenHeight();
	if (is2d)
	{
		perspective2dBegin((int)canvasWidth,(int)canvasHeight);
	}

	glPushMatrix();

	double scaleW = textWidth;
	double scaleH = textHeight;
	if (!is2d)
	{
		scaleW *= 0.01;
		scaleH *= 0.01;
	}
	//debugPrintf("x:%.0f, y:%.0f, z:%.0f, w:%d, h:%d text:'%s'", textX, textY, textZ, textStringWidth, textStringHeight, textString);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	
	double xFixed = -getTextStringWidth()/2.0;
	double yFixed = getTextStringHeight()/2.0;

	double x = textX;
	double y = textY;
	if (is2d)
	{
		switch (textCenter)
		{
			case 1:
				x += (canvasWidth/2.0);
				y += (canvasHeight/2.0);
				break;
			case 2:
				x += (canvasWidth/2.0);
				break;
			case 3:
				y += (canvasHeight/2.0);
				break;
			case 4:
				xFixed = 0;
				break;
			case 5:
				x += -getTextStringWidth()/2.0+canvasWidth;
				break;
			default:
				break;
		}
	}

	double ox = xFixed + x/scaleW;
	double oy = yFixed + y/scaleH;
	double oz = textZ;
	


	//debugPrintf("x:%.0f, y:%.0f, w:%d, h:%d, text:'%s'", ox, oy, getTextStringWidth(), getTextStringHeight(), textString);
	double px = (getTextStringWidth()/2.0+textPivotX)*scaleW;
	double py = (-getTextStringHeight()/2.0+textPivotY)*scaleH;
	double pz = textPivotZ;
	//if (is2d)
	{
		//px += (getTextStringWidth()/2.0);
		//py += (getTextStringHeight()/2.0);
	}
	//else
	{
		//px += -(getTextStringWidth()/(double)getTextCharacterWidth()/2.0);
		//py += -(getTextStringHeight()/(double)getTextCharacterHeight()/2.0);
	}


	glTranslated(ox*scaleW,oy*scaleH,oz);

	glTranslated(px,py,pz);
	glRotated(textAngleX,-1,0,0);
	glRotated(textAngleY,0,-1,0);
	glRotated(textAngleZ,0,0,-1);
	glTranslated(-px,-py,-pz);

	glScaled(scaleW,scaleH,1);

	if (defaultFont != NULL)
	{
		my_stbtt_print(0, 0, textString);
	}
	else
	{
		int length = strlen(textString);
		double width3d = 1.0*scaleW;
		double height3d = 1.0*scaleH;

		int offsetY = 0;
		int offsetX = 0;
		int i = 0;
		for(i = 0; i < length; i++)
		{
			if (textString[i] == '\n')
			{
				if (is2d)
				{
					offsetX = (i+1)*getTextCharacterWidth();
					offsetY -= getTextCharacterHeight();
				}
				else
				{
					offsetX = (i+1)*width3d;
					offsetY -= height3d;
				}
				continue;
			}
			

			texture_t *texture = &font[(textString[i]-ASCII_CONTROL_CHARACTERS)];
			
			glBindTexture(GL_TEXTURE_2D, texture->id);
			
			double w = getTextCharacterWidth();
			double h = getTextCharacterHeight();
			double x = ((i*getTextCharacterWidth())-offsetX);
			/*if (is2d && textWrap)
			{
				if (x+getTextCharacterWidth() >= canvasWidth)
				{
					offsetX = (i)*getTextCharacterWidth();
					x -= offsetX;
					offsetY -= getTextCharacterHeight();
				}
			}*/
			double y = offsetY;
			double z = 0;
			if (!is2d)
			{
				w = width3d*50;
				h = height3d*50;
				x = ((i*width3d)-offsetX)*50;
				y = offsetY;
				z = 0;
			}

			//debugPrintf("x:%.0f, y:%.0f, w:%.0f, h:%.0f, '%c'",x,y,w,h,textString[i]);

			glBegin(GL_QUADS);
			glTexCoord2f(1.0f,1.0f);
			glVertex3d(x+w,y+h,z);
			glTexCoord2f(0.0f,1.0f);
			glVertex3d(x,y+h,z);
			glTexCoord2f(0.0f,0.0f);
			glVertex3d(x, y,z);
			glTexCoord2f(1.0f,0.0f);
			glVertex3d(x+w,y,z);
			glEnd();
		}
	}

	glBindTexture(GL_TEXTURE_2D,0);
	
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	
	glPopMatrix();

	if (is2d)
	{
		perspective2dEnd();
	}
}

void drawText2d()
{
	drawText(1);
}

void drawText3d()
{
	drawText(0);
}

#endif
