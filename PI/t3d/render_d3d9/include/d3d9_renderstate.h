#ifndef INCLUDE_D3D9_RENDERSTATE_H
#define INCLUDE_D3D9_RENDERSTATE_H

#include "renderstate.h"
#include "rendersystem.h"
#include "d3d9_renderlayout.h"

#include <d3d9.h>

#define MAX_VERTEX_SAMPLER 4
#define MAX_PIXEL_SAMPLER 16

typedef struct
{
	BlendState bs;
	RasterizerState rs;
	DepthStencilState dss;

	IDirect3DVertexShader9 *vertex_shader;
	IDirect3DPixelShader9 *pixel_shader;

	PiRenderTarget *target;

	SamplerState vs_unit_slot[MAX_VERTEX_SAMPLER];
	SamplerState ps_unit_slot[MAX_PIXEL_SAMPLER];

	/* 视口 */
	uint32 left, bottom, width, height;

	/* 上一次设置的和不同于默认的的状态 */
	StateList last_state_list;
	StateList cache_state_list;	/* 只用于设置时候计算用的缓存 */

	/* 设备的主目标 */
	PiRenderView *back_buffer_view;
	PiRenderView *back_depth_stencil_view;

	/* 设备丢失时候，需要释放和创建的纹理，vb，ib的集合 */
	PiVector view_vector;
	PiVector default_vb_vector;
	PiVector default_ib_vector;
	PiVector default_texture_vector;
	
	/* 全局默认状态 */
	uint32 def_state[MAX_STATE_LEN];

	/* 当前StreamSource */
	void* curr_stream_source[EVS_NUM];
	void *curr_vertex_declaration;
	void *curr_ib;
} D3D9RenderState;

/* 以下接口仅供t3d_d3d9_内部使用 */

void d3d9_state_init(D3D9RenderState *state);

void d3d9_state_clear(D3D9RenderState *state);

void d3d9_state_reset(D3D9RenderState *state);

void d3d9_state_set_viewport(uint32 left, uint32 bottom, uint32 width, uint32 height);

void d3d9_state_set_uniform(GpuProgram *program, Uniform *uniform, Uniform *material_uniform);

void d3d9_state_set_default_uniform(uint index, void* value, uint32 count, PiBool is_vs, PiBool is_ps);

PiBool d3d9_state_use_shader(GpuProgram *program);

PiBool d3d9_state_remove_texture(PiTexture *tex);

void d3d9_state_force_sampler_state(void);

// 重置前，清空所有的default资源
void d3d9_state_before_reset(void);

// 重置后，创建所有的default资源
void d3d9_state_after_reset(void);

void d3d9_state_add_default_texture(PiTexture *tex);

void d3d9_state_remove_default_texture(PiTexture *tex);

void d3d9_state_add_default_vb(D3D9VertexElement *vb);

void d3d9_state_remove_default_vb(D3D9VertexElement *vb);

void d3d9_state_add_default_ib(D3D9RenderLayout *ib);

void d3d9_state_remove_default_ib(D3D9RenderLayout *ib);

void d3d9_state_add_view(PiRenderView *view);

void d3d9_state_remove_view(PiRenderView *view);

#endif /* INCLUDE_D3D9_RENDERSTATE_H */
