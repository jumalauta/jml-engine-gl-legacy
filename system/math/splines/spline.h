#ifndef EXH_SYSTEM_MATH_SPLINES_SPLINE_H_
#define EXH_SYSTEM_MATH_SPLINES_SPLINE_H_

/* splineContainer types */
#define CUBIC  0
#define BEZIER 1

/* spline draw types */
#define LINE_STRIP 0
#define POINTS     1

typedef struct {
	float x, y, z;
} point3d;

typedef struct {
	/*
	add some spline related information here
	*/
	float x, y, z;
	struct splinePoint *next;
} splinePoint;

typedef struct {
	/*
	add some spline related information here
	*/
	unsigned int size;
	unsigned int detail;
	float width;

	splinePoint *splinePointTail;
	splinePoint *splinePointHead;
	struct spline *next;
} spline;

typedef struct {
	/*
	add some spline related information here
	*/
	unsigned int size;

	spline *splineTail;
	spline *splineHead;
	struct splineLayer *next;
} splineLayer;

typedef struct {
	/*
	add some spline related information here
	*/
	unsigned int size;
	unsigned int pointsCount;

	splineLayer *splineLayerTail;
	splineLayer *splineLayerHead;
	struct splineContainer *next;
} splineContainer;

extern splinePoint *addSplinePoint(splineContainer *container, spline *s, float x, float y, float z);
extern spline *addSpline(splineLayer *layer);
extern splineLayer *addSplineLayer(splineContainer *container);
extern splineContainer *addSplineContainer(void);
extern splineContainer *loadSplineContainerFromFile(const char *filename);
extern void drawSplineContainer(splineContainer *container, float start, float end);

#endif /*EXH_SYSTEM_MATH_SPLINES_SPLINE_H_*/
