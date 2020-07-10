#ifndef INCLUDE_RENDERBATCH_H
#define INCLUDE_RENDERBATCH_H

#include <pi_lib.h>
#include <pi_matrix4.h>

#include <renderlist.h>
#include <rendertarget.h>

/**
 * 渲染批次：
 *	具有优先级，越大越优先
 * 	渲染目标 + 相机（视图，投影）+ 渲染列表 + 公用状态 + 公用uniform
 */

enum { SIZEOF_RENDERBATCH = 1700 };

typedef struct 
{
	uint		mem[SIZEOF_RENDERBATCH];
} PiRenderBatch;

PI_BEGIN_DECLS

/* 渲染批次用指定的优先级初始化，越大越优先 */
void PI_API pi_renderbatch_init(PiRenderBatch *batch, void *pipeline, sint priority);

/* 设置batch是否可用 */
void PI_API pi_renderbatch_set_enable(PiRenderBatch *batch, PiBool is_enable);

/* 获得渲染批次的优先级 */
sint PI_API pi_renderbatch_get_priority(PiRenderBatch *batch);

/* 渲染批次清除 */
void PI_API pi_renderbatch_clear(PiRenderBatch *batch, PiBool free_buf);

/* 渲染批次设置渲染目标 */
void PI_API pi_renderbatch_set_target(PiRenderBatch *batch, PiRenderTarget *target);

/* 渲染批次设置渲染列表 */
void PI_API pi_renderbatch_set_list(PiRenderBatch *batch, PiRenderList* list);

/* 渲染批次设置视图矩阵 */
void PI_API pi_renderbatch_set_view(PiRenderBatch *batch, PiMatrix4 *view);

/* 渲染批次设置投影矩阵 */
void PI_API pi_renderbatch_set_proj(PiRenderBatch *batch, PiMatrix4 *proj);

/* 渲染批次设置状态到指定编号的状态环境中 */
void PI_API pi_renderbatch_set_state(PiRenderBatch *batch, uint index, RenderStateType key, uint32 value);

/* 渲染批次设置uniform到指定编号的变量环境中 */
void PI_API pi_renderbatch_set_uniform(PiRenderBatch *batch, uint index, const char *name, UniformType type, uint32 count, void *data);

/* 渲染批次的绘制方法 */
void PI_API pi_renderbatch_draw(PiRenderBatch *batch);

PI_END_DECLS

#endif /* INCLUDE_RENDERBATCH_H */