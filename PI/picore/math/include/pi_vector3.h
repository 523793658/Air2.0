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

#ifndef __PI_VECTOR3_H__
#define __PI_VECTOR3_H__

#include <pi_lib.h>

/* ǰ������ */
struct PiMatrix4;
struct PiQuaternion;

/**
 * ���ļ�������3ά����������صĴ�����
 */

typedef struct PiVector3
{
	float x, y, z;
}PiVector3;

PI_BEGIN_DECLS

/* ��ֵ��dst={x,y,z} */
static void pi_vec3_set(PiVector3 *dst, float x, float y, float z);

/* ������dst = src */
static void pi_vec3_copy(PiVector3 *dst, const PiVector3 *src);

/* �������������Ƿ���� */
static PiBool pi_vec3_is_equal(const PiVector3 *src1, const PiVector3 *src2);

/* ������� dst = src1 + src2 */
static void pi_vec3_add(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* ������� dst = src1 - src2 */
static void pi_vec3_sub(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* ����������� dst = src1 * src2 */
static void pi_vec3_mul(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* ����������� dst = src1 / src2 */
static void pi_vec3_div(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* �������� */
static void pi_vec3_scale(PiVector3 *dst, const PiVector3 *src, float scale);

/* ������� return dot(src1, src2) */
static float pi_vec3_dot(const PiVector3 *src1, const PiVector3 *src2);

/* ������� dst = Cross(src1, src2) */
static void pi_vec3_cross(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* ȡ���������ȵ�ƽ�� */
static float pi_vec3_len_square(const PiVector3 *src);

/* ȡ���������� */
static float pi_vec3_len(const PiVector3 *src);

/* ȡ���������������ƽ�� */
static float pi_vec3_distance_square(const PiVector3 *src1, const PiVector3 *src2);

/* ȡ�������������� */
static float pi_vec3_distance(const PiVector3 *src1, const PiVector3 *src2);

/* ���������Բ�ֵ dst = src1 + frac * (src2 - src1) */
static void pi_vec3_lerp(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2, float frac);

/* ȡ��0����{0, 0, 0} */
const PiVector3* pi_vec3_get_zero(void);

/* ȡ��x�ᵥλ���� {1, 0, 0} */
const PiVector3* pi_vec3_get_xunit(void);

/* ȡ��y�ᵥλ���� {0, 1, 0} */
const PiVector3* pi_vec3_get_yunit(void);

/* ȡ��z�ᵥλ����  {0, 0, 1} */
const PiVector3* pi_vec3_get_zunit(void);

/* ȡ�����ŵ�λ����  {1, 1, 1} */
const PiVector3* pi_vec3_get_scale_unit(void);

/**
 * ��λ������ dst = norm(src)
 * ���src����Ϊ0.0f����λ��ʧ�ܣ�����FALSE����ʱdst����Ϊ0����
 */
PiBool pi_vec3_normalise(PiVector3 *dst, const PiVector3 *src);

/* �������������ļнǣ���λ������ */
float pi_vec3_angle(const PiVector3 *src1, const PiVector3 *src2);

#include "vector3.inl"

PI_END_DECLS

#endif /* __PI_VECTOR3_H__ */
