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

#ifndef __PI_PLANE_H__
#define __PI_PLANE_H__

#include <pi_lib.h>
#include <pi_vector3.h>

/** ƽ�淽���ǣ�
 * v.x * x + v.y * y + v.z * z + d = 0 
 * ��v��λ����v����ƽ��ķ�������
 * v��λ��ʱ��d��������ԭ�㵽ƽ��ľ���
 */
typedef struct 
{
	float d;
	PiVector3 v;
}PiPlane;

PI_BEGIN_DECLS

static void pi_plane_set(PiPlane *plane, PiVector3 *v, float d);

static void pi_plane_by_3points(PiPlane *plane, PiVector3* p1, PiVector3* p2, PiVector3* p3);

/* ����ƽ������ */
static void pi_plane_copy(PiPlane* dst, PiPlane* src);

/* ƽ��ĵ�λ�� */
static void pi_plane_normalise(PiPlane *dst, PiPlane *src);

/* ƽ��͵�ĵ����С��0��ʾ����ƽ��ı��棬����0��ʾ����ƽ���ϣ�����0��ʾ����ƽ������ */
static float pi_plane_dot(PiPlane *plane, float x, float y, float z);

/* �����ƽ��ĶԳƵ�*/

static void pi_plane_symmetry_point(PiVector3* dst, PiVector3* src, PiPlane *plane);

/* �����ĶԳ�*/
static void pi_plane_symmetry_vector(PiVector3* dst, PiVector3* src, PiPlane *plane);

#include "plane.inl"

PI_END_DECLS

#endif /* __PI_PLANE_H__ */
