#ifndef INCLUDE_RENDERBATCH_H
#define INCLUDE_RENDERBATCH_H

#include <pi_lib.h>
#include <pi_matrix4.h>

#include <renderlist.h>
#include <rendertarget.h>

/**
 * ��Ⱦ���Σ�
 *	�������ȼ���Խ��Խ����
 * 	��ȾĿ�� + �������ͼ��ͶӰ��+ ��Ⱦ�б� + ����״̬ + ����uniform
 */

enum { SIZEOF_RENDERBATCH = 1700 };

typedef struct 
{
	uint		mem[SIZEOF_RENDERBATCH];
} PiRenderBatch;

PI_BEGIN_DECLS

/* ��Ⱦ������ָ�������ȼ���ʼ����Խ��Խ���� */
void PI_API pi_renderbatch_init(PiRenderBatch *batch, void *pipeline, sint priority);

/* ����batch�Ƿ���� */
void PI_API pi_renderbatch_set_enable(PiRenderBatch *batch, PiBool is_enable);

/* �����Ⱦ���ε����ȼ� */
sint PI_API pi_renderbatch_get_priority(PiRenderBatch *batch);

/* ��Ⱦ������� */
void PI_API pi_renderbatch_clear(PiRenderBatch *batch, PiBool free_buf);

/* ��Ⱦ����������ȾĿ�� */
void PI_API pi_renderbatch_set_target(PiRenderBatch *batch, PiRenderTarget *target);

/* ��Ⱦ����������Ⱦ�б� */
void PI_API pi_renderbatch_set_list(PiRenderBatch *batch, PiRenderList* list);

/* ��Ⱦ����������ͼ���� */
void PI_API pi_renderbatch_set_view(PiRenderBatch *batch, PiMatrix4 *view);

/* ��Ⱦ��������ͶӰ���� */
void PI_API pi_renderbatch_set_proj(PiRenderBatch *batch, PiMatrix4 *proj);

/* ��Ⱦ��������״̬��ָ����ŵ�״̬������ */
void PI_API pi_renderbatch_set_state(PiRenderBatch *batch, uint index, RenderStateType key, uint32 value);

/* ��Ⱦ��������uniform��ָ����ŵı��������� */
void PI_API pi_renderbatch_set_uniform(PiRenderBatch *batch, uint index, const char *name, UniformType type, uint32 count, void *data);

/* ��Ⱦ���εĻ��Ʒ��� */
void PI_API pi_renderbatch_draw(PiRenderBatch *batch);

PI_END_DECLS

#endif /* INCLUDE_RENDERBATCH_H */