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

/** 平面方程是：
 * v.x * x + v.y * y + v.z * z + d = 0 
 * 将v单位化，v就是平面的法线向量
 * v单位化时的d就是坐标原点到平面的距离
 */
typedef struct 
{
	float d;
	PiVector3 v;
}PiPlane;

PI_BEGIN_DECLS

static void pi_plane_set(PiPlane *plane, PiVector3 *v, float d);

static void pi_plane_by_3points(PiPlane *plane, PiVector3* p1, PiVector3* p2, PiVector3* p3);

/* 拷贝平面数据 */
static void pi_plane_copy(PiPlane* dst, PiPlane* src);

/* 平面的单位化 */
static void pi_plane_normalise(PiPlane *dst, PiPlane *src);

/* 平面和点的点积，小于0表示点在平面的背面，等于0表示点在平面上，大于0表示点在平面正面 */
static float pi_plane_dot(PiPlane *plane, float x, float y, float z);

/* 点关于平面的对称点*/

static void pi_plane_symmetry_point(PiVector3* dst, PiVector3* src, PiPlane *plane);

/* 向量的对称*/
static void pi_plane_symmetry_vector(PiVector3* dst, PiVector3* src, PiPlane *plane);

#include "plane.inl"

PI_END_DECLS

#endif /* __PI_PLANE_H__ */
