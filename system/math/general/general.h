#ifndef EXH_SYSTEM_MATH_GENERAL_GENERAL_H_
#define EXH_SYSTEM_MATH_GENERAL_GENERAL_H_

#include "linmath.h"
#include "math_defines.h"
#include "system/datatypes/datatypes.h"

#include "system/debug/debug.h"

#include "expr.h"

double interpolateSmoothStep(double p, double a, double b);
double interpolateSmootherStep(double p, double a, double b);

extern void normalizeVector(point3d_t *point3d);
extern double innerProduct(point3d_t vec1, point3d_t vec2);
extern point3d_t crossProduct(point3d_t vec1, point3d_t vec2);

extern void setRotatef(point3d_t *point3d, float angle, float x, float y, float z);

extern void rotateZ(point3d_t *point3d, float angle);
extern void rotateY(point3d_t *point3d, float angle);
extern void rotateX(point3d_t *point3d, float angle);

#ifndef TINYGL
extern point2d_t getScreenCoordinateFrom3dCoordinate(point3d_t point_3d);
#endif

#endif /*EXH_SYSTEM_MATH_SORT_INSERTSORT_H_*/
