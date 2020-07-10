#ifndef INCLUDE_SK_ANIMATE_H
#define INCLUDE_SK_ANIMATE_H

#include <controller.h>
#include <pi_skeleton.h>
#include "physics_ragdoll.h"

/**
 * ��������������
 */

PI_BEGIN_DECLS

typedef enum {
	SKT_ANIM = 0,
	SKT_RAGDOLL
}SkeletonAnimType;

typedef enum
{
	AS_LOOP,	/* ѭ������ */
	AS_DEFAULT,	/* ����󣬻ָ���Ĭ������ */
	AS_KEEP,	/* ����󣬱������֡���� */
	AS_FIRST,   /* ����󣬱��ֵ�һ֡���� */
} AnimationState;

/**
 * ������������������
 */
PiController *PI_API pi_skanim_new();

/**
 * �ͷŹ�������������
 */
void PI_API pi_skanim_free(PiController *c);

/**
 * ���ù���
 */
PiBool PI_API pi_skanim_set_skeleton(PiController *c, PiSkeleton *sk);


/*
���û�����
*/
void PI_API pi_skamin_set_flag(PiController *c, PiBool cacheFlag);

/**
 * ��Ӷ���
 */
PiBool PI_API pi_skanim_add(PiController *c, const char *name, PiSkeleton *anim);

/**
 * �Ƴ�����
 */
PiBool PI_API pi_skanim_remove(PiController *c, const char *name);

/*
 * ���¶������ٶ�
*/
void PI_API pi_skanim_update_speed(PiController *c, float speed);

/**
 * ����Ҫ���ŵĶ���
 * anim_id, ���ŵĶ���id
 * transient_time, ����ʱ�䣨���뵭����ʱ�䣩��0.0f��ʾ�޹��ɣ�ֱ���л�����; ��λ����
 * speed, �����ٶȣ�����ʱ�䵥λ����
 * state: ��������
 * start_time, ���ĸ�ʱ�俪ʼ����, ��λ����
 * is_clear_queue, �Ƿ���Ҫ�����������
 */
PiBool PI_API pi_skanim_play(PiController *c, const char *name, float transient_time, float speed, AnimationState state, float start_time, PiBool is_clear_queue);

PiBool PI_API pi_skanim_add_anim_to_queue(PiController *c, const char *name, float transient_time, float speed, AnimationState state, float start_time, const char* defaultName);

/**
 * ֹͣ����
 */
PiBool PI_API pi_skanim_stop(PiController *c);

/**
 * ȡ��ĳ����ͷ�ľ���
 */
PiBool PI_API pi_skanim_get_bone_matrix(PiController *c, uint id, PiMatrix4 *dst);

/**
 * ȡ��ͷ���ֶ�Ӧ��id
 */
sint PI_API pi_skanim_get_bone_id(PiController *c, const char *bone_name);

PiBool PI_API pi_skanim_attach_ragdoll(PiController *c, Ragdoll* ragdoll);

void PI_API pi_skanim_detach_ragdoll(PiController* c);

void PI_API pi_skanim_change_status(PiController *c, SkeletonAnimType type);

SkeletonAnimType PI_API pi_skanim_get_status(PiController* c);

PI_END_DECLS

#endif /* INCLUDE_SK_ANIMATE_H */