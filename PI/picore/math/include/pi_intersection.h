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

#ifndef __PI_INTERSECTION_H__
#define __PI_INTERSECTION_H__

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_aabb.h>
#include <pi_obb.h>
#include <pi_line_segment.h>
#include <pi_sphere.h>
#include <pi_frustum.h>

/** 
 * 各种相交性检测算法
 */

typedef enum 
{
	EIS_OUTSIDE,	//分离
	EIS_INTERSECTS,	//相交
	EIS_INSIDE,	//包含
}EIntersectState;

PI_BEGIN_DECLS

/**
 * 两个2D三角形是否重叠
 * 假设：t1, t2各顶点的z值都相同
 * 假设：t1, t2顺逆时针要相同
 * @param t1 三角型1
 * @param t2 三角形2
 * @return 是否重叠
 */
PiBool PI_API is_2d_tri_tri_overlapped(PiVector3 *t1, PiVector3 *t2);

/**
 * 判断指定的线段和AABB是否相交
 * @param aabb AABB包围盒
 * @param line 线段
 * @return 是否相交
 */
PiBool PI_API line2aabb(PiAABBBox* aabb, PiLineSegment* line);

/**
 * 判断指定的球体和AABB是否相交
 * @param aabb AABB包围盒
 * @param sphere 球体
 * @return 是否相交
 */
PiBool PI_API sphere2aabb(PiAABBBox* aabb, PiSphere* sphere);

/**
 * 判断指定的球体和AABB是否相交
 * @param aabb_p 被检测AABB包围盒
 * @param aabb_i 检测AABB包围盒
 * @return 是否相交
 */
PiBool PI_API aabb2aabb(PiAABBBox* aabb_p, PiAABBBox* aabb_i);

/**
 * 判断指定的视椎体和AABB是否相交
 * @param aabb AABB包围盒
 * @param frustum 视椎体
 * @return 是否相交
 */
PiBool PI_API frustum2aabb(PiAABBBox* aabb, PiFrustum* frustum);

/**
 * 判断指定的线段和OBB是否相交
 * @param obb OBB包围盒
 * @param line 线段
 * @return 是否相交
 */
PiBool PI_API line2obb(PiOBBBox* obb, PiLineSegment* line);

/**
 * 判断指定的球体和OBB是否相交
 * @param obb OBB包围盒
 * @param sphere 球体
 * @return 是否相交
 */
PiBool PI_API sphere2obb(PiOBBBox* obb, PiSphere* sphere);

/**
 * 判断指定的AABB和OBB是否相交
 * @param obb OBB包围盒
 * @param aabb AABB包围盒
 * @return 是否相交
 */
PiBool PI_API aabb2obb(PiOBBBox* obb, PiAABBBox* aabb);

/**
 * 判断指定的2个OBB和OBB是否相交
 * @param obb_p 被检测的OBB包围盒
 * @param obb_i OBB包围盒
 * @return 是否相交
 */
PiBool PI_API obb2obb(PiOBBBox* obb_p, PiOBBBox* obb_i);

/**
 * 判断指定的视椎体和OBB是否相交
 * @param obb OBB包围盒
 * @param frustum 视椎体
 * @return 是否相交
 */
PiBool PI_API frustum2obb(PiOBBBox* obb, PiFrustum* frustum);

/**
 * 判断指定的线段和点是否相交
 * @param point 被检测的点
 * @param line 线段
 * @return 是否相交
 */
PiBool PI_API line2point(PiVector3* point, PiLineSegment* line);

/**
 * 判断指定的球体和点是否相交
 * @param point 被检测的点
 * @param sphere 球体
 * @return 是否相交
 */
PiBool PI_API sphere2point(PiVector3* point, PiSphere* sphere);

/**
 * 判断指定的AABB和点是否相交
 * @param point 被检测的点
 * @param aabb AABB包围盒
 * @return 是否相交
 */
PiBool PI_API aabb2point(PiVector3* point, PiAABBBox* aabb);

/**
 * 判断指定的OBB和点是否相交
 * @param point 被检测的点
 * @param obb OBB包围盒
 * @return 是否相交
 */
PiBool PI_API obb2point(PiVector3* point, PiOBBBox* obb);

/**
 * 判断指定的视椎体和点是否相交
 * @param point 被检测的点
 * @param frustum 视椎体
 * @return 是否相交
 */
PiBool PI_API frustum2point(PiVector3* point, PiFrustum* frustum);

/**
 * 判断指定的线段和aabb的相交关系
 * @param aabb 被检测的AABB包围盒
 * @param line 视椎体
 * @return 相交关系（包含时为AABB被完全包含）
 */
EIntersectState PI_API line2aabb_i(PiAABBBox* aabb, PiLineSegment* line);

/**
 * 判断指定的球体和aabb的相交关系
 * @param aabb 被检测的AABB包围盒
 * @param sphere 球体
 * @return 相交关系（包含时为AABB被完全包含）
 */
EIntersectState PI_API sphere2aabb_i(PiAABBBox* aabb, PiSphere* sphere);

/**
 * 判断指定的2个AABB的相交关系
 * @param aabb_p 被检测的AABB包围盒
 * @param aabb_i AABB包围盒
 * @return 相交关系（包含时为aabb_p被完全包含）
 */
EIntersectState PI_API aabb2aabb_i(PiAABBBox* aabb_p, PiAABBBox* aabb_i);

/**
 * 判断指定的OBB和AABB的相交关系
 * @param aabb 被检测的AABB包围盒
 * @param obb OBB包围盒
 * @return 相交关系（包含时为AABB被完全包含）
 */
EIntersectState PI_API obb2aabb_i(PiAABBBox* aabb, PiOBBBox* obb);

/**
 * 判断指定的视椎体和aabb的相交关系
 * @param aabb 被检测的AABB包围盒
 * @param frustum 视椎体
 * @return 相交关系（包含时为AABB被完全包含）
 */
EIntersectState PI_API frustum2aabb_i(PiAABBBox* aabb, 
							  PiFrustum* frustum);

PI_END_DECLS

#endif /* __PI_INTERSECTION_H__ */
