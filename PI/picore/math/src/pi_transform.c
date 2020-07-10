#include "pi_transform.h"


TransformData* PI_API pi_transform_new()
{
	return pi_new0(TransformData, 1);
}

void PI_API pi_transform_copy(TransformData* dst, TransformData* src)
{
	pi_memcpy_inline(dst, src, sizeof(TransformData));
}

void PI_API pi_transform_mul_mat(TransformData* dst, TransformData* src1, PiMatrix4* src2)
{
	PiMatrix4 m;
	pi_mat4_build_transform(&m, &src1->translate, &src1->scale, &src1->rotate);
	pi_mat4_mul(&m, &m, src2);
	pi_mat4_decompose(&dst->translate, &dst->scale, &dst->rotate, &m);
}

void PI_API pi_transform_mul(TransformData* dst, TransformData* src1, TransformData* src2)
{

}

void PI_API pi_transform_set(TransformData* dst, float tx, float ty, float tz, float rx, float ry, float rz, float rw)
{
	pi_vec3_set(&dst->translate, tx, ty, tz);
	pi_quat_set(&dst->rotate, rw, rx, ry, rz);
}