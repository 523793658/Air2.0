#ifndef INCLUDE_SK_ANIMATE_H
#define INCLUDE_SK_ANIMATE_H

#include <controller.h>
#include <pi_skeleton.h>
#include "physics_ragdoll.h"

/**
 * 骨骼动画控制器
 */

PI_BEGIN_DECLS

typedef enum {
	SKT_ANIM = 0,
	SKT_RAGDOLL
}SkeletonAnimType;

typedef enum
{
	AS_LOOP,	/* 循环播放 */
	AS_DEFAULT,	/* 播完后，恢复到默认类型 */
	AS_KEEP,	/* 播完后，保持最后帧类型 */
	AS_FIRST,   /* 播完后，保持第一帧类型 */
} AnimationState;

/**
 * 创建骨骼动画控制器
 */
PiController *PI_API pi_skanim_new();

/**
 * 释放骨骼动画控制器
 */
void PI_API pi_skanim_free(PiController *c);

/**
 * 设置骨骼
 */
PiBool PI_API pi_skanim_set_skeleton(PiController *c, PiSkeleton *sk);


/*
设置缓冲标记
*/
void PI_API pi_skamin_set_flag(PiController *c, PiBool cacheFlag);

/**
 * 添加动画
 */
PiBool PI_API pi_skanim_add(PiController *c, const char *name, PiSkeleton *anim);

/**
 * 移除动画
 */
PiBool PI_API pi_skanim_remove(PiController *c, const char *name);

/*
 * 更新动作的速度
*/
void PI_API pi_skanim_update_speed(PiController *c, float speed);

/**
 * 设置要播放的动画
 * anim_id, 播放的动画id
 * transient_time, 过渡时间（淡入淡出的时间），0.0f表示无过渡，直接切换动画; 单位：秒
 * speed, 播放速度，其中时间单位：秒
 * state: 动画类型
 * start_time, 从哪个时间开始播放, 单位：秒
 * is_clear_queue, 是否需要清除动画队列
 */
PiBool PI_API pi_skanim_play(PiController *c, const char *name, float transient_time, float speed, AnimationState state, float start_time, PiBool is_clear_queue);

PiBool PI_API pi_skanim_add_anim_to_queue(PiController *c, const char *name, float transient_time, float speed, AnimationState state, float start_time, const char* defaultName);

/**
 * 停止播放
 */
PiBool PI_API pi_skanim_stop(PiController *c);

/**
 * 取到某根骨头的矩阵
 */
PiBool PI_API pi_skanim_get_bone_matrix(PiController *c, uint id, PiMatrix4 *dst);

/**
 * 取骨头名字对应的id
 */
sint PI_API pi_skanim_get_bone_id(PiController *c, const char *bone_name);

PiBool PI_API pi_skanim_attach_ragdoll(PiController *c, Ragdoll* ragdoll);

void PI_API pi_skanim_detach_ragdoll(PiController* c);

void PI_API pi_skanim_change_status(PiController *c, SkeletonAnimType type);

SkeletonAnimType PI_API pi_skanim_get_status(PiController* c);

PI_END_DECLS

#endif /* INCLUDE_SK_ANIMATE_H */