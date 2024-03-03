#ifndef EXH_SYSTEM_MATH_GENERAL_MATH_DEFINES_H_
#define EXH_SYSTEM_MATH_GENERAL_MATH_DEFINES_H_

#include <math.h>

//to have backwards compatibility, please remove at some point
#define DEG_TO_RAD(x) ((x)*(M_PI/180.0))
#define RAD_TO_DEG(x) ((x)*(180.0/M_PI))

//general purpose macro functions
#define degToRad(x) ((x)*(M_PI/180.0))
#define radToDeg(x) ((x)*(180.0/M_PI))
#define getMin(A, B) ((A)<(B)?(A):(B))
#define getMax(A, B) ((A)>(B)?(A):(B))
#define getClamp(value, min, max) (getMin((max), getMax((value), (min))))
#define interpolateLinear(P, A, B) (P)*((B)-(A)) + (A)

#endif /*EXH_SYSTEM_MATH_GENERAL_MATH_DEFINES_H_*/
