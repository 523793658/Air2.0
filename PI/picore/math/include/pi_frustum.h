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

#ifndef __PI_FRUSTUM_H__
#define __PI_FRUSTUM_H__

#include <pi_lib.h>
#include <pi_matrix4.h>
#include <pi_vector3.h>
#include <pi_plane.h>
#include <pi_aabb.h>

/* ƽ��ͷ��������� */
typedef enum
{
	EFP_LEFT = 0,
	EFP_RIGHT,
	EFP_TOP,
	EFP_BOTTOM,
	EFP_NEAR,
	EFP_FAR,
	EFP_COUNT,
} EFrustumPlane;

/* ƽ��ͷ�� */
typedef struct PiFrustum
{
	PiMatrix4 viewMat;			/* ��ͼ���� */
	PiMatrix4 projMat;			/* ͶӰ���� */
	PiMatrix4 pvMat;			/* ��ͼ�����ͶӰ����ĳ˻� */
	PiMatrix4 inversePVM;		/* pvMat ����������ڰ�������ռ�����任���������� */
	PiMatrix4 inverseProjMat;	/* ͶӰ���������� */
	PiMatrix4 inverseViewMat;	/* ��ͼ���������� */

	PiPlane plane[EFP_COUNT];	/* ����ռ��е� 6 ���� */
} PiFrustum;

PI_BEGIN_DECLS

/* ����ƽ��ͷ��� 6 ���� */
static void pi_frustum_set_planes(PiFrustum* frustum, PiPlane* planes);

/* ����ƽ��ͷ�� */
static void pi_frustum_update(PiFrustum* frustum, PiMatrix4* viewMat, PiMatrix4* projMat);

/* �ж�ƽ��ͷ���Ƿ�� aabb �ཻ����� */
static PiBool pi_frustum_is_aabb_visible(PiFrustum* frustum, PiAABBBox* box);

#include "frustum.inl"

PI_END_DECLS

#endif /* __PI_FRUSTUM_H__ */
