#ifndef INCLUDE_RENDERLIST_H
#define INCLUDE_RENDERLIST_H

#include <pi_lib.h>
#include <entity.h>

/**
 * ��Ⱦ����
 */
typedef struct
{
	uint block;			/* ÿ��Entryβ�����ֽ��� */
	PiBool dirty;		/* �Ƿ��Ѿ���������� */

	PiCompareFunc sort_func;
	
	void *instance;	/* ��Ϊÿ��Entry��β������Ҫblock�ֽڣ�������Ҫһ��ʵ������push���б�β�� */
	PiDvector entries;	/* Ԫ��������RenderListEntry */
}PiRenderList;

PI_BEGIN_DECLS

/* ��ʼ����Ⱦ���У�block��add��reset�����ݶ�Ӧ���ֽ��� */
void PI_API pi_renderlist_init(PiRenderList *list, uint block);

/* ������Ⱦ���� */
void PI_API pi_renderlist_clear(PiRenderList *list);

/* �����ȾԪ�أ�entity + ״̬���� + uniform���� */
/* ��Ⱦ�б�ͬһʱ��ֻ��һ��batchʹ�ã�batchҪ���б�ÿ��Ԫ�ص�state_env, shader_envƥ�� */
void PI_API pi_renderlist_add(PiRenderList *list, PiEntity *entity, void *data, uint state_env, uint shader_env);

/* ɾ����Ӧ��entity */
void PI_API pi_renderlist_delete(PiRenderList *list, PiEntity *entity);

/* �����࣬�������� */
void PI_API pi_renderlist_set_dirty(PiRenderList *list);

/* ���� */
void PI_API pi_renderlist_sort(PiRenderList *list);

/* ָ�������� */
void PI_API pi_renderlist_set_sort_func(PiRenderList *list, PiCompareFunc func);

PI_END_DECLS

#endif /* INCLUDE_RENDERLIST_H */