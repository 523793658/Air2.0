#include <gl_renderlayout.h>
#include <gl_convert.h>
#include <gl_shader.h>
#include <renderinfo.h>
#include <pi_renderdata.h>
#include <gl_rendersystem.h>

#include <gl_interface.h>

extern PiRenderSystem *g_rsystem;

static void _gl_bind_array_buffer(uint vao)
{
	GLRenderSystem *gl_system = g_rsystem->impl;
	GLRenderState *state = &gl_system->state;
	
	if(state->current_vao != vao)
	{
		state->current_vao = vao;
		gl3_BindVertexArray(vao); 	
	}	
}

static void _gl_bind_buffer(uint type, uint id)
{
	if(type == GL2_ELEMENT_ARRAY_BUFFER)
	{
		pi_renderinfo_add_ib_change_num(1);
	}
	else if(type == GL2_ARRAY_BUFFER)
	{
		pi_renderinfo_add_vb_change_num(1);
	}
	gl2_BindBuffer(type, id);
}

PiBool PI_API render_layout_init(PiRenderLayout *layout)
{
	GLRenderLayout *impl = pi_new0(GLRenderLayout, 1);
	layout->impl = impl;
	return TRUE;
}

PiBool PI_API render_layout_clear(PiRenderLayout *layout)
{
	GLRenderLayout *impl = layout->impl;
	if(impl != NULL)
	{
		uint i;
		if(impl->gl_index_id != 0)
		{
			pi_renderinfo_add_ib_num(-1);
			gl2_DeleteBuffers(1, &impl->gl_index_id);
		}
		for(i = 0; i < EVS_NUM; ++i)
		{
			if(impl->vertexs[i].gl_vbo_id != 0)
			{
				pi_renderinfo_add_vb_num(-1);
				gl2_DeleteBuffers(1, &impl->vertexs[i].gl_vbo_id);
			}
		}

		if(impl->gl_vao_id != 0)
		{
			gl3_DeleteVertexArrays(1, &impl->gl_vao_id);
		}

		pi_free(impl);
	}
	layout->impl = NULL;
	return TRUE;
}

static PiBool _renderlayout_set_index(GLRenderLayout *layout, EBufferUsage usage, EIndexType type, uint num, void *data)
{
	uint size;
	PiBool is_vao_dirty = FALSE;
	
	if(num != 0 && data != NULL)
	{
		if(layout->gl_index_id == 0)
		{
			is_vao_dirty = TRUE;
			pi_renderinfo_add_ib_num(1);
			gl2_GenBuffers(1, &layout->gl_index_id);
		}

		layout->index_num = num;
		layout->gl_index_type = (type == EINDEX_16BIT) ? GL2_UNSIGNED_SHORT : GL2_UNSIGNED_INT;
		size = num;
		size *= (type == EINDEX_16BIT) ? 2 : 4;

		_gl_bind_buffer(GL2_ELEMENT_ARRAY_BUFFER, layout->gl_index_id);

		if(layout->index_size >= size)
		{/* 局部更新即可 */
			gl2_BufferSubData(layout->gl_index_id, 0, size, data);
		}
		else
		{/* 需要创建显存 */
			uint gl_usage = gl_buffer_usage_get(usage);
			layout->index_size = 0;
			gl2_BufferData(GL2_ELEMENT_ARRAY_BUFFER, size, data, gl_usage);
		}
	}
	else 
	{/* 释放IB */
		if(layout->gl_index_id != 0)
		{
			is_vao_dirty = TRUE;
			pi_renderinfo_add_ib_num(-1);
			gl2_DeleteBuffers(1, &layout->gl_index_id);
			layout->gl_index_id = 0;
			layout->index_size = 0;
		}
	}
	return is_vao_dirty;
}

static PiBool _renderlayout_set_vertex(GLRenderLayout *layout, EBufferUsage usage, VertexSemantic semantic, EVertexType type, uint32 num, void *data, uint32 size)
{
	PiBool is_vao_dirty = FALSE;
	VertexLayout *vertex_layout = &layout->vertexs[semantic];
	
	if(size != 0 && data != NULL)
	{
		if(vertex_layout->gl_vbo_id == 0)
		{/* 创建VBO */
			is_vao_dirty = TRUE;
			pi_renderinfo_add_vb_num(1);
			gl2_GenBuffers(1, &vertex_layout->gl_vbo_id);
		}
		
		_gl_bind_buffer(GL2_ARRAY_BUFFER, vertex_layout->gl_vbo_id);

		vertex_layout->num = num;
		vertex_layout->gl_type = gl_vertex_type_get(type);
		
		if(vertex_layout->buffer_size >= size)
		{
			gl2_BufferSubData(GL2_ARRAY_BUFFER, 0, size, data);
		}
		else
		{
			uint gl_usage = gl_buffer_usage_get(usage);
			vertex_layout->buffer_size = size;
			gl2_BufferData(GL2_ARRAY_BUFFER, size, data, gl_usage);
		}
	}
	else
	{
		if(vertex_layout->gl_vbo_id != 0)
		{
			is_vao_dirty = TRUE;
			pi_renderinfo_add_vb_num(-1);
			gl2_DeleteBuffers(1, &vertex_layout->gl_vbo_id);
			vertex_layout->num = 0;
			vertex_layout->gl_type = 0;
			vertex_layout->gl_vbo_id = 0;
			vertex_layout->buffer_size = 0;			
		}
	}
	return is_vao_dirty;
}

static void _bind_vbo_layout(GLRenderLayout *layout)
{
	uint i;
	PiBool is_enable;
	
	GLRenderSystem *gl_system = g_rsystem->impl;
	GLRenderState *state = &gl_system->state;

	_gl_bind_buffer(GL2_ELEMENT_ARRAY_BUFFER, layout->gl_index_id);
	for(i = 0; i < state->max_attrib_num; ++i)
	{
		VertexLayout *v = &layout->vertexs[i];
		
		is_enable = (v->gl_vbo_id != 0);
		if(is_enable)
		{
			gl2_EnableVertexAttribArray(i);
		}
		else
		{
			gl2_DisableVertexAttribArray(i);
		}
		
		if(is_enable)
		{
			_gl_bind_buffer(GL2_ARRAY_BUFFER, layout->vertexs[i].gl_vbo_id);
			gl2_VertexAttribPointer(i, v->num, v->gl_type, FALSE, 0, 0);
		}
	}
}

typedef struct  
{
	PiBool is_vao_dirty;
	GLRenderLayout *gl_layout;
}VertexUpdateData;

static PiSelectR PI_API _update_vertex(void *user_data, void *value)
{
	VertexUpdateData *v_data = (VertexUpdateData *)user_data;
	GLRenderLayout *layout = v_data->gl_layout;
	VertexElement *elem = (VertexElement *)value;

	if(elem->is_dirty)
	{
		elem->is_dirty = FALSE;
		v_data->is_vao_dirty |= _renderlayout_set_vertex(layout, elem->usage, elem->semantic, elem->type, elem->num, elem->data, elem->size);
	}
	return SELECT_NEXT;
}

PiBool PI_API render_layout_update(PiRenderMesh *mesh)
{
	PiBool is_vao_dirty = FALSE;
	GLRenderLayout *gl_layout;
	PiRenderData *data = &mesh->mesh->data;
	PiRenderLayout *layout = (PiRenderLayout *)mesh->gpu_data;
	PiBool is_dirty = data->is_dirty;

	if(layout == NULL)
	{
		mesh->gpu_data = layout = pi_renderlayout_new();
	}

	gl_layout = layout->impl;

	if(is_dirty)
	{
		VertexUpdateData v_data;
		data->is_dirty = FALSE;
		
		_gl_bind_array_buffer(0);

		gl_layout->gl_primary_type = gl_primitive_get(data->type);

		if(data->idata.is_dirty)
		{
			data->idata.is_dirty = FALSE;
			is_vao_dirty = _renderlayout_set_index(gl_layout, data->idata.usage, data->idata.type, data->idata.num, data->idata.data);
		}
		gl_layout->vertex_num = data->vertex_num;

		v_data.is_vao_dirty = FALSE;
		v_data.gl_layout = gl_layout;

		pi_dhash_foreach(&data->vertex_map, _update_vertex, &v_data);

		is_vao_dirty |= v_data.is_vao_dirty;

		/* VAO */
		if(gl_Self_IsVertexArrayObject())
		{
			if(gl_layout->gl_vao_id == 0)
			{
				is_vao_dirty = TRUE;
				gl3_GenVertexArrays(1, &gl_layout->gl_vao_id);
			}
			
			if(is_vao_dirty)
			{
				_gl_bind_array_buffer(gl_layout->gl_vao_id);
				_bind_vbo_layout(gl_layout);
			}
		}
	}

	return TRUE;
}

PiBool PI_API gl_renderlayout_draw(PiRenderMesh *mesh, uint num)
{
	PiRenderLayout *layout = (PiRenderLayout*)mesh->gpu_data;
	GLRenderLayout *layout_impl = layout->impl;

	if(layout_impl->gl_vao_id != 0)
	{
		_gl_bind_array_buffer(layout_impl->gl_vao_id); 
	}
	else
	{
		_bind_vbo_layout(layout_impl);
	}

	if (layout_impl->gl_index_id == 0)
	{
		if(num == 1)
		{
			gl2_DrawArrays(layout_impl->gl_primary_type, 0, layout_impl->vertex_num);
		}
		else
		{
			gl3_DrawArraysInstanced(layout_impl->gl_primary_type, 0, layout_impl->vertex_num, num);
		}
	}
	else
	{
		if(num == 1)
		{
			gl2_DrawElements(layout_impl->gl_primary_type, layout_impl->index_num, layout_impl->gl_index_type, 0);
		}
		else
		{
			gl3_DrawElementsInstanced(layout_impl->gl_primary_type, layout_impl->index_num, layout_impl->gl_index_type, 0, num);
		}
	}

	return TRUE;
}