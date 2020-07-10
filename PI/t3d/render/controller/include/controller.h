#ifndef INCLUDE_CONTROLLER_H
#define INCLUDE_CONTROLLER_H

#include <pi_lib.h>
#include <entity.h>

/**
 * ������������ÿ֡������̵��ƶ��͸���
 */

typedef enum
{
	CT_SKANIM,		/* �������������� */
	CT_VERTEX_ANIM,	/* ���㶯�������� */
	CT_SKBINDING,	/* �����󶨿����� */
	CT_TAIL,		/* ��β��������� */
	CT_BILLBOARD,	/* ����������   */
	CT_CHAIN,		/* ������������� */
	CT_VEGETATION_ANIM,		/* ֲ������������ */
	CT_VANISH,               /*������ɢ������*/
	CT_SWING,                /*����Ʈ������*/
	CT_SHINY,                /*��Ȧ��Ч������*/
	CT_COUNT,
	CT_USER = 5000		/* �Զ������� */
} ControllerType;

/* �������� */
typedef enum
{
	CAT_MESH,		/* ���� */
	CAT_ENTITY,		/* ʵ�� */
	CAT_SPATIAL,	/* �ռ���Ϣ */
	CAT_BINDING_LIST, /* ���б� */
	CAT_PARTICLE_CLUSTER,
} ControllerApplyType;

typedef struct PiController PiController;

typedef PiBool (*ControllerUpdateFunc)(struct PiController *c, float tpf);

typedef PiBool (*ControllerApplyFunc)(struct PiController *c, ControllerApplyType type, void *obj);

/**
 * ��Ⱦ��
 */
typedef struct PiController
{
	ControllerType type;	/* ���������� */
	PiBool is_alive;
	void* impl;				/* ����Ŀ�����ʵ�� */
	PiBool need_update;
	float delta_time;

	ControllerUpdateFunc update_func;	/* ���� */
	ControllerApplyFunc apply_func;		/* ���õ������������ */
} PiController;


PI_BEGIN_DECLS
void PI_API pi_controller_free(PiController *c);

PiController* PI_API pi_controller_new(ControllerType type, ControllerApplyFunc apply, ControllerUpdateFunc update, void *impl);

PiBool PI_API pi_controller_update(PiController *c, float tpf);

PiBool PI_API pi_controller_apply(PiController *c, ControllerApplyType type, void *obj);
PI_END_DECLS
#endif /* INCLUDE_CONTROLLER_H */