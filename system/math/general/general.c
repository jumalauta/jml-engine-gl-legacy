/*
 Contains some general mathematic functions not suitable for other modules
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "graphicsIncludes.h"
#include "system/ui/window/window.h"
#include "system/debug/debug.h"

#include "general.h"

double interpolateSmoothStep(double p, double a, double b)
{
	double x = getClamp((interpolateLinear(p, a, b) - a)/(b - a), 0.0, 1.0);
    return x*x*(3 - 2*x);
}

double interpolateSmootherStep(double p, double a, double b)
{
    double x = getClamp((interpolateLinear(p, a, b) - a)/(b - a), 0.0, 1.0);
    return x*x*x*(x*(x*6 - 15) + 10);
}

/*
X - Axis:
|1  0    0    0|
|0  cos  sin  0|
|0 -sin  cos  0|
|0  0    0    1|

Y - Axis:
|cos  0 -sin  0|
|0    1  0    0|
|sin  0  cos  0|
|0    0  0    1|

Z - Axis:
|cos  sin 0   0|
|-sin cos 0   0|
|0    0   1   0|
|0    0   0   1|
*/


void normalizeVector(point3d_t *point3d)
{
	float l = sqrt((point3d->x * point3d->x) + (point3d->y * point3d->y) + (point3d->z * point3d->z));
	point3d->x /= l;
	point3d->y /= l;
	point3d->z /= l;
}

double innerProduct(point3d_t vec1, point3d_t vec2)
{
	return (vec1.x * vec2.x
		+ vec1.y * vec2.y
		+ vec1.z * vec2.z);
}

point3d_t crossProduct(point3d_t vec1, point3d_t vec2)
{
	point3d_t result;
	
	result.x = vec1.y * vec2.z - vec2.y * vec1.z;
	result.y = vec1.z * vec2.x - vec2.z * vec1.x;
	result.z = vec1.x * vec2.y - vec2.x * vec1.y;
	
	return result;
}	

/*glRotate produces a rotation of angle	degrees	around the
 vector (x,y,z).  The current matrix (see glMatrixMode) is
 multiplied by	a rotation matrix with the product replacing
 the current matrix, as if glMultMatrix were called with the
 following matrix as its argument:
 
 | xx(1-c)+c   xy(1-c)-zs  xz(1-c)+ys 0  |
 | yx(1-c)+zs  yy(1-c)+c   yz(1-c)-xs 0  |
 | xz(1-c)-ys  yz(1-c)+xs  zz(1-c)+c  0  |
 | 0           0           0          1  |
 
 Where	c = cos(angle),	s = sine(angle), and ||( x,y,z )|| = 1
 (if not, the GL will normalize this vector).
 */

void setRotatef(point3d_t *point3d, float angle, float x, float y, float z)
{

	angle += 45;
	angle = DEG_TO_RAD(angle);
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	
	point3d_t p3d = *point3d;
	normalizeVector(&p3d);
	
	if ((point3d->y>0 && point3d->x<0) || (point3d->y < 0 && point3d->x > 0))
	{
		point3d->y *= x*(1-c)+c   + y*(1-c)-z*s;
		point3d->x *= x*(1-c)+z*s + y*(1-c)+c;
	}
	else
	{
		point3d->x *= x*(1-c)+c   + y*(1-c)-z*s;
		point3d->y *= x*(1-c)+z*s + y*(1-c)+c;
	}
}


void rotateZ(point3d_t *point3d, float angle)
{
	/*
	 x' = x*cos q - y*sin q
	 y' = x*sin q + y*cos q
	 z' = z
	 */
	angle += 45;
	angle = DEG_TO_RAD(angle);
	float ca = (float)cos(angle);
	float sa = (float)sin(angle);

	point3d->x = point3d->x*ca - point3d->y*sa;
	point3d->y = point3d->x*sa + point3d->y*ca;
}

void rotateY(point3d_t *point3d, float angle)
{
	/*
	 z' = z*cos q - x*sin q
	 x' = z*sin q + x*cos q
	 y' = y
	 */
	angle = DEG_TO_RAD(angle);
	float ca = (float)cos(angle);
	float sa = (float)sin(angle);
	point3d->z = point3d->z * ca - point3d->x * sa;
	point3d->x = point3d->z * sa + point3d->x * ca;
}

void rotateX(point3d_t *point3d, float angle)
{
	/*
	 y' = y*cos q - z*sin q
	 z' = y*sin q + z*cos q
	 x' = x
	 */
	angle = DEG_TO_RAD(angle);
	float ca = (float)cos(angle);
	float sa = (float)sin(angle);
	point3d->y = point3d->y * ca - point3d->z * sa;
	point3d->z = point3d->y * sa + point3d->z * ca;
}

#ifndef TINYGL
//Converts 3D point to screen 2D coordinate
point2d_t getScreenCoordinateFrom3dCoordinate(point3d_t point3d)
{
	point2d_t screenPoint2d;
	
	double x,y,z;
	double model_view[16];
	double projection[16];
	int viewport[4];
	
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	
	gluProject(point3d.x, point3d.y, point3d.z,
			   model_view, projection, viewport,
			   &x, &y, &z);
	
	screenPoint2d.x = x;
	screenPoint2d.y = y;
	
	return screenPoint2d;
}
#endif

