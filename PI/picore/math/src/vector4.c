#include "vector4.h"
void pi_math_vec4_copy_from_vec3(PiVector4* dst, PiVector3* src)
{
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;
}


void pi_math_vec4_set(PiVector4* dst, float x, float y, float z, float w)
{
	dst->x = x;
	dst->y = y;
	dst->z = z;
	dst->w = w;
}

