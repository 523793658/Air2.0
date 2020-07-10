#ifndef _Vector_H_
#define _Vector_H_
#include "pi_vector3.h"
typedef struct PiVector4_
{
	float x, y, z, w;
}PiVector4;

typedef struct PiVector4I_
{
	int x, y, z, w;
}PiVector4I;

PI_BEGIN_DECLS

void pi_math_vec4_copy_from_vec3(PiVector4* dst, PiVector3* src);

void pi_math_vec4_set(PiVector4* dst, float x, float y, float z, float w);

PI_END_DECLS


#endif

