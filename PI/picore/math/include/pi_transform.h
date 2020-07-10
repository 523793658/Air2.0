#ifndef _Pi_Transform_H_
#define _Pi_Transform_H_
#include "pi_vector3.h"
#include "pi_quaternion.h"

/* �任���� */
typedef struct
{
	PiVector3 scale;			/* �������� */
	PiVector3 translate;		/* ƽ������ */
	PiQuaternion rotate;		/* ��ת��Ԫ�� */
} TransformData;

PI_BEGIN_DECLS

TransformData* PI_API pi_transform_new();

void PI_API pi_transform_copy(TransformData* dst, TransformData* src);

void PI_API pi_transform_mul_mat(TransformData* dst, TransformData* src1, PiMatrix4* src2);

void PI_API pi_transform_mul(TransformData* dst, TransformData* src1, TransformData* src2);

void PI_API pi_transform_set(TransformData* dst, float tx, float ty, float tz, float rx, float ry, float rz, float rw);


PI_END_DECLS

#endif