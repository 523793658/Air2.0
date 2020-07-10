#ifndef INCLUDE_GL_RENDERSTATE_H
#define INCLUDE_GL_RENDERSTATE_H

#include <renderstate.h>
#include <rendersystem.h>

typedef struct  
{
	uint id;
	uint target;
}TextureUnitSlot;

typedef struct  
{
	/* 当前激活的纹理单元 */
	sint current_active_unit;

	/* 最大数量 */
	sint max_num;

	/* 目前的空闲槽 */
	uint free_slot_num;
	uint *free_slot;

	TextureUnitSlot unit_slot[MAX_TEXCOORD_NUM];

	/* 缓存数据，用LRU */
	sint use_num;
	PiTexture **cache;
}TextureCache;

typedef struct
{
	BlendState bs;
	RasterizerState rs;
	DepthStencilState dss;
	
	uint fbo;
	uint current_vao;

	GpuProgram *program;	/* 着色器程序 */
	PiRenderTarget *target;
	
	/* 最大的流的数量 */
	uint max_attrib_num;
	/* 最大纹理单元数目 */
	uint max_texture_unit;

	TextureCache tex_cache;

	/* 清空的颜色，深度，模板 */
	float clear_depth;
	uint clear_stencil;
	PiColor clear_color;
	
	/* 视口 */
	uint32 left, bottom, width, height;

	/* 上一次设置的和不同于默认的的状态 */
	StateList last_state_list;
	StateList cache_state_list;	/* 只用于设置时候计算用的缓存 */
	
	/* 是否允许SRGB */
	PiBool is_srgb_enable;

	/* 全局默认状态 */
	uint32 def_state[MAX_STATE_LEN];
}GLRenderState;

PI_BEGIN_DECLS

/* 以下接口仅供opengl内部使用 */

PiBool glstate_init(GLRenderState *state);

void glstate_clear(GLRenderState *state);

void glstate_set_viewport(uint left, uint bottom, uint width, uint height);

PiBool glstate_delete_rendertarget(PiRenderTarget *rs);

void glstate_bind_texture(PiTexture *tex);

uint32 glstate_bind_fbo(uint fbo);

PiBool glstate_enable_fbo_srgb(PiBool is_srgb_enable);

PiBool glstate_use_shader(GpuProgram *program);

PiBool glstate_remove_texture(PiTexture *tex);

PiBool texturecache_insert(TextureCache *cache, PiTexture *tex);

PiBool texturecache_remove(TextureCache *cache, PiTexture *tex);

PI_END_DECLS

#endif /* INCLUDE_GL_RENDERSTATE_H */