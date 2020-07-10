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

/* 骨骼的骨头变换数据 */
typedef struct
{
	uint num;					/* 数量等于骨骼骨头的数量 */
	PiBool *is_computed;		/* 对应的骨头变换数据是否计算过 */
	TransformData *transforms;	/* 变换数据数组 */
} BoneTransform;

/* 缓冲数据 */
typedef struct
{
	uint32 elemNum;				/* 元素的个数 */
	uint32 elemSize;			/* 单个元素的字节数 */
	void *data;					/* 元素的首地址 */
} BufferData;


/* 骨头数据 */
typedef struct
{
	wchar *name;				/* 骨骼的名字 */
	int16 parentID;				/* 父骨头的ID，根骨头的父骨头就是其自身 */
	TransformData localData;	/* 相对于父骨头空间的变换数据 */
} Bone;

/* 关键帧 */
typedef struct
{
	uint32 startTime;			/* 该帧的开始时间，单位：毫秒 */
	TransformData transform;	/* 该帧的变换数据 */
} AnimKeyFrame;

/* 动画轨迹 */
typedef struct
{
	int16 boneId;
	BufferData keyframes;		/* 关键帧数据，元素类型为AnimKeyFrame */
} AnimTrack;

/* 动画 */
typedef struct
{
	wchar *name;
	BufferData tracks;			/* 动画轨迹，元素类型为AnimTrack */
	uint32 playTime;			/* 播放时间，单位：毫秒 */
	BufferData matrix_cache;
} Animation;

/* 骨骼动画 */
typedef struct
{
	BufferData boneData;		/* 骨头数据，元素类型为Bone */
	BufferData animData;		/* 动画数据，元素类型为Animation */

	PiBool *isCacheCompute;		/* 缓冲数据是否已经计算 */
	TransformData *cacheTrans;	/* 缓冲的变换数据，用于骨骼动画的计算，元素个数：boneData.elemNum */
	TransformData *invBindPose;	/* 骨头的初始姿态的逆元素，仅于加载时计算，用于动画时(元素个数有boneData.elemNum) */
} PiSkeleton;

PI_BEGIN_DECLS

/* 从二进制数据data创建骨骼数据 */
PiSkeleton *PI_API pi_skeleton_new(byte *data, uint32 size);

/* 释放 */
void PI_API pi_skeleton_free(PiSkeleton *sk);

/* 从二进制数据data里面解析骨骼数据 */
PiBool PI_API pi_skeleton_load(PiSkeleton *sk, byte *data, uint32 size);

/* 释放骨骼数据 */
void PI_API pi_skeleton_close(PiSkeleton *sk);

/* 得到动画中指定ID骨头的矩阵 */
void PI_API pi_skeleton_get_animmat(PiSkeleton *sk, PiMatrix4 *mat, uint32 boneId, PiMatrix4 *mats, uint32 numMat);

/* 取骨骼对应骨头的数目 */
uint32 PI_API pi_skeleton_get_bonenum(PiSkeleton *sk);

/* 通过名字获取骨骼的ID */
uint32 PI_API pi_skeleton_get_bone_id(PiSkeleton *sk, wchar *name);

void PI_API pi_skeleton_init_cache_data(PiSkeleton* anim, float delta_);

/*获取骨骼动画的是时长*/
uint PI_API pi_skeleton_get_animation_time(PiSkeleton *anim);

uint PI_API pi_skanim_get_cache_num(PiSkeleton* anim);

float* PI_API pi_skeleton_get_cache_matrix(PiSkeleton *anim, uint frame_index);

float* PI_API pi_skeleton_set_cache_matrix(PiSkeleton *anim, uint frame_index, float* data);

/* --------------------------- 不需要调用的函数 --------------------------- */

/* 通过名字获取动画ID */
uint32 PI_API pi_skeleton_get_anim_id(PiSkeleton *sk, wchar *name);

/* 通过ID获得动画持续时间 */
uint32 PI_API pi_skeleton_get_anim_time(PiSkeleton *sk, uint32 animID);

/* 将另一个骨骼的动画数据融合进前一个 */
void PI_API pi_skeleton_merge_anim(PiSkeleton *dstSk, PiSkeleton *srcSk);

void PI_API pi_skeleton_replace_inv_pose(PiSkeleton *dstSk, PiSkeleton *srcSk);

/* 将动画类型应用于骨骼，得到该骨骼作用于网格的矩阵 */
void PI_API pi_skeleton_anim_apply(PiMatrix4 *mat, uint32 numMat, PiSkeleton *sk, uint32 time, float weight,
                                   PiSkeleton *blendSk, uint32 blendTime);

/* 得到骨头boneID对应的初始的姿态矩阵 */
void PI_API pi_skeleton_get_initposemat(PiSkeleton *sk, PiMatrix4 *dst, uint32 boneID);

PI_END_DECLS

#endif /* __PI_SKELETON_H__ */