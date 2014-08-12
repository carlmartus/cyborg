#include "util.h"
#include <math.h>

float
math_angle(float ax, float ay, float bx, float by)
{
	return atan2f(
			bx - ax,
			by - ay);
}

float
math_dist(float ax, float ay, float bx, float by)
{
	float dx, dy;
	dx = bx - ax;
	dy = by - ay;
	return sqrtf(dx*dx + dy*dy);
}

