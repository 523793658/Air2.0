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

#ifndef __PI_MATRIX4_H__
#define __PI_MATRIX4_H__

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_quaternion.h>

/**
 * ���ļ�������4*4�ľ�������صĴ�����
 * ע�⣺������������������˵ľ���
 * ����m������v��ˣ�m * v��v��������:
 * 
 * [ m[0][0]  m[0][1]  m[0][2]  m[0][3] ]   {v[0]}
 * | m[1][0]  m[1][1]  m[1][2]  m[1][3] | * {v[1]}
 * | m[2][0]  m[2][1]  m[2][2]  m[2][3] |   {v[2]}
 * [ m[3][0]  m[3][1]  m[3][2]  m[3][3] ]   {1}
 *
 * תOpenGL�����ʱ����Ҫ��mת�ù���(OpenGL��ȻҲ�Ǻ���������ˣ�����OpenGL�ľ���Ҳ��������)
 * תD3D�����ʱ��Ҳ��Ҫ��mת�ù�����D3D�����������ȣ��������������;�����ˣ�
 * �����õ�����������ϵ����OpenGL��ͬ����D3D�෴
 *
 * ��CVV�õ���OpenGL�ķ�Χ��-1 <= x <= 1, -1 <= y <= 1, -1 <= z <= 1
 */

/* ǰ������ */
struct PiVector3;
struct PiQuaternion;
				 
typedef struct PiMatrix4
{
	float m[4][4];
}PiMatrix4;

PI_BEGIN_DECLS

/* dst = src */
static void pi_mat4_copy(PiMatrix4 *dst, const PiMatrix4 *src2);

/* �ж���� */
static PiBool pi_mat4_is_equal(const PiMatrix4 *src1, const PiMatrix4 *src2);

/* ����ӷ� */
static void pi_mat4_add(PiMatrix4 *dst, const PiMatrix4 *src1, const PiMatrix4 *src2);

/* �����һ������ */
static void pi_mat4_scale(PiMatrix4 *dst, const PiMatrix4 *src, float scale);

/* ת�þ���dst = Transpose(src) */
static void pi_mat4_transpose(PiMatrix4 *dst, const PiMatrix4 *src);

/* ���������ڵ㣺ƽ�Ʒ��������� */
static void pi_mat4_apply_point(PiVector3 *dst, const PiVector3 *src, const PiMatrix4 *mat);

/* ����������������ƽ�Ʒ����������� */
static void pi_mat4_apply_vector(PiVector3 *dst, const PiVector3 *src, const PiMatrix4 *mat);

/* ����ƽ�ƾ��� */
static void pi_mat4_set_translate(PiMatrix4 *dst, const PiVector3 *translate);

/* ��ȡƽ�Ʒ��� */
static void pi_mat4_extract_translate( PiVector3 *dst, const PiMatrix4 *mat );

/* �Ѿ����ƽ��������ת�ֽ���� */
void pi_mat4_decompose(PiVector3 *pos, PiVector3 *scale, struct PiQuaternion *rotate, const PiMatrix4 *mat);

/* �������ž��� */
static void pi_mat4_set_scale(PiMatrix4 *dst, const PiVector3 *scale);

/* ��ʼ��dstΪ��λ�� */
void pi_mat4_set_identity(PiMatrix4 *dst);

/* ȡ��λ���� */
PiMatrix4* pi_mat4_get_identity(void);

/* ������ˣ�dst = src1 * src2 ע�⣺�������������������������ϵľ������ */
void pi_mat4_mul(PiMatrix4 *dst, const PiMatrix4 *src1, const PiMatrix4 *src2);

/* �����dst = Inverse(src) */
void pi_mat4_inverse(PiMatrix4 *dst, const PiMatrix4 *src);

/* ������ת���� */
void pi_mat4_build_rotate(PiMatrix4 *dst, const struct PiQuaternion *rotate);

/**
 * ����translate��rotate��scale������任����
 * ע��Լ����˳����scale����rotate�������translate
 */
void pi_mat4_build_transform(PiMatrix4 *dst, 
	const PiVector3 *translate, const PiVector3 *scale, const struct PiQuaternion *rotate);

/** ������������ϵ����ͼ���� */
PiBool pi_mat4_lookat_rh(PiMatrix4 *dst, const PiVector3 *pos, const PiVector3 *target, const PiVector3 *up);

/** 
 * ��������ϵ��͸��ͶӰ����
 * lt��rt��bm��tpΪ���ü���ľ�������ҡ��ϡ��±߽����ֵ
 * ע�⣺����͸�Ӿ���������λ�ò���λ�ڲü����ڲ�������zn��zf����ȡ��ֵ������znԽ�٣����������Խ��
 */
void pi_mat4_frustum_rh(PiMatrix4 *dst, float lt, float rt, float bm, float tp, float zn, float zf);

/** 
 * ����һ������ϵ��͸��ͶӰ����
 * fovRad: yz�������Ұ�Ƕȣ���λ����
 * zNear��zFar����Զ�ü��浽����ľ��룬��Ϊ����������zNear���ܵ���zFar
 * ratio����߱� 
 */
void pi_mat4_pers_fov_rh(PiMatrix4 *dst, float fovRad, float ratio, float zNear, float zFar);

/* ����һ������ϵ������ͶӰ����
 * lt��zf������ռ��еĳ�����߽�
 * zNear��zFar�����ɸ�����zNear��zfar������� 
 * zNear ��ʾzֵ����Сֵ
 * zFar ��ʾzֵ�����ֵ
 */
void pi_mat4_ortho_rh(PiMatrix4 *dst, float lt, float rt, float bm, float tp, float zn, float zf);

#include "matrix4.inl"

PI_END_DECLS

#endif /* __PI_MATRIX4_H__ */
