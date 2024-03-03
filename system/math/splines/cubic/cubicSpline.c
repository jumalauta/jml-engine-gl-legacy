#include "cubicSpline.h"

//http://mathworld.wolfram.com/CubicSpline.html

typedef struct cubic
{
	float a, b, c, d;
} cubic;

typedef struct cubic3d
{
	cubic x, y, z;
} cubic3d;

#define cubicInterpolate(p,a,b,c,d) ((((d*p) + c)*p + b)*p + a)

void createCubicSpline(point3d *dest, int n, int detail, point3d *spline)
{
	cubic3d temp[n];
	float factor[n+1];
	point3d point[n+1], deltaPoint[n+1];
	int i, j, k;

	factor[0] = 1.0f/2.0f;
	point[0].x = deltaPoint[0].x = 3*(spline[1].x-spline[0].x)*factor[0];
	point[0].y = deltaPoint[0].y = 3*(spline[1].y-spline[0].y)*factor[0];
	point[0].z = deltaPoint[0].z = 3*(spline[1].z-spline[0].z)*factor[0];
	for (i = 1; i < n; i++)
	{
		factor[i] = 1/(4-factor[i-1]);
	
		deltaPoint[i].x = (3*(spline[i+1].x-spline[i-1].x)-deltaPoint[i-1].x)*factor[i];
		deltaPoint[i].y = (3*(spline[i+1].y-spline[i-1].y)-deltaPoint[i-1].y)*factor[i];
		deltaPoint[i].z = (3*(spline[i+1].z-spline[i-1].z)-deltaPoint[i-1].z)*factor[i];

		point[i].x = deltaPoint[i].x - factor[i]*point[i-1].x;
		point[i].y = deltaPoint[i].y - factor[i]*point[i-1].y;
		point[i].z = deltaPoint[i].z - factor[i]*point[i-1].z;
	}
	factor[n] = 1/(2-factor[n-1]);
	point[n].x = deltaPoint[n].x = (3*(spline[n].x-spline[n-1].x)-deltaPoint[n-1].x)*factor[n];
	point[n].y = deltaPoint[n].y = (3*(spline[n].y-spline[n-1].y)-deltaPoint[n-1].y)*factor[n];
	point[n].z = deltaPoint[n].z = (3*(spline[n].z-spline[n-1].z)-deltaPoint[n-1].z)*factor[n];

	for (i = 0, j = 0; i < n; i++)
	{
		temp[i].x.a = (float)spline[i].x;
		temp[i].x.b = point[i].x;
		temp[i].x.c = 3*(spline[i+1].x - spline[i].x) - 2*point[i].x - point[i+1].x;
		temp[i].x.d = 2*(spline[i].x - spline[i+1].x) + point[i].x + point[i+1].x;

		temp[i].y.a = (float)spline[i].y;
		temp[i].y.b = point[i].y;
		temp[i].y.c = 3*(spline[i+1].y - spline[i].y) - 2*point[i].y - point[i+1].y;
		temp[i].y.d = 2*(spline[i].y - spline[i+1].y) + point[i].y + point[i+1].y;

		temp[i].z.a = (float)spline[i].z;
		temp[i].z.b = point[i].z;
		temp[i].z.c = 3*(spline[i+1].z - spline[i].z) - 2*point[i].z - point[i+1].z;
		temp[i].z.d = 2*(spline[i].z - spline[i+1].z) + point[i].z + point[i+1].z;

		for (k = 0; k < detail; k++, j++)
		{
			float p = k / (float) detail;
			dest[j].x = cubicInterpolate(p, temp[i].x.a, temp[i].x.b, temp[i].x.c, temp[i].x.d);
			dest[j].y = cubicInterpolate(p, temp[i].y.a, temp[i].y.b, temp[i].y.c, temp[i].y.d);
			dest[j].z = cubicInterpolate(p, temp[i].z.a, temp[i].z.b, temp[i].z.c, temp[i].z.d);
		}
	}
}
