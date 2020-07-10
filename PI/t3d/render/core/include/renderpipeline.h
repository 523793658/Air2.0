#ifndef INCLUDE_RENDERPIPELINE_H
#define INCLUDE_RENDERPIPELINE_H

#include <pi_lib.h>
#include <renderbatch.h>

enum { SIZEOF_RENDERPIPELINE = 8 + SIZEOF_DVECTOR + sizeof(FullState) / sizeof(uint)};

/**
 * ��Ⱦ����
 */
typedef struct 
 {
	 uint		mem[SIZEOF_RENDERPIPELINE];
 } PiRenderPipeline;

PI_BEGIN_DECLS

void PI_API pi_renderpipeline_init(PiRenderPipeline *pipeline);

void PI_API pi_renderpipeline_clear(PiRenderPipeline *pipeline);

/**
 * ���ù��ߵ�ȫ����Ⱦ״̬
 * ����ñ���������batch����֮ǰȫ�ֳ�ʼ���ã�������������
 */
void PI_API pi_renderpipeline_set_state(PiRenderPipeline *pipeline, RenderStateType key, uint32 value);

/* ��ˮ����Ⱦ */
void PI_API pi_renderpipeline_draw(PiRenderPipeline *pipeline);

/* ���ȱʡ��Ⱦ״̬���ϴ����õ����״̬�б� */
FullState* PI_API pi_renderpipeline_get_full_state(PiRenderPipeline *pipeline);

/* �����û����� */
void PI_API pi_renderpipeline_set_userdata(PiRenderPipeline *pipeline, void* data);

/* ��ȡ�û����� */
void* PI_API pi_renderpipeline_get_userdata(PiRenderPipeline *pipeline);

/* ��ȡ��ǰ��֡�������ڲ�֮ͬ֡���uniformȫ�ֱȽ� */
uint PI_API pi_renderpipeline_get_frame(PiRenderPipeline *pipelline);

PI_END_DECLS

#endif /* INCLUDE_RENDERPIPELINE_H */