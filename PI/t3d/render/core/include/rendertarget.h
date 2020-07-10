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

/* 渲染目标初始化 */
PiRenderTarget* PI_API pi_rendertarget_new(TargetType type, PiBool is_create_handle);

/* 初始化显存资源: rt必须是通过new创建出来的 */
PiBool PI_API pi_rendertarget_init(PiRenderTarget *target);

/* 渲染目标清理 */
PiBool PI_API pi_rendertarget_free(PiRenderTarget* target);

/* 取渲染目标的宽 */
uint PI_API pi_rendertarget_get_width(PiRenderTarget *target);

/* 取渲染目标的视口的高 */
uint PI_API pi_rendertarget_get_height(PiRenderTarget *target);

/* 设置视口区域 */
PiBool PI_API pi_rendertarget_set_viewport(PiRenderTarget *target, uint32 left, uint32 bottom, uint32 width, uint32 height);

/* att上附加视图 */
PiBool PI_API pi_rendertarget_attach(PiRenderTarget *target, TargetAttachment attachment, PiRenderView *view);

/* 卸载att上的视图 */
PiBool PI_API pi_rendertarget_detach(PiRenderTarget *target, TargetAttachment attachment);

/* 开始使用目标 */
PiBool PI_API pi_rendertarget_bind(PiRenderTarget *target);

/* 结束使用目标 */
PiBool PI_API pi_rendertarget_unbind(PiRenderTarget *target);

/** 
 * 拷贝渲染目标，将源target的视口范围拷贝到目的target的视口范围 
 * 注意：
 *    当mask有Depth或Stencil时，filter必须是Nearest
 *    当mask含depth或stencil时，depth和stencil格式必须匹配
 *    当filter是linear时，源不能喊整数数据
 *    当mask含Color时，src含定点或浮点数据，dst必须含定点或浮点数据
 *    当mask含Color时，src含无符号整形，dst必须含无符号整形
 *    当mask含Color时，src含有符号整形，dst必须含有符号整形
 *    当src和dst为同一个指针，或者指向同一个视图区域时，两个目标的视口不能相同，否则驱动无定义
 */
PiBool PI_API pi_rendertarget_copy(PiRenderTarget *dst, PiRenderTarget *src, TargetBufferMask mask, TargetFilterType filter);

PI_END_DECLS

#endif /* INCLUDE_RENDERTARGET_H */