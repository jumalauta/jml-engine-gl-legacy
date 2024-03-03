#include "splineEditor.h"
#include "graphicsIncludes.h"
#include "system/ui/window/window.h"
#include "system/graphics/graphics.h"
#include "system/timer/timer.h"
#include "system/debug/debug.h"
#include "system/io/io.h"
#include "system/math/splines/spline.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern char buttonPressed, buttonReleased, ctrlPressed, shiftPressed, enterPressed, f1Pressed, f2Pressed, deletePressed, zPressed;
extern int mouseX, mouseY;
extern int lineRedraw;
extern unsigned int lineWidth;

static int saveCount = 1;

typedef struct {
	float x, y;
	float bX, bY;
} point;

typedef struct {
	float x, y;
	float bX, bY;
	char isCurve;
	struct cubicSpline *next;
} cubicSpline;

#define SPLINES_MAX 1024
static cubicSpline *bSTail[SPLINES_MAX];
static cubicSpline *bSHead[SPLINES_MAX];
static cubicSpline *bSCurrent[SPLINES_MAX];
static int pointNum[SPLINES_MAX];
static int splineWidth[SPLINES_MAX];
static int splinesNum = 0, splinesMax = 1;

void splineEditorInit(void)
{
	pointNum[0] = 0;
	splineWidth[0] = 3;
}

static cubicSpline *dragSpline=NULL;

typedef struct cubic
{
	float a, b, c, d;
} cubic;

typedef struct cubic3d
{
	cubic x, y, z;
} cubic3d;

extern void createCubicSpline(point3d *dest, int n, int detail, point3d *spline);

void splineEditorRun(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	resetViewport();
	viewReset();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glLineWidth(10.0f);

	glColor3f(0,0,0);

	glPointSize(lineWidth);
	perspective2dBegin(getWindowWidth(), getWindowHeight());


	if ((!ctrlPressed) && (dragSpline != NULL))
	{
		dragSpline = NULL;
		}

	if (ctrlPressed && (dragSpline))
	{
		dragSpline->x = (float)mouseX;
		dragSpline->y = (float)mouseY;
	}

	if ((buttonReleased == 1) && (bSHead[splinesNum] != NULL))
	{
		splinesNum++;
		splinesMax++;
		bSHead[splinesNum] = NULL;
		pointNum[splinesNum]=0;
		splineWidth[splinesNum]=3;
	}

	if ((ctrlPressed) && (zPressed))
	{
		bSCurrent[splinesNum] = bSHead[splinesNum];
		while(bSCurrent[splinesNum])
		{
			cubicSpline *next = (cubicSpline*)bSCurrent[splinesNum]->next;
			free(bSCurrent[splinesNum]);
			bSCurrent[splinesNum] = next;
		}
		bSHead[splinesNum] = NULL;
		pointNum[splinesNum]=0;
		if (splinesNum > 0)
		{
			splinesNum--;
			splinesMax--;
		}
	}

	if (buttonPressed == 1)
	{
		if (ctrlPressed)
		{
			int i=0;
			for(i=0; (i < splinesMax && (dragSpline == NULL)); i++)
			{
				bSCurrent[i] = bSHead[i];
				while(bSCurrent[i])
				{
					if ((bSCurrent[i]->x-10 <= (float)mouseX) && (bSCurrent[i]->x+10 >= (float)mouseX)
						&& (bSCurrent[i]->y-10 <= (float)mouseY) && (bSCurrent[i]->y+10 >= (float)mouseY))
					{
						dragSpline = bSCurrent[i];
						break;
					}
					bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				}
			}
		}
		else
		{
			cubicSpline *bs = (cubicSpline*)malloc(sizeof(cubicSpline));

			bs->next = NULL;
	
			bs->x = bs->bX = (float)mouseX;
			bs->y = bs->bY = (float)mouseY;
	
	
			if (bSHead[splinesNum] == NULL)
			{
				bSHead[splinesNum] = bSTail[splinesNum] = bs;
			}
			else
			{
				if ((bs->x-lineRedraw <= bSTail[splinesNum]->x) && (bs->x+lineRedraw >= bSTail[splinesNum]->x)
					&& (bs->y-lineRedraw <= bSTail[splinesNum]->y) && (bs->y+lineRedraw >= bSTail[splinesNum]->y))

				{
					free(bs);
					pointNum[splinesNum]--;
				}
				else
				{
					bSTail[splinesNum]->next = (struct cubicSpline*)bs;
					bSTail[splinesNum] = bs;
				}
			}
			pointNum[splinesNum]++;
		}
	}

	int i=0;
	for(i=0; i < splinesMax; i++)
	{
		#define SPLINE_DETAIL 10
		//point points[SPLINE_DETAIL];
		//point bezierCurve[4], bezierCurveTemp[4];

		if (pointNum[i] > 0)
		{
			cubic3d cubSpline[pointNum[i]];

			#define STEPS 12
			//glLineWidth(1.0f);
			//glColor3f(0,0,1);
			//glBegin(GL_LINE_STRIP);
			int j=0;
			bSCurrent[i] = bSHead[i];
			while(bSCurrent[i])
			{
			//	glVertex2f(bSCurrent[i]->x, bSCurrent[i]->y);
				cubSpline[j].x.a = bSCurrent[i]->x;
				cubSpline[j].y.a = bSCurrent[i]->y;
				bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				j++;

			}
			//glColor3f(1,1,1);
			//glEnd();

			if (pointNum[i]>0)
			{
				point3d dest[pointNum[i]*STEPS];
				point3d spline[pointNum[i]];
				for(j=0;j<pointNum[i];j++)
				{
					spline[j].x = cubSpline[j].x.a;
					spline[j].y = cubSpline[j].y.a;
					spline[j].z = 0;
				}
				createCubicSpline(dest, pointNum[i]-1, STEPS, spline);

				splineWidth[splinesNum]=lineWidth;
				glLineWidth(splineWidth[i]);
				glBegin(GL_LINE_STRIP);

				for(j = 0; j < (pointNum[i]-1)*STEPS; j++)
				{
					glVertex2f(
						dest[j].x,
						dest[j].y
					);
				}

				glEnd();
			}
		}

		if ((ctrlPressed) || (shiftPressed))
		{
			if (ctrlPressed)
			{
				glLineWidth(1.0f);
				glColor3f(0,0,1);
				glBegin(GL_LINE_STRIP);
				bSCurrent[i] = bSHead[i];
				while(bSCurrent[i])
				{
					glVertex2f(bSCurrent[i]->x, bSCurrent[i]->y);
					bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				}
				glEnd();
				glLineWidth(3.0f);
				glColor3f(ctrlPressed,0,0);
			}

			glPointSize(10.0f);
			glBegin(GL_POINTS);
			bSCurrent[i] = bSHead[i];
			while(bSCurrent[i])
			{
				glVertex2f(bSCurrent[i]->x, bSCurrent[i]->y);
				bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
			}
			glEnd();

			glColor3f(1,1,1);
		}
	}


	glBegin(GL_POINTS);
	glVertex2f((float)mouseX, (float)mouseY);
	glEnd();

	perspective2dEnd();

	glDisable(GL_BLEND);
glColor3f(1,1,1);
//	graphicsFlush();

	//save this
	if (f1Pressed)
	{
		char lamah[64];
		sprintf(lamah, "draw%d.dat", saveCount);
		FILE *f = fopen(lamah, "wb");

		if (f)
		{
			for(i=0; i < splinesMax; i++)
			{
				if (pointNum[i] == 0) { continue; }

				fwrite(&pointNum[i], 1, sizeof(int), f);
				fwrite(&splineWidth[i], 1, sizeof(int), f);
				bSCurrent[i] = bSHead[i];
				float startX=0.0f, startY=0.0f;
				if(bSCurrent[i])
				{
					startX = bSCurrent[i]->x;
					startY = bSCurrent[i]->y;
					//printf(": %.f %.f\n", startX, startY);
					fwrite(&bSCurrent[i]->x, 1, sizeof(float), f);
					fwrite(&bSCurrent[i]->y, 1, sizeof(float), f);
					bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				}
				while(bSCurrent[i])
				{
					float tX = startX - bSCurrent[i]->x;
					float tY = startY - bSCurrent[i]->y;
					fwrite(&tX, 1, sizeof(float), f);
					fwrite(&tY, 1, sizeof(float), f);
					startX = bSCurrent[i]->x;
					startY = bSCurrent[i]->y;
					//printf("%.f %.f => %.f %.f\n", bSCurrent[i]->x, bSCurrent[i]->y, tX, tY);
					bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				}
			}
			fclose(f);
			debugPrintf("draw%d.dat saved again",saveCount);
		}
		else { debugErrorPrintf("Couldn't open file \"%s\" for writing!\n", lamah); }
	}
	if (f2Pressed)
	{
		saveCount++;
		char lamah[64];
		sprintf(lamah, "draw%d.dat", saveCount);

		FILE *f = fopen(lamah,"rb");
		while(f)
		{
			fclose(f);

			sprintf(lamah, "draw%d.dat", ++saveCount);
			f = fopen(lamah, "rb");
		}
		if (f) { fclose(f); }

		f = fopen(lamah, "wb");
		if (f)
		{
			for(i=0; i < splinesMax; i++)
			{
				if (pointNum[i] == 0) { continue; }

				fwrite(&pointNum[i], 1, sizeof(int), f);
				fwrite(&splineWidth[i], 1, sizeof(int), f);
				bSCurrent[i] = bSHead[i];
				float startX=0.0f, startY=0.0f;
				if(bSCurrent[i])
				{
					startX = bSCurrent[i]->x;
					startY = bSCurrent[i]->y;
					//printf(": %.f %.f\n", startX, startY);
					fwrite(&bSCurrent[i]->x, 1, sizeof(float), f);
					fwrite(&bSCurrent[i]->y, 1, sizeof(float), f);
					bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				}
				while(bSCurrent[i])
				{
					float tX = startX - bSCurrent[i]->x;
					float tY = startY - bSCurrent[i]->y;
					fwrite(&tX, 1, sizeof(float), f);
					fwrite(&tY, 1, sizeof(float), f);
					startX = bSCurrent[i]->x;
					startY = bSCurrent[i]->y;
					//printf("%.f %.f => %.f %.f\n", bSCurrent[i]->x, bSCurrent[i]->y, tX, tY);
					bSCurrent[i] = (cubicSpline*)bSCurrent[i]->next;
				}
			}
			fclose(f);
			debugPrintf("Saved and created a new file draw%d.dat",saveCount);
		}
		else { debugErrorPrintf("Couldn't open file \"%s\" for writing!\n", lamah); }
	}
	
	graphicsFlush();
}

void splineEditorDeinit(void)
{
	int i;
	for(i = 0; i < SPLINES_MAX; i++)
	{
		bSCurrent[i] = bSHead[i];
		while(bSCurrent[i])
		{
			cubicSpline *next = (cubicSpline*)bSCurrent[i]->next;
			free(bSCurrent[i]);
			bSCurrent[i] = next;
		}
	}
}

void splineEditorLoad(const char *filename)
{
	FILE *f = fopen(getFilePath(filename), "rb");
	if (f)
	{
		while(fread(&pointNum[splinesNum+1], sizeof(int), 1, f))
		{
			if (fread(&splineWidth[splinesNum+1], sizeof(int), 1, f) != 1)
			{
				debugErrorPrintf("Parse error in file '%s'!", filename);
				return;
			}

			splinesNum++;
			splinesMax++;
			bSHead[splinesNum] = NULL;

			cubicSpline *prevBS = NULL;
			int i;
			for(i=0; i<pointNum[splinesNum]; i++)
			{
				cubicSpline *bs = (cubicSpline*)malloc(sizeof(cubicSpline));

				bs->next = NULL;

				if (prevBS)
				{
					float tX, tY;
					if (fread(&tX, sizeof(float), 1, f) != 1)
					{
						debugErrorPrintf("Parse error in file '%s'!", filename);
						return;
					}
					if (fread(&tY, sizeof(float), 1, f) != 1)
					{
						debugErrorPrintf("Parse error in file '%s'!", filename);
						return;
					}
					bs->x = prevBS->x - tX;
					bs->y = prevBS->y - tY;
					//printf("%.f %.f => %.f %.f\n", bs->x, bs->y, tX, tY);
				}
				else
				{
					if (fread(&bs->x, sizeof(float), 1, f) != 1)
					{
						debugErrorPrintf("Parse error in file '%s'!", filename);
						return;
					}

					if (fread(&bs->y, sizeof(float), 1, f) != 1)
					{
						debugErrorPrintf("Parse error in file '%s'!", filename);
						return;
					}
					//printf(": %.f %.f\n", bs->x, bs->y);
				}

				if (bSHead[splinesNum] == NULL)
				{
					bSHead[splinesNum] = bSTail[splinesNum] = bs;
				}
				else
				{
					bSTail[splinesNum]->next = (struct cubicSpline*)bs;
					bSTail[splinesNum] = bs;
				}

				prevBS = bs;
			}
		}

		fclose(f);
	}
	else
	{
		debugErrorPrintf(
			"Couldn't open file \"%s\" for reading!\n",
			filename
		);
	}

	if ((buttonReleased == 1) && (bSHead[splinesNum] != NULL))
	{
		splinesNum++;
		splinesMax++;
		bSHead[splinesNum] = NULL;
		pointNum[splinesNum]=0;
	}

	if (buttonPressed == 1)
	{
		cubicSpline *bs = (cubicSpline*)malloc(sizeof(cubicSpline));

		bs->next = NULL;

		bs->x = bs->bX = (float)mouseX;
		bs->y = bs->bY = (float)mouseY;


		if (bSHead[splinesNum] == NULL)
		{
			bSHead[splinesNum] = bSTail[splinesNum] = bs;
		}
		else
		{
			bSTail[splinesNum]->next = (struct cubicSpline*)bs;
			bSTail[splinesNum] = bs;
		}
		pointNum[splinesNum]++;
	}
}
