#ifndef INCLUDE_RENDERLIST_H
#define INCLUDE_RENDERLIST_H

#include <pi_lib.h>
#include <entity.h>

/**
 * 渲染队列
 */
typedef struct
{
	uint block;			/* 每个Entry尾部的字节数 */
	PiBool dirty;		/* 是否已经排序的脏标记 */

	PiCompareFunc sort_func;
	
	void *instance;	/* 因为每个Entry的尾部都需要block字节，所以需要一个实例用于push到列表尾端 */
	PiDvector entries;	/* 元素类型是RenderListEntry */
}PiRenderList;

PI_BEGIN_DECLS

/* 初始化渲染队列，block是add和reset的数据对应的字节数 */
void PI_API pi_renderlist_init(PiRenderList *list, uint block);

/* 清理渲染队列 */
void PI_API pi_renderlist_clear(PiRenderList *list);

/* 添加渲染元素：entity + 状态环境 + uniform环境 */
/* 渲染列表同一时刻只被一个batch使用，batch要和列表每个元素的state_env, shader_env匹配 */
void PI_API pi_renderlist_add(PiRenderList *list, PiEntity *entity, void *data, uint state_env, uint shader_env);

/* 删掉对应的entity */
void PI_API pi_renderlist_delete(PiRenderList *list, PiEntity *entity);

/* 设置脏，用于排序 */
void PI_API pi_renderlist_set_dirty(PiRenderList *list);

/* 排序 */
void PI_API pi_renderlist_sort(PiRenderList *list);

/* 指定排序函数 */
void PI_API pi_renderlist_set_sort_func(PiRenderList *list, PiCompareFunc func);

PI_END_DECLS

#endif /* INCLUDE_RENDERLIST_H */