#ifndef INCLUDE_RENDERSYSTEM_H
#define INCLUDE_RENDERSYSTEM_H

#include <pi_lib.h>

#include <renderview.h>
#include <rendercap.h>
#include <renderstate.h>
#include <rendertarget.h>
#include <renderlayout.h>

#include <shader.h>
#include <texture.h>
#include <entity.h>

/* 加载渲染环境的类型*/
typedef enum 
{
	CONTEXT_LOAD_EMPTY,
	CONTEXT_LOAD_NATIVE	
}RenderContextLoadType;

/* 渲染检查的返回值 */
typedef enum
{
	CHECK_LOST,
	CHECK_RUNNING,
	CHECK_REST
}RenderCheckType;

typedef struct PiRenderContextParams
{
	MULTISAMPLE_TYPE multiSampleType;
	uint16 multiSampleQuality;
}PiRenderContextParams;

/**
 * 渲染系统，渲染线程只有一个实例
 */

typedef struct 
{
	/* 帧数 */
	uint frame;
	
	RenderContextLoadType type;

	/* 着色器默认的uniform变量, 以g_开头 */
	GDefaultVariable gdv;

	/* camera的信息 */
	PiVector3* view_position;
	PiMatrix4 view_mat;
	PiMatrix4 proj_mat;
	PiMatrix4 view_proj_mat;

	/* 渲染特性，由具体的渲染系统负责初始化和销毁 */
	PiRenderCap cap;
	
	BlendState bs;
	RasterizerState rs;
	DepthStencilState dss;
	
	PiHash source_map;				/* 元素为 ShaderSource，键 source_key域 */
	PiHash shader_map[ST_NUM];		/* 元素为 ShaderData, 键 key域 */
	PiHash program_map;				/* 元素为 Program，键 <vs_id, fs_id> */

	PiRenderTarget main_target;

	PiRenderContextParams contextParams;

	void *impl;
}PiRenderSystem;

PI_BEGIN_DECLS

RenderContextLoadType PI_API pi_rendersystem_get_type(void);

PiRenderSystem* PI_API pi_rendersystem_get_instance(void);

PiRenderTarget* PI_API pi_rendersystem_init(RenderContextLoadType type, uint width, uint height, void *data, PiRenderContextParams* param);

void PI_API pi_rendersystem_clear(void);

RenderCheckType PI_API pi_rendersystem_check(void);

PiBool PI_API pi_rendersystem_begin_draw(void);

PiBool PI_API pi_rendersystem_end_draw(void);

/* 重置渲染环境 */
PiBool PI_API pi_rendersystem_reset(PiBool is_window, uint width, uint height);

/* 设置着色器的源码内容 */
PiBool PI_API pi_rendersystem_add_shader_source(const wchar *key, byte *data, uint size);

/* 添加预编译shader*/
PiBool PI_API pi_rendersystem_add_compiled_shader(const char *key, byte *data, uint size, ShaderType type);


/* 移除着色器的源码内容 */
void PI_API pi_rendersystem_remove_shader_source(const wchar *key);

/* 设置渲染目标和视口 */
PiBool PI_API pi_rendersystem_set_target(PiRenderTarget* rt);

/* 设置着色器程序 */
PiBool PI_API pi_rendersystem_set_program(void *program);

/* 设置相机矩阵，用于C */
PiBool PI_API pi_rendersystem_set_camera(PiCamera *camera);

/* 设置相机数据，用于JS */
PiBool PI_API pi_rendersystem_set_camera_data(PiVector3 *location, PiMatrix4 *view_mat, PiMatrix4 *proj_mat);

/* 设置这一帧的渲染时间 */

PiBool PI_API pi_rendersystem_set_frame_time(float ms);

/* 利用当前渲染状态渲染数据 */
PiBool PI_API pi_rendersystem_draw(PiEntity *entity, uint num);

/* 清空目标的缓冲区 */
PiBool PI_API pi_rendersystem_clearview(uint32 flags, PiColor *clr, float depth, sint stencil);

/* 交换缓冲区 */
void PI_API pi_rendersystem_swapbuffer();

/* 获得当前的帧数 */
uint PI_API pi_rendersystem_get_frame();

/* 获得渲染特性的指针 */
PiRenderCap* PI_API pi_rendersystem_get_cap(void);

uint PI_API pi_rendersystem_get_device_vendor();

PI_END_DECLS

#endif /* INCLUDE_RENDERSYSTEM_H */