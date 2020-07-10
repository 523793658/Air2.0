#ifndef INCLUDE_RENDERTARGET_H
#define INCLUDE_RENDERTARGET_H

#include <pi_lib.h>
#include <renderview.h>

typedef enum
{
	TT_WIN,
	TT_MRT,
}TargetType;

typedef enum 
{
	ATT_COLOR0,
	ATT_COLOR1,
	ATT_COLOR2,
	ATT_COLOR3,
	ATT_COLOR4,
	ATT_COLOR5,
	ATT_COLOR6,
	ATT_COLOR7,
	ATT_DEPTHSTENCIL,
	ATT_NUM
}TargetAttachment;

typedef enum
{
	TBM_COLOR   = 1UL << 0,
	TBM_DEPTH   = 1UL << 1,
	TBM_STENCIL = 1UL << 2,
	TBM_COLOR_DEPTH = (TBM_COLOR | TBM_DEPTH),
	TBM_ALL = (TBM_COLOR | TBM_DEPTH | TBM_STENCIL),
}TargetBufferMask;

typedef enum 
{
	FILTER_NEAREST,
	FILTER_LINEAR,
}TargetFilterType;

typedef struct  
{
	/* viewport */
	uint32 left;
	uint32 bottom;
	uint32 width;
	uint32 height;
	
	TargetType type;
	PiBool is_bind;
	PiBool is_update;

	PiRenderView *views[ATT_NUM];
		
	void *impl;
}PiRenderTarget;

PI_BEGIN_DECLS

/* ��ȾĿ���ʼ�� */
PiRenderTarget* PI_API pi_rendertarget_new(TargetType type, PiBool is_create_handle);

/* ��ʼ���Դ���Դ: rt������ͨ��new���������� */
PiBool PI_API pi_rendertarget_init(PiRenderTarget *target);

/* ��ȾĿ������ */
PiBool PI_API pi_rendertarget_free(PiRenderTarget* target);

/* ȡ��ȾĿ��Ŀ� */
uint PI_API pi_rendertarget_get_width(PiRenderTarget *target);

/* ȡ��ȾĿ����ӿڵĸ� */
uint PI_API pi_rendertarget_get_height(PiRenderTarget *target);

/* �����ӿ����� */
PiBool PI_API pi_rendertarget_set_viewport(PiRenderTarget *target, uint32 left, uint32 bottom, uint32 width, uint32 height);

/* att�ϸ�����ͼ */
PiBool PI_API pi_rendertarget_attach(PiRenderTarget *target, TargetAttachment attachment, PiRenderView *view);

/* ж��att�ϵ���ͼ */
PiBool PI_API pi_rendertarget_detach(PiRenderTarget *target, TargetAttachment attachment);

/* ��ʼʹ��Ŀ�� */
PiBool PI_API pi_rendertarget_bind(PiRenderTarget *target);

/* ����ʹ��Ŀ�� */
PiBool PI_API pi_rendertarget_unbind(PiRenderTarget *target);

/** 
 * ������ȾĿ�꣬��Դtarget���ӿڷ�Χ������Ŀ��target���ӿڷ�Χ 
 * ע�⣺
 *    ��mask��Depth��Stencilʱ��filter������Nearest
 *    ��mask��depth��stencilʱ��depth��stencil��ʽ����ƥ��
 *    ��filter��linearʱ��Դ���ܺ���������
 *    ��mask��Colorʱ��src������򸡵����ݣ�dst���뺬����򸡵�����
 *    ��mask��Colorʱ��src���޷������Σ�dst���뺬�޷�������
 *    ��mask��Colorʱ��src���з������Σ�dst���뺬�з�������
 *    ��src��dstΪͬһ��ָ�룬����ָ��ͬһ����ͼ����ʱ������Ŀ����ӿڲ�����ͬ�����������޶���
 */
PiBool PI_API pi_rendertarget_copy(PiRenderTarget *dst, PiRenderTarget *src, TargetBufferMask mask, TargetFilterType filter);

PI_END_DECLS

#endif /* INCLUDE_RENDERTARGET_H */