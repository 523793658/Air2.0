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
 * �����ཻ�Լ���㷨
 */

typedef enum 
{
	EIS_OUTSIDE,	//����
	EIS_INTERSECTS,	//�ཻ
	EIS_INSIDE,	//����
}EIntersectState;

PI_BEGIN_DECLS

/**
 * ����2D�������Ƿ��ص�
 * ���裺t1, t2�������zֵ����ͬ
 * ���裺t1, t2˳��ʱ��Ҫ��ͬ
 * @param t1 ������1
 * @param t2 ������2
 * @return �Ƿ��ص�
 */
PiBool PI_API is_2d_tri_tri_overlapped(PiVector3 *t1, PiVector3 *t2);

/**
 * �ж�ָ�����߶κ�AABB�Ƿ��ཻ
 * @param aabb AABB��Χ��
 * @param line �߶�
 * @return �Ƿ��ཻ
 */
PiBool PI_API line2aabb(PiAABBBox* aabb, PiLineSegment* line);

/**
 * �ж�ָ���������AABB�Ƿ��ཻ
 * @param aabb AABB��Χ��
 * @param sphere ����
 * @return �Ƿ��ཻ
 */
PiBool PI_API sphere2aabb(PiAABBBox* aabb, PiSphere* sphere);

/**
 * �ж�ָ���������AABB�Ƿ��ཻ
 * @param aabb_p �����AABB��Χ��
 * @param aabb_i ���AABB��Χ��
 * @return �Ƿ��ཻ
 */
PiBool PI_API aabb2aabb(PiAABBBox* aabb_p, PiAABBBox* aabb_i);

/**
 * �ж�ָ������׵���AABB�Ƿ��ཻ
 * @param aabb AABB��Χ��
 * @param frustum ��׵��
 * @return �Ƿ��ཻ
 */
PiBool PI_API frustum2aabb(PiAABBBox* aabb, PiFrustum* frustum);

/**
 * �ж�ָ�����߶κ�OBB�Ƿ��ཻ
 * @param obb OBB��Χ��
 * @param line �߶�
 * @return �Ƿ��ཻ
 */
PiBool PI_API line2obb(PiOBBBox* obb, PiLineSegment* line);

/**
 * �ж�ָ���������OBB�Ƿ��ཻ
 * @param obb OBB��Χ��
 * @param sphere ����
 * @return �Ƿ��ཻ
 */
PiBool PI_API sphere2obb(PiOBBBox* obb, PiSphere* sphere);

/**
 * �ж�ָ����AABB��OBB�Ƿ��ཻ
 * @param obb OBB��Χ��
 * @param aabb AABB��Χ��
 * @return �Ƿ��ཻ
 */
PiBool PI_API aabb2obb(PiOBBBox* obb, PiAABBBox* aabb);

/**
 * �ж�ָ����2��OBB��OBB�Ƿ��ཻ
 * @param obb_p ������OBB��Χ��
 * @param obb_i OBB��Χ��
 * @return �Ƿ��ཻ
 */
PiBool PI_API obb2obb(PiOBBBox* obb_p, PiOBBBox* obb_i);

/**
 * �ж�ָ������׵���OBB�Ƿ��ཻ
 * @param obb OBB��Χ��
 * @param frustum ��׵��
 * @return �Ƿ��ཻ
 */
PiBool PI_API frustum2obb(PiOBBBox* obb, PiFrustum* frustum);

/**
 * �ж�ָ�����߶κ͵��Ƿ��ཻ
 * @param point �����ĵ�
 * @param line �߶�
 * @return �Ƿ��ཻ
 */
PiBool PI_API line2point(PiVector3* point, PiLineSegment* line);

/**
 * �ж�ָ��������͵��Ƿ��ཻ
 * @param point �����ĵ�
 * @param sphere ����
 * @return �Ƿ��ཻ
 */
PiBool PI_API sphere2point(PiVector3* point, PiSphere* sphere);

/**
 * �ж�ָ����AABB�͵��Ƿ��ཻ
 * @param point �����ĵ�
 * @param aabb AABB��Χ��
 * @return �Ƿ��ཻ
 */
PiBool PI_API aabb2point(PiVector3* point, PiAABBBox* aabb);

/**
 * �ж�ָ����OBB�͵��Ƿ��ཻ
 * @param point �����ĵ�
 * @param obb OBB��Χ��
 * @return �Ƿ��ཻ
 */
PiBool PI_API obb2point(PiVector3* point, PiOBBBox* obb);

/**
 * �ж�ָ������׵��͵��Ƿ��ཻ
 * @param point �����ĵ�
 * @param frustum ��׵��
 * @return �Ƿ��ཻ
 */
PiBool PI_API frustum2point(PiVector3* point, PiFrustum* frustum);

/**
 * �ж�ָ�����߶κ�aabb���ཻ��ϵ
 * @param aabb ������AABB��Χ��
 * @param line ��׵��
 * @return �ཻ��ϵ������ʱΪAABB����ȫ������
 */
EIntersectState PI_API line2aabb_i(PiAABBBox* aabb, PiLineSegment* line);

/**
 * �ж�ָ���������aabb���ཻ��ϵ
 * @param aabb ������AABB��Χ��
 * @param sphere ����
 * @return �ཻ��ϵ������ʱΪAABB����ȫ������
 */
EIntersectState PI_API sphere2aabb_i(PiAABBBox* aabb, PiSphere* sphere);

/**
 * �ж�ָ����2��AABB���ཻ��ϵ
 * @param aabb_p ������AABB��Χ��
 * @param aabb_i AABB��Χ��
 * @return �ཻ��ϵ������ʱΪaabb_p����ȫ������
 */
EIntersectState PI_API aabb2aabb_i(PiAABBBox* aabb_p, PiAABBBox* aabb_i);

/**
 * �ж�ָ����OBB��AABB���ཻ��ϵ
 * @param aabb ������AABB��Χ��
 * @param obb OBB��Χ��
 * @return �ཻ��ϵ������ʱΪAABB����ȫ������
 */
EIntersectState PI_API obb2aabb_i(PiAABBBox* aabb, PiOBBBox* obb);

/**
 * �ж�ָ������׵���aabb���ཻ��ϵ
 * @param aabb ������AABB��Χ��
 * @param frustum ��׵��
 * @return �ཻ��ϵ������ʱΪAABB����ȫ������
 */
EIntersectState PI_API frustum2aabb_i(PiAABBBox* aabb, 
							  PiFrustum* frustum);

PI_END_DECLS

#endif /* __PI_INTERSECTION_H__ */
