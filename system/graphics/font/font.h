#ifndef EXH_SYSTEM_GRAPHICS_FONT_FONT_H_
#define EXH_SYSTEM_GRAPHICS_FONT_FONT_H_

#include "graphicsIncludes.h"

typedef struct {
	char *name;
	texture_t *fontTexture;
	void *characterData;
} font_t;

extern void deinitFont(font_t *font);
extern void setTextPivot(double x, double y, double z);
extern void setTextRotation(double x, double y, double z);
extern void setTextSize(double w, double h);
extern void setTextPosition(double x, double y, double z);
extern void setTextCenterAlignment(int center);
extern void setTextFont(const char *font);
extern void fontInit(void);
extern void setTextDefaults(void);
extern void fontDeinit();
extern void setDrawTextString(const char *txt);
extern double getTextStringWidth();
extern double getTextStringHeight();
extern double getTextCharacterHeight();
extern double getTextCharacterWidth();
extern void setTextWrap(int wrap);
extern void drawText2d();
extern void drawText3d();


#endif /* EXH_SYSTEM_GRAPHICS_FONT_FONT_H_ */
