#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "graphicsIncludes.h"
#include "spline.h"
#include "cubic/cubicSpline.h"
#include "system/debug/debug.h"
#include "system/io/io.h"
#include "system/datatypes/memory.h"

/*
splineContainer - mainContainer for all the splines
splineLayer	- a splineLayer in a splineContainer
spline		- a spline in a splineLayer
splinePoint	- a point in a spline

loadFile(a)
	-> addSplineContainer (drawing.dat)
		-> addSplineLayer - NOTE: splineLayers are not implemented in the editor
			-> addSpline 
				-> addSplinePoint
*/

splineContainer *splineContainerHead = NULL;
splineContainer *splineContainerTail = NULL;

splinePoint *addSplinePoint(splineContainer *container, spline *s, float x, float y, float z)
{
	splinePoint *sP
		= (splinePoint*)malloc(sizeof(splinePoint));

	if (sP)
	{
		container->pointsCount += s->detail;

		s->size++;

		sP->x = x;
		sP->y = y;
		sP->z = z;
		sP->next = NULL;

		if (s->splinePointHead == NULL)
		{
			s->splinePointHead
				= s->splinePointTail = sP;
		}
		else
		{
			s->splinePointTail->next
				= (struct splinePoint*)sP;
			s->splinePointTail = sP;
		}
	}

	return sP;
}

spline *addSpline(splineLayer *layer)
{
	spline *s
		= (spline*)malloc(sizeof(spline));

	if (s)
	{
		layer->size++;

		s->size = 0;
		s->detail = 10;
		s->width = 3.0f;

		s->splinePointHead = NULL;
		s->splinePointTail = NULL;
		s->next = NULL;

		if (layer->splineHead == NULL)
		{
			layer->splineHead
				= layer->splineTail = s;
		}
		else
		{
			layer->splineTail->next
				= (struct spline*)s;
			layer->splineTail = s;
		}
	}

	return s;
}

splineLayer *addSplineLayer(splineContainer *container)
{
	splineLayer *sL
		= (splineLayer*)malloc(sizeof(splineLayer));

	if (sL)
	{
		container->size++;

		sL->size = 0;

		sL->splineHead = NULL;
		sL->splineTail = NULL;
		sL->next = NULL;

		if (container->splineLayerHead == NULL)
		{
			container->splineLayerHead
				= container->splineLayerTail = sL;
		}
		else
		{
			container->splineLayerTail->next
				= (struct splineLayer*)sL;
			container->splineLayerTail = sL;
		}
	}

	return sL;
}

splineContainer *addSplineContainer(void)
{
	splineContainer *sC
		= (splineContainer*)malloc(sizeof(splineContainer));

	if (sC)
	{
		sC->size = 0;
		sC->pointsCount=0;

		sC->splineLayerHead = NULL;
		sC->splineLayerTail = NULL;
		sC->next = NULL;

		if (splineContainerHead == NULL)
		{
			splineContainerHead = splineContainerTail = sC;
		}
		else
		{
			splineContainerTail->next
				= (struct splineContainer*)sC;
			splineContainerTail = sC;
		}
	}

	return sC;
}

static void deleteSplineContainer(void *containerPointer)
{
	splineContainer *current = (splineContainer *)containerPointer;
	while(current)
	{
		splineLayer *splineLayerCurrent = current->splineLayerHead;
		while(splineLayerCurrent)
		{
			spline *splineCurrent = splineLayerCurrent->splineHead;
			while(splineCurrent)
			{
				splinePoint *splinePointCurrent = splineCurrent->splinePointHead;
				while(splinePointCurrent)
				{
					splinePoint *splinePointNext = (splinePoint*)splinePointCurrent->next;
					free(splinePointCurrent);
					splinePointCurrent = splinePointNext;
				}
				spline *splineNext = (spline*)splineCurrent->next;
				free(splineCurrent);
				splineCurrent = splineNext;
			}

			splineLayer *splineLayerNext = (splineLayer*)splineLayerCurrent->next;
			free(splineLayerCurrent);
			splineLayerCurrent = splineLayerNext;
		}

		splineContainer *next = (splineContainer*)current->next;
		if (containerPointer != (void*)current)
		{
			free(current); //Generic garbage collection will handle the first pointer
		}

		current = next;
	}
}

static splineContainer *loadSplineContainer(const char *data, unsigned int size)
{
	splineContainer *container = addSplineContainer();
	splineLayer *layer = addSplineLayer(container); //NOTE: fileformat doesn't support layers yet
	unsigned int i = 0;
	while(i < size)
	{
		spline *s = addSpline(layer);

		unsigned int splineSize;
		memcpy(&splineSize, &data[i], sizeof(int));
		i += sizeof(int);

		float splineWidth;
		memcpy(&splineWidth, &data[i], sizeof(float));
		s->width = splineWidth>=1.0f?splineWidth:3.0f;
		i += sizeof(float);

		float prevX=0.0f, prevY=0.0f;
		unsigned int j;
		for(j = 0; j < splineSize; j++)
		{
			float x,y;
			if (j == 0)
			{
				memcpy(&x, &data[i], sizeof(float));
				i += sizeof(float);
				memcpy(&y, &data[i], sizeof(float));
				i += sizeof(float);
			}
			else
			{
				float tX, tY;
				memcpy(&tX, &data[i], sizeof(float));
				i += sizeof(float);
				memcpy(&tY, &data[i], sizeof(float));
				i += sizeof(float);

				x = prevX - tX;
				y = prevY - tY;
			}

			prevX = x;
			prevY = y;
			addSplinePoint(container, s, x, y, 0.0f);
		}
	}

	return container;
}

splineContainer *loadSplineContainerFromFile(const char *filename)
{
	unsigned int fileSize = 0;
	char *data = ioReadFileToBuffer(filename, &fileSize);
	assert(data);
	
	splineContainer *container = loadSplineContainer(data, (unsigned int)fileSize);
	memoryAddGeneralPointerToGarbageCollection(container, deleteSplineContainer);
	free(data);
	
	return container;
}

void drawSplineContainer(splineContainer *container, float start, float end)
{
	int pointsCount = container->pointsCount*start, i;
	splineLayer *splineLayerCurrent = container->splineLayerHead;
	while(splineLayerCurrent)
	{
		spline *splineCurrent = splineLayerCurrent->splineHead;
		while(splineCurrent)
		{
			if (splineCurrent->size > 0)
			{
				point3d *roughSpline = (point3d*)malloc(splineCurrent->size*sizeof(point3d));
				point3d *lineSmooth = (point3d*)malloc((splineCurrent->size)*splineCurrent->detail*sizeof(point3d));
				splinePoint *splinePointCurrent = splineCurrent->splinePointHead;

				i=0;
				while(splinePointCurrent)
				{
					roughSpline[i].x = splinePointCurrent->x;
					roughSpline[i].y = splinePointCurrent->y;
					roughSpline[i].z = splinePointCurrent->z;

					i++;
					splinePointCurrent = (splinePoint*)splinePointCurrent->next;
				}

				createCubicSpline(lineSmooth, splineCurrent->size-1, splineCurrent->detail, roughSpline);
				const float splinePoints = (splineCurrent->size-1)*splineCurrent->detail;
				glLineWidth(splineCurrent->width);
				glBegin(GL_LINE_STRIP);

				for(i = splinePoints*start; i < splinePoints; i++, pointsCount++)
				{
					glVertex3f(lineSmooth[i].x, lineSmooth[i].y, lineSmooth[i].z);
					if ((end < 1.0f) && (pointsCount >= container->pointsCount*end))
					{
						glEnd();
						free(roughSpline);
						free(lineSmooth);
						return;
					}
				}
				glEnd();
				
				free(roughSpline);
				free(lineSmooth);
			}

			splineCurrent = (spline*)splineCurrent->next;
		}

		splineLayerCurrent = (splineLayer*)splineLayerCurrent->next;
	}
}
