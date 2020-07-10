#ifndef INCLUDE_RENDERPIPELINE_H
#define INCLUDE_RENDERPIPELINE_H

#include <pi_lib.h>
#include <renderbatch.h>

enum { SIZEOF_RENDERPIPELINE = 8 + SIZEOF_DVECTOR + sizeof(FullState) / sizeof(uint)};

/**
 * 渲染管线
 */
typedef struct 
 {
	 uint		mem[SIZEOF_RENDERPIPELINE];
 } PiRenderPipeline;

PI_BEGIN_DECLS

void PI_API pi_renderpipeline_init(PiRenderPipeline *pipeline);

void PI_API pi_renderpipeline_clear(PiRenderPipeline *pipeline);

/**
 * 设置管线的全局渲染状态
 * 其调用必须在所有batch创建之前全局初始化好，否则会引起断言
 */
void PI_API pi_renderpipeline_set_state(PiRenderPipeline *pipeline, RenderStateType key, uint32 value);

/* 流水线渲染 */
void PI_API pi_renderpipeline_draw(PiRenderPipeline *pipeline);

/* 获得缺省渲染状态及上次设置的最后状态列表 */
FullState* PI_API pi_renderpipeline_get_full_state(PiRenderPipeline *pipeline);

/* 设置用户数据 */
void PI_API pi_renderpipeline_set_userdata(PiRenderPipeline *pipeline, void* data);

/* 获取用户数据 */
void* PI_API pi_renderpipeline_get_userdata(PiRenderPipeline *pipeline);

/* 获取当前的帧数，用于不同帧之间的uniform全局比较 */
uint PI_API pi_renderpipeline_get_frame(PiRenderPipeline *pipelline);

PI_END_DECLS

#endif /* INCLUDE_RENDERPIPELINE_H */