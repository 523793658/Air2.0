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

/* 平截头体的六个面 */
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

/* 平截头体 */
typedef struct PiFrustum
{
	PiMatrix4 viewMat;			/* 视图矩阵 */
	PiMatrix4 projMat;			/* 投影矩阵 */
	PiMatrix4 pvMat;			/* 视图矩阵和投影矩阵的乘积 */
	PiMatrix4 inversePVM;		/* pvMat 的逆矩阵，用于把摄像机空间坐标变换到世界坐标 */
	PiMatrix4 inverseProjMat;	/* 投影矩阵的逆矩阵 */
	PiMatrix4 inverseViewMat;	/* 视图矩阵的逆矩阵 */

	PiPlane plane[EFP_COUNT];	/* 世界空间中的 6 个面 */
} PiFrustum;

PI_BEGIN_DECLS

/* 设置平截头体的 6 个面 */
static void pi_frustum_set_planes(PiFrustum* frustum, PiPlane* planes);

/* 更新平截头体 */
static void pi_frustum_update(PiFrustum* frustum, PiMatrix4* viewMat, PiMatrix4* projMat);

/* 判断平截头体是否和 aabb 相交或包含 */
static PiBool pi_frustum_is_aabb_visible(PiFrustum* frustum, PiAABBBox* box);

#include "frustum.inl"

PI_END_DECLS

#endif /* __PI_FRUSTUM_H__ */
