#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "system/timer/timer.h"
#include "datatypes.h"

void setPoint3d(point3d_t *p, float x, float y, float z)
{
	p->x = x;
	p->y = y;
	p->z = z;
}

point3d_t sumPoint3d(point3d_t p1, point3d_t p2)
{
	point3d_t t;
	t.x = p1.x + p2.x;
	t.y = p1.y + p2.y;
	t.z = p1.z + p2.z;

	return t;
}

point3d_t subtractPoint3d(point3d_t p1, point3d_t p2)
{
	point3d_t t;
	t.x = p1.x - p2.x;
	t.y = p1.y - p2.y;
	t.z = p1.z - p2.z;

	return t;
}
