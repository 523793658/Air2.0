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
	/* ��ǰ���������Ԫ */
	sint current_active_unit;

	/* ������� */
	sint max_num;

	/* Ŀǰ�Ŀ��в� */
	uint free_slot_num;
	uint *free_slot;

	TextureUnitSlot unit_slot[MAX_TEXCOORD_NUM];

	/* �������ݣ���LRU */
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

	GpuProgram *program;	/* ��ɫ������ */
	PiRenderTarget *target;
	
	/* ������������ */
	uint max_attrib_num;
	/* �������Ԫ��Ŀ */
	uint max_texture_unit;

	TextureCache tex_cache;

	/* ��յ���ɫ����ȣ�ģ�� */
	float clear_depth;
	uint clear_stencil;
	PiColor clear_color;
	
	/* �ӿ� */
	uint32 left, bottom, width, height;

	/* ��һ�����õĺͲ�ͬ��Ĭ�ϵĵ�״̬ */
	StateList last_state_list;
	StateList cache_state_list;	/* ֻ��������ʱ������õĻ��� */
	
	/* �Ƿ�����SRGB */
	PiBool is_srgb_enable;

	/* ȫ��Ĭ��״̬ */
	uint32 def_state[MAX_STATE_LEN];
}GLRenderState;

PI_BEGIN_DECLS

/* ���½ӿڽ���opengl�ڲ�ʹ�� */

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