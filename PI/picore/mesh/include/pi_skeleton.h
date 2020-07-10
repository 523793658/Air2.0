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

#ifndef __PI_SKELETON_H__
#define __PI_SKELETON_H__

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_quaternion.h>
#include "pi_transform.h"
#include <pi_matrix4.h>

#define frame_delta_time 33.3333333f

/* �����Ĺ�ͷ�任���� */
typedef struct
{
	uint num;					/* �������ڹ�����ͷ������ */
	PiBool *is_computed;		/* ��Ӧ�Ĺ�ͷ�任�����Ƿ����� */
	TransformData *transforms;	/* �任�������� */
} BoneTransform;

/* �������� */
typedef struct
{
	uint32 elemNum;				/* Ԫ�صĸ��� */
	uint32 elemSize;			/* ����Ԫ�ص��ֽ��� */
	void *data;					/* Ԫ�ص��׵�ַ */
} BufferData;


/* ��ͷ���� */
typedef struct
{
	wchar *name;				/* ���������� */
	int16 parentID;				/* ����ͷ��ID������ͷ�ĸ���ͷ���������� */
	TransformData localData;	/* ����ڸ���ͷ�ռ�ı任���� */
} Bone;

/* �ؼ�֡ */
typedef struct
{
	uint32 startTime;			/* ��֡�Ŀ�ʼʱ�䣬��λ������ */
	TransformData transform;	/* ��֡�ı任���� */
} AnimKeyFrame;

/* �����켣 */
typedef struct
{
	int16 boneId;
	BufferData keyframes;		/* �ؼ�֡���ݣ�Ԫ������ΪAnimKeyFrame */
} AnimTrack;

/* ���� */
typedef struct
{
	wchar *name;
	BufferData tracks;			/* �����켣��Ԫ������ΪAnimTrack */
	uint32 playTime;			/* ����ʱ�䣬��λ������ */
	BufferData matrix_cache;
} Animation;

/* �������� */
typedef struct
{
	BufferData boneData;		/* ��ͷ���ݣ�Ԫ������ΪBone */
	BufferData animData;		/* �������ݣ�Ԫ������ΪAnimation */

	PiBool *isCacheCompute;		/* ���������Ƿ��Ѿ����� */
	TransformData *cacheTrans;	/* ����ı任���ݣ����ڹ��������ļ��㣬Ԫ�ظ�����boneData.elemNum */
	TransformData *invBindPose;	/* ��ͷ�ĳ�ʼ��̬����Ԫ�أ����ڼ���ʱ���㣬���ڶ���ʱ(Ԫ�ظ�����boneData.elemNum) */
} PiSkeleton;

PI_BEGIN_DECLS

/* �Ӷ���������data������������ */
PiSkeleton *PI_API pi_skeleton_new(byte *data, uint32 size);

/* �ͷ� */
void PI_API pi_skeleton_free(PiSkeleton *sk);

/* �Ӷ���������data��������������� */
PiBool PI_API pi_skeleton_load(PiSkeleton *sk, byte *data, uint32 size);

/* �ͷŹ������� */
void PI_API pi_skeleton_close(PiSkeleton *sk);

/* �õ�������ָ��ID��ͷ�ľ��� */
void PI_API pi_skeleton_get_animmat(PiSkeleton *sk, PiMatrix4 *mat, uint32 boneId, PiMatrix4 *mats, uint32 numMat);

/* ȡ������Ӧ��ͷ����Ŀ */
uint32 PI_API pi_skeleton_get_bonenum(PiSkeleton *sk);

/* ͨ�����ֻ�ȡ������ID */
uint32 PI_API pi_skeleton_get_bone_id(PiSkeleton *sk, wchar *name);

void PI_API pi_skeleton_init_cache_data(PiSkeleton* anim, float delta_);

/*��ȡ������������ʱ��*/
uint PI_API pi_skeleton_get_animation_time(PiSkeleton *anim);

uint PI_API pi_skanim_get_cache_num(PiSkeleton* anim);

float* PI_API pi_skeleton_get_cache_matrix(PiSkeleton *anim, uint frame_index);

float* PI_API pi_skeleton_set_cache_matrix(PiSkeleton *anim, uint frame_index, float* data);

/* --------------------------- ����Ҫ���õĺ��� --------------------------- */

/* ͨ�����ֻ�ȡ����ID */
uint32 PI_API pi_skeleton_get_anim_id(PiSkeleton *sk, wchar *name);

/* ͨ��ID��ö�������ʱ�� */
uint32 PI_API pi_skeleton_get_anim_time(PiSkeleton *sk, uint32 animID);

/* ����һ�������Ķ��������ںϽ�ǰһ�� */
void PI_API pi_skeleton_merge_anim(PiSkeleton *dstSk, PiSkeleton *srcSk);

void PI_API pi_skeleton_replace_inv_pose(PiSkeleton *dstSk, PiSkeleton *srcSk);

/* ����������Ӧ���ڹ������õ��ù�������������ľ��� */
void PI_API pi_skeleton_anim_apply(PiMatrix4 *mat, uint32 numMat, PiSkeleton *sk, uint32 time, float weight,
                                   PiSkeleton *blendSk, uint32 blendTime);

/* �õ���ͷboneID��Ӧ�ĳ�ʼ����̬���� */
void PI_API pi_skeleton_get_initposemat(PiSkeleton *sk, PiMatrix4 *dst, uint32 boneID);

PI_END_DECLS

#endif /* __PI_SKELETON_H__ */