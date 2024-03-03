#ifndef SYSTEM_DATATYPES_DATATYPES_H_
#define SYSTEM_DATATYPES_DATATYPES_H_

#include "string.h"

/**
 * 2D coordinate
 */
typedef struct {
	float x, y;
} point2d_t;

/**
 * 3D coordinate
 */
typedef struct {
	float x, y, z;
} point3d_t;

/**
 * RGBA color information. Floating point 0.0 - 1.0.
 */
typedef struct {
	float r, g, b, a;
} color_t;

/**
 * Dimension information
 */
typedef struct {
	int width, height;
} dimension_t;

/**
 * 4x4 matrix
 */
typedef struct {
	float m[16];
} matrix44_t;

/**
 * 3x3 matrix
 */
typedef struct {
	float m[9];
} matrix33_t;

extern void setPoint3d(point3d_t *p, float x, float y, float z);
extern point3d_t sumPoint3d(point3d_t p1, point3d_t p2);
extern point3d_t subtractPoint3d(point3d_t p1, point3d_t p2);

#endif /* SYSTEM_DATATYPES_DATATYPES_H_ */
