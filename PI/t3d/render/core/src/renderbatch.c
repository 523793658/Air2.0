/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */


#include <renderbatch.h>
#include <renderpipeline.h>
#include <rendersystem.h>
#include "renderlistentry.h"
#include "renderutil.h"

/* 最大状态环境的长度 */
#define MAX_STATE_ENV_LEN 16

/* 最大shader环境的长度 */
#define MAX_SHADER_ENV_LEN 16

/**
 * 渲染批次实现
 */
typedef struct
{
	PiBool is_enable;							/* 是否可用，默认是可用的 */
	PiRenderPipeline *pipeline;

	PiDhash uniform_env[MAX_SHADER_ENV_LEN];	/* uniform环境 */
	StateList state_env[MAX_STATE_ENV_LEN];		/* 状态环境 */

	PiRenderTarget *target;	/* 渲染目标 */
	PiRenderList* list;		/* 渲染列表 */
	
	PiMatrix4 view;			/* 视图矩阵 */
	PiMatrix4 proj;			/* 投影矩阵 */
	sint priority;			/* 优先级，越大越优先 */
} RenderBatchImpl;

/** 
 * 在渲染流水线上加入指定渲染批次，仅供RenderBatch内部操作
 * 仅由RenderBatch模块内部使用，应用代码不要调用
 */
void renderpipeline_add(PiRenderPipeline *pipeline, PiRenderBatch *batch);

/**
 * 在渲染流水线上删除指定的渲染批次，仅供RenderBatch内部操作
 * 仅由RenderBatch模块内部使用，应用代码不要调用
 */
void renderpipeline_remove(PiRenderPipeline *pipeline, PiRenderBatch *batch);

/* 越大越优先 */
void PI_API pi_renderbatch_init(PiRenderBatch *batch, void *pipeline, sint priority)
{
	uint i;
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	FullState *full = pi_renderpipeline_get_full_state(pipeline);
	
	pi_memset(impl, 0, sizeof(RenderBatchImpl));
	impl->priority = priority;
	impl->pipeline = pipeline;
	
	for(i = 0; i < MAX_STATE_ENV_LEN; ++i)
	{
		pi_memcpy(&impl->state_env[i], &full->state, MAX_STATE_LEN * sizeof(uint32));
		impl->state_env[i].len = 0;
	}

	for(i = 0; i < MAX_SHADER_ENV_LEN; ++i) 
	{
		pi_dhash_init(&impl->uniform_env[i], sizeof(Uniform), 0.75, pi_kv_str_hash, pi_kv_str_equal);
	}

	pi_renderbatch_set_enable(batch, TRUE);
}

void PI_API pi_renderbatch_set_enable(PiRenderBatch *batch, PiBool is_enable)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	if(is_enable != impl->is_enable)
	{
		impl->is_enable = is_enable;
		if(is_enable)
		{
			renderpipeline_add(impl->pipeline, batch);
		}
		else
		{
			renderpipeline_remove(impl->pipeline, batch);
		}
	}
}

sint PI_API pi_renderbatch_get_priority(PiRenderBatch *batch)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	return impl->priority;
}

void PI_API pi_renderbatch_clear(PiRenderBatch *batch, PiBool free_buf)
{
	uint i;
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	
	for(i = 0; i < MAX_SHADER_ENV_LEN; i++) 
	{
		pi_dhash_foreach(&impl->uniform_env[i], (PiSelectFunc)renderutil_free_uniform, NULL);
		pi_dhash_clear(&impl->uniform_env[i], free_buf);
	}
}

void PI_API pi_renderbatch_set_target(PiRenderBatch *batch, PiRenderTarget *target)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	impl->target = target;
}

void PI_API pi_renderbatch_set_list(PiRenderBatch *batch, PiRenderList* list)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	impl->list = list;
}

void PI_API pi_renderbatch_set_view(PiRenderBatch *batch, PiMatrix4 *view)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	pi_memcpy(&impl->view, view, sizeof(PiMatrix4));
}

void PI_API pi_renderbatch_set_proj(PiRenderBatch *batch, PiMatrix4 *proj)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	pi_memcpy(&impl->proj, proj, sizeof(PiMatrix4));
}

void PI_API pi_renderbatch_set_state(PiRenderBatch *batch, uint index, RenderStateType key, uint32 value)
{
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;
	FullState *full = pi_renderpipeline_get_full_state(impl->pipeline);
	if(full->state[key] == value) 
	{/* 需要设置的值和全局的一样，就删掉差异上的值 */
		pi_renderutil_state_remove(&impl->state_env[index], key);
	}
	else
	{
		pi_renderutil_state_add(&impl->state_env[index], key, value);
	}
}

void PI_API pi_renderbatch_set_uniform(PiRenderBatch *batch, 
	uint index, const char *name, UniformType type, uint32 count, void *data)
{
	Uniform old, *u;
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;

	Uniform uniform;
	pi_memset(&uniform, 0, sizeof(uniform));
	uniform.name = (char *)name;
	uniform.type = type;
	uniform.count = count;
	uniform.value = data;

	if(pi_dhash_enter(&impl->uniform_env[index], &uniform, &old, &u))
	{
		/* 重用旧的内存 */
		renderutil_copy_uniform(&old, &uniform);
		u->value = old.value;
		u->count = old.count;
	}else{
		/* 新拷贝 */
		u->value = NULL;
		renderutil_copy_uniform(u, &uniform);
	}
}

void PI_API pi_renderbatch_draw(PiRenderBatch *batch)
{
	PiMatrix4 id_mat;
	uint i, n, frame;
	
	/* 默认uniform的值 */
	GDefaultVariable gdv;
	
	/* pipeline 全局状态 */
	FullState *full = NULL;
	
	RenderBatchImpl* impl = (RenderBatchImpl*)batch;

	/* 每个entity对应的batch上的某个环境上的状态 */
	StateList *last_env_state = NULL, *cur_env_state = NULL;
	
	/* 实体对应材质上的状态 */
	StateList *last_entity_state = NULL, *cur_entity_state = NULL;
		
	/* 上一次和这一次环境和实体合并后的状态 */
	StateList cur_merge, last_merge;
	pi_memset(&cur_merge, 0, sizeof(cur_merge));
	pi_memset(&last_merge, 0, sizeof(last_merge));

	if(!impl->is_enable)
	{
		return;
	}

	n = pi_dvector_size(&impl->list->entries);
	if(n == 0)
	{
		return;
	}
	
	/* 准备好默认的uniform值 */
	gdv.wvp_mask = 0;
	pi_mat4_set_identity(&id_mat);
	gdv.g_world = &id_mat;
	gdv.g_view = &impl->view;
	gdv.g_proj = &impl->proj;
	gdv.g_viewport[0] = impl->target->width;
	gdv.g_viewport[1] = impl->target->height;
	
	frame = pi_renderpipeline_get_frame(impl->pipeline);
	
	/* 获得缺省渲染状态及上次设置的最后状态列表 */
	full = pi_renderpipeline_get_full_state(impl->pipeline);
	
	/* 绑定渲染目标 */
	pi_rendersystem_set_target(impl->target);

	last_env_state = &full->diff;

	for(i = 0; i < n; ++i)
	{
		RenderListEntry *e = pi_dvector_get(&impl->list->entries, i);
		void* program = e->entity->material->program;
				
		/* 处理状态变化 */
		cur_env_state = &impl->state_env[e->state_env];
		cur_entity_state = &e->entity->material->state;
		
		renderutil_merge_order_set(&cur_merge, cur_env_state, cur_entity_state);
		
		if(last_env_state != cur_env_state)
		{
			pi_renderutil_state_order_set(full->state, &last_merge, &cur_merge);
			
			last_env_state = cur_env_state;
		}
		else
		{
			/* 环境相同，将上个改变状态和本次改变状态比较 */
			pi_renderutil_state_order_set((const uint32 *)cur_env_state, last_entity_state, cur_entity_state);
		}
		
		/* 作为下一次的合并使用 */
		pi_memcpy(&last_merge, &cur_merge, sizeof(cur_merge));

		last_entity_state = cur_entity_state;

		/* 处理uniform */

		if(i == 0 || !pi_mat4_is_equal(gdv.g_world, &e->entity->world_mat))
		{/* 涉及到world的矩阵都需要重新计算 */
			gdv.g_world = &e->entity->world_mat;
			gdv.wvp_mask &= ~WVP_NORMAL;
			gdv.wvp_mask &= ~WVP_WORLD_VIEW;
			gdv.wvp_mask &= ~WVP_WORLD_VIEW_PROJ;
		}
		
		renderutil_program_set_uniform(program, frame, &gdv,
			&impl->uniform_env[e->shader_env], &e->entity->material->uniforms, TRUE);
		
		/* 设置Shader */
		pi_rendersystem_set_program(program);
		
		pi_rendersystem_draw(e->entity);
	}

	if(cur_env_state != NULL)
	{/* 将最后的改变状态记录在全局状态上 */
		pi_memset(&full->diff, 0, sizeof(full->diff));
		renderutil_merge_order_set(&full->diff, cur_env_state, cur_entity_state);
	}
}