/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __PI_QUATERNION_H__
#define __PI_QUATERNION_H__

/**
 * ���ļ���������Ԫ��������صĴ�����
 */

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_matrix4.h>

/* ǰ������ */
struct PiVector3;
struct PiMatrix4;

typedef struct PiQuaternion
{
	float w, x, y, z;
}PiQuaternion;

PI_BEGIN_DECLS

/* dst = {w, x, y, z} */
static void pi_quat_set(PiQuaternion *dst, float w, float x, float y, float z);

/* dst = src */
static void pi_quat_copy(PiQuaternion *dst, const PiQuaternion *src);

/* ����TRUE��ʾsrc1��src2��� */
static PiBool pi_quat_is_equal(const PiQuaternion *src1, const PiQuaternion *src2);

/* ��Ԫ����ӣ�dst = src1 + src2 */
static void pi_quat_add(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2);

/* ��Ԫ�������dst = src1 - src2 */
static void pi_quat_sub(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2);

static void pi_quat_scale(PiQuaternion *dst, const PiQuaternion *src, float scale);

/* ��Ԫ���ĵ�� */
static float pi_quat_dot(const PiQuaternion *src1, const PiQuaternion *src2);

/* ��Ԫ��ģ��ƽ�� */
static float pi_quat_len_square(const PiQuaternion *src);

/* ������Ԫ�� dst = Conj(src) */
static void pi_quat_conjugate(PiQuaternion *dst, const PiQuaternion *src);

/* ��Ԫ����ģ */
static float pi_quat_len(const PiQuaternion *src);

/* ���ص�λ��Ԫ����ָ�� */
PiQuaternion* pi_quat_get_unit(void);

/* ��Ԫ����ˣ�dst = src1 * src2 */
void pi_quat_mul(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2);

/* ��Ԫ���ĵ�λ�� */
PiBool pi_quat_normalise(PiQuaternion *dst, const PiQuaternion *src);

/* ��Ԫ������ */
PiBool pi_quat_inverse(PiQuaternion *dst, const PiQuaternion *src);

/* ����ת����ת��Ϊ��Ԫ�� */
void pi_quat_from_mat4(PiQuaternion *dst, const struct PiMatrix4 *src);

/**
 * ����ͻ��Ƚǹ�����Ԫ��
 * ע�⣺axis�����ǵ�λ����
 */
void pi_quat_from_angle_axis(PiQuaternion *dst, const PiVector3 *axis, float radAngle);

/* ����Ԫ�������ͻ��Ƚ� */
void pi_quat_to_angle_axis(const PiQuaternion *src, PiVector3 *axis, float *radAngle);

/* ��Ԫ����ת���� */
void pi_quat_rotate_vec3(PiVector3 *dst, const PiVector3 *src, const PiQuaternion *rotate);

/* ��Ԫ���������Բ�ֵ��isShortest��ʾ���Ƿ�ѡ����̻� */
void pi_quat_slerp(PiQuaternion *dst, const PiQuaternion *from, const PiQuaternion *to, float frac, PiBool isShortest);

/* ��Ԫ�����Բ�ֵ��isShortest��ʾ���Ƿ�ѡ����̻� */
void pi_quat_lerp(PiQuaternion *dst, const PiQuaternion *from, const PiQuaternion *to, float frac, PiBool isShortest);

/* �����src1ת��src2����Ԫ����dst = Rotate(from, to) */
void pi_quat_rotate_to(PiQuaternion *dst, const PiVector3 *from, const PiVector3 *to);

/* �õ��ֲ���x�� */
void pi_quat_get_xaxis(PiQuaternion *q, PiVector3 *dst);

/* �õ��ֲ���y�� */
void pi_quat_get_yaxis(PiQuaternion *q, PiVector3 *dst);

/* �õ��ֲ���z�� */
void pi_quat_get_zaxis(PiQuaternion *q, PiVector3 *dst);

/* �õ���z��ת���Ļ��� */
float pi_quat_get_roll_rad(PiQuaternion *q);

/* �õ���x��ת���Ļ��� */
float pi_quate_get_pitch_rad(PiQuaternion *q);

/* �õ���y��ת���Ļ��� */
float pi_quat_get_yaw_rad(PiQuaternion *q);

/* �� x-y-z ��õ���Ԫ����scene ģ��� set_direction Ҫ�� */
void pi_quat_from_axes(PiQuaternion* quat, PiVector3* xaxis, const PiVector3* yaxis, const PiVector3* zaxis);

void pi_quat_from_euler_angle(PiQuaternion* quat, float pitch, float yaw, float roll);

#include "quaternion.inl"

PI_END_DECLS

#endif /* __PI_QUATERNION_H__ */
