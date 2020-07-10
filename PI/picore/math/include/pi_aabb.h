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

#ifndef __PI_AABB_H__
#define __PI_AABB_H__

/** 
 * AABB包围盒
 */

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_matrix4.h>

typedef struct
{
	PiVector3 minPt;
	PiVector3 maxPt;
}PiAABBBox;

PI_BEGIN_DECLS

/* 初始化，调用add_point前必须调用这个函数 */
static void pi_aabb_init(PiAABBBox *box);

/* 清理 */
static void pi_aabb_clear(PiAABBBox *box);

/* 拷贝 */
static void pi_aabb_copy(PiAABBBox *dst, PiAABBBox *src);

/* 包围盒扩充一个点 */
static void pi_aabb_add_point(PiAABBBox *box, PiVector3 *pt);

/* 包围盒合并 */
static void pi_aabb_merge(PiAABBBox *dst, PiAABBBox *src1, PiAABBBox *src2);

/* 取包围盒的中心点，将其填充至dst中 */
static void pi_aabb_get_center(PiAABBBox *box, PiVector3 *dst);

/* 将box用世界矩阵mat变换，得到变换后的AABB */
static void pi_aabb_transform(PiAABBBox *box, PiMatrix4 *mat);

/* 判断两个 aabb 是否有交叠 */
static PiBool pi_aabb_is_overlapped(PiAABBBox* b1, PiAABBBox* b2);

/* 判断 aabb2 是否在 aabb1 中 */
static PiBool pi_aabb_is_2in1(PiAABBBox* b1, PiAABBBox* b2);

/* 判断点pt是否在box内（如果点pt在box的6个面上，返回FALSE */
static PiBool pi_aabb_is_contian_point(PiAABBBox *box, PiVector3 *pt);

#include "aabb.inl"

PI_END_DECLS

#endif /* __PI_AABB_H__ */
