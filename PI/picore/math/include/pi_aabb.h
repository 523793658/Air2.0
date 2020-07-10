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
 * AABB��Χ��
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

/* ��ʼ��������add_pointǰ�������������� */
static void pi_aabb_init(PiAABBBox *box);

/* ���� */
static void pi_aabb_clear(PiAABBBox *box);

/* ���� */
static void pi_aabb_copy(PiAABBBox *dst, PiAABBBox *src);

/* ��Χ������һ���� */
static void pi_aabb_add_point(PiAABBBox *box, PiVector3 *pt);

/* ��Χ�кϲ� */
static void pi_aabb_merge(PiAABBBox *dst, PiAABBBox *src1, PiAABBBox *src2);

/* ȡ��Χ�е����ĵ㣬���������dst�� */
static void pi_aabb_get_center(PiAABBBox *box, PiVector3 *dst);

/* ��box���������mat�任���õ��任���AABB */
static void pi_aabb_transform(PiAABBBox *box, PiMatrix4 *mat);

/* �ж����� aabb �Ƿ��н��� */
static PiBool pi_aabb_is_overlapped(PiAABBBox* b1, PiAABBBox* b2);

/* �ж� aabb2 �Ƿ��� aabb1 �� */
static PiBool pi_aabb_is_2in1(PiAABBBox* b1, PiAABBBox* b2);

/* �жϵ�pt�Ƿ���box�ڣ������pt��box��6�����ϣ�����FALSE */
static PiBool pi_aabb_is_contian_point(PiAABBBox *box, PiVector3 *pt);

#include "aabb.inl"

PI_END_DECLS

#endif /* __PI_AABB_H__ */
