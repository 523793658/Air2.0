#include "d3d9_renderlayout.h"
#include "d3d9_convert.h"
#include "d3d9_rendersystem.h"
#include "d3d9_renderstate.h"

#include "renderinfo.h"

extern "C" {

	extern PiRenderSystem *g_rsystem;

	void d3d9_create_vb(D3D9VertexElement *vertex_layout)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		IDirect3DDevice9_CreateVertexBuffer(context->device, vertex_layout->buffer_size, vertex_layout->d3d9_usage, 0, vertex_layout->d3d9_pool, &vertex_layout->vertex_buffer, NULL);
	}

	void d3d9_release_vb(D3D9VertexElement *element)
	{
		IDirect3DVertexBuffer9_Release(element->vertex_buffer);
		element->vertex_buffer = NULL;
	}

	PiBool PI_API vertex_element_init(VertexElement *vertex_element)
	{
		D3D9VertexElement *vertex_layout = pi_new0(D3D9VertexElement, 1);
		vertex_element->impl = vertex_layout;
		vertex_element->is_init = TRUE;
		if (vertex_element->size != 0)
		{
			vertex_layout->decl.Stream = (WORD)vertex_element->semantic;
			vertex_layout->decl.Offset = 0;
			d3d9_vertex_type_get(vertex_element->type, vertex_element->num, &vertex_layout->decl.Type, &vertex_layout->stride);
			vertex_layout->decl.Method = D3DDECLMETHOD_DEFAULT;
			d3d9_vertex_semantic_get(vertex_element->semantic, &vertex_layout->decl.Usage, &vertex_layout->decl.UsageIndex);

			d3d9_buffer_usage_get(vertex_element->usage, &vertex_layout->d3d9_usage, &vertex_layout->d3d9_pool, &vertex_layout->d3d9_lock_flags);

			vertex_layout->buffer_size = vertex_element->size;
			if (vertex_layout->vertex_buffer == NULL)
			{
				pi_renderinfo_add_vb_num(1);
				if (vertex_layout->d3d9_pool == D3DPOOL_DEFAULT)
				{
					d3d9_state_add_default_vb(vertex_layout);
				}
				d3d9_create_vb(vertex_layout);
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}



	void PI_API vertex_element_update(VertexElement* element, uint first_vertex, uint num_vertex, const void* data, const uint32 srcStride)
	{

		void* vertices;
		D3D9VertexElement* vertex_element = (D3D9VertexElement*)element->impl;
		uint32 buffer_size = vertex_element->stride * num_vertex;
		IDirect3DVertexBuffer9_Lock(vertex_element->vertex_buffer, vertex_element->stride * first_vertex, buffer_size, &vertices, vertex_element->d3d9_lock_flags);
		if (vertex_element->stride != srcStride)
		{
			uint i;
			for (i = 0; i < num_vertex; ++i)
			{
				pi_memcpy_inline(vertices, data, srcStride);
				vertices = ((uint8_t*)vertices) + vertex_element->stride;
				data = ((uint8_t*)data) + srcStride;
			}
		}
		else
		{
			pi_memcpy_inline(vertices, data, buffer_size);
		}
		IDirect3DVertexBuffer9_Unlock(vertex_element->vertex_buffer);
	}

	void PI_API vertex_element_free(VertexElement* element)
	{
		D3D9VertexElement* vertex_element = (D3D9VertexElement*)element->impl;
		if (vertex_element->vertex_buffer != NULL)
		{
			pi_renderinfo_add_vb_num(-1);
			if (vertex_element->d3d9_pool == D3DPOOL_DEFAULT)
			{
				d3d9_state_remove_default_vb(vertex_element);
			}
			d3d9_release_vb(vertex_element);
		}
		pi_free(vertex_element);
	}

	void d3d9_create_ib(D3D9RenderLayout *d3d9_layout)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		D3DFORMAT fmt = (D3DFORMAT)d3d9_layout->d3d9_index_format;
		IDirect3DDevice9_CreateIndexBuffer(context->device, d3d9_layout->buffer_size, d3d9_layout->d3d9_index_usage, fmt, d3d9_layout->d3d9_index_pool, &d3d9_layout->d3d9_ib, NULL);
	}


	
	void d3d9_release_ib(D3D9RenderLayout *layout)
	{
		IDirect3DIndexBuffer9_Release(layout->d3d9_ib);
		layout->d3d9_ib = NULL;
	}

	PiBool PI_API index_data_init(IndexData* index_data)
	{
		D3D9IndexElement *d3d9_index_element = pi_new0(D3D9IndexElement, 1);
		index_data->impl = d3d9_index_element;
		if (index_data->num != 0)
		{
			uint buffer_size;
			d3d9_index_element->index_num = index_data->num;
			d3d9_buffer_usage_get(index_data->usage, &d3d9_index_element->d3d9_index_usage, &d3d9_index_element->d3d9_index_pool, &d3d9_index_element->d3d9_index_lock_flags);
			d3d9_index_element->d3d9_index_format = (index_data->type == EINDEX_16BIT) ? D3DFMT_INDEX16 : D3DFMT_INDEX32;

			buffer_size = ((index_data->type == EINDEX_16BIT) ? 2 : 4) * index_data->num;

			d3d9_index_element->buffer_size = buffer_size;

			if (d3d9_index_element->d3d9_ib == NULL)
			{
				pi_renderinfo_add_ib_num(1);
				D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
				D3D9Context *context = d3d9_system->context;
				IDirect3DDevice9_CreateIndexBuffer(context->device, d3d9_index_element->buffer_size, d3d9_index_element->d3d9_index_usage, d3d9_index_element->d3d9_index_format, d3d9_index_element->d3d9_index_pool, &d3d9_index_element->d3d9_ib, NULL);
			}
			return TRUE;
		}
		return FALSE;
	}

	void PI_API index_data_update(IndexData* index_data, uint offset, uint num, const void * data)
	{
		VOID *indices;
		uint stride = (index_data->type == EINDEX_16BIT) ? 2 : 4;

		D3D9IndexElement* d3d9_index_data = (D3D9IndexElement*)index_data->impl;
		IDirect3DIndexBuffer9_Lock(d3d9_index_data->d3d9_ib, offset * stride, num * stride, &indices, d3d9_index_data->d3d9_index_lock_flags);
		pi_memcpy_inline(indices, data, num * stride);
		IDirect3DIndexBuffer9_Unlock(d3d9_index_data->d3d9_ib);
	}

	void PI_API index_data_free(IndexData* index_data)
	{
		D3D9IndexElement* index_element = (D3D9IndexElement*)index_data->impl;
		if (index_element->d3d9_ib)
		{
			IDirect3DIndexBuffer9_Release(index_element->d3d9_ib);
			index_element->d3d9_ib = NULL;
		}
		pi_free(index_element);
	}

	PiBool PI_API render_layout_init(PiRenderLayout *layout)
	{
		D3D9RenderLayout *d3d9_layout = pi_new0(D3D9RenderLayout, 1);
		layout->impl = d3d9_layout;
		return TRUE;
	}

	PiBool PI_API render_layout_clear(PiRenderLayout *layout)
	{
		D3D9RenderLayout *d3d9_layout = (D3D9RenderLayout *)layout->impl;
		if (d3d9_layout != NULL)
		{
			uint i;

			if (d3d9_layout->vertex_declaration != NULL)
			{
				IDirect3DVertexDeclaration9_Release(d3d9_layout->vertex_declaration);
			}

			if (d3d9_layout->d3d9_ib != NULL)
			{
				pi_renderinfo_add_ib_num(-1);
				d3d9_release_ib(d3d9_layout);
				d3d9_state_remove_default_ib(d3d9_layout);
			}

			for (i = 0; i < EVS_NUM; ++i)
			{
				if (d3d9_layout->elements[i].vertex_buffer != NULL)
				{
					pi_renderinfo_add_vb_num(-1);
					d3d9_release_vb(&d3d9_layout->elements[i]);
					d3d9_state_remove_default_vb(&d3d9_layout->elements[i]);
				}
			}

			pi_free(d3d9_layout);

			layout->impl = NULL;
		}

		return TRUE;
	}

	static void _renderlayout_set_index(D3D9RenderLayout *d3d9_layout, EBufferUsage usage, EIndexType type, uint num_indices, void *data)
	{
		if (num_indices != 0 && data != NULL)
		{
			uint buffer_size;
			VOID *indices;

			d3d9_buffer_usage_get(usage, &d3d9_layout->d3d9_index_usage, &d3d9_layout->d3d9_index_pool, &d3d9_layout->d3d9_index_lock_flags);
			if (d3d9_layout->primitive_type == D3DPT_POINTLIST)
			{
				d3d9_layout->d3d9_index_usage |= D3DUSAGE_POINTS;
			}

			d3d9_layout->d3d9_index_format = (type == EINDEX_16BIT) ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
			
			buffer_size = ((type == EINDEX_16BIT) ? 2 : 4) * num_indices;

			if (d3d9_layout->buffer_size != buffer_size && d3d9_layout->d3d9_ib != NULL)
			{
				pi_renderinfo_add_ib_num(-1);
				d3d9_release_ib(d3d9_layout);
				d3d9_state_remove_default_ib(d3d9_layout);
			}

			d3d9_layout->buffer_size = buffer_size;

			if (d3d9_layout->d3d9_ib == NULL)
			{
				pi_renderinfo_add_ib_num(1);
				if (d3d9_layout->d3d9_index_pool == D3DPOOL_DEFAULT)
				{
					d3d9_state_add_default_ib(d3d9_layout);
				}
				d3d9_create_ib(d3d9_layout);
			}
			
			IDirect3DIndexBuffer9_Lock(d3d9_layout->d3d9_ib, 0, d3d9_layout->buffer_size, &indices, d3d9_layout->d3d9_index_lock_flags);
			pi_memcpy_inline(indices, data, d3d9_layout->buffer_size);
			IDirect3DIndexBuffer9_Unlock(d3d9_layout->d3d9_ib);
		}
		else
		{
			/* ÊÍ·ÅIB */
			if (d3d9_layout->d3d9_ib != NULL)
			{
				d3d9_release_ib(d3d9_layout);
				d3d9_state_remove_default_ib(d3d9_layout);
			}
		}
	}

	static PiBool _renderlayout_set_vertex(D3D9RenderLayout *d3d9_layout, VertexSemantic semantic, EBufferUsage usage, EVertexType type, uint32 num_components, void *data, uint32 buffer_size)
	{
		PiBool is_vertex_declaration_dirty = FALSE;
		D3D9VertexElement *vertex_layout = &d3d9_layout->elements[semantic];

		if (buffer_size != 0 && data != NULL)
		{
			VOID *vertices;

			vertex_layout->decl.Stream = (WORD)semantic;
			vertex_layout->decl.Offset = 0;
			d3d9_vertex_type_get(type, num_components, &vertex_layout->decl.Type, &vertex_layout->stride);
			vertex_layout->decl.Method = D3DDECLMETHOD_DEFAULT;
			d3d9_vertex_semantic_get(semantic, &vertex_layout->decl.Usage, &vertex_layout->decl.UsageIndex);

			d3d9_buffer_usage_get(usage, &vertex_layout->d3d9_usage, &vertex_layout->d3d9_pool, &vertex_layout->d3d9_lock_flags);
			if (d3d9_layout->primitive_type == D3DPT_POINTLIST)
			{
				vertex_layout->d3d9_usage |= D3DUSAGE_POINTS;
			}

			if (buffer_size != vertex_layout->buffer_size && vertex_layout->vertex_buffer != NULL)
			{
				pi_renderinfo_add_vb_num(-1);
				d3d9_release_vb(vertex_layout);
				d3d9_state_remove_default_vb(vertex_layout);
			}
			
			vertex_layout->buffer_size = buffer_size;
			if (vertex_layout->vertex_buffer == NULL)
			{
				pi_renderinfo_add_vb_num(1);
				if (vertex_layout->d3d9_pool == D3DPOOL_DEFAULT)
				{
					d3d9_state_add_default_vb(vertex_layout);
				}
				d3d9_create_vb(vertex_layout);
			}

			IDirect3DVertexBuffer9_Lock(vertex_layout->vertex_buffer, 0, buffer_size, &vertices, vertex_layout->d3d9_lock_flags);
			pi_memcpy_inline(vertices, data, buffer_size);
			IDirect3DVertexBuffer9_Unlock(vertex_layout->vertex_buffer);

			is_vertex_declaration_dirty = TRUE;
		}
		else
		{
			/* ÊÍ·ÅVB */
			if (vertex_layout->vertex_buffer != NULL)
			{
				d3d9_release_vb(vertex_layout);
				d3d9_state_remove_default_vb(vertex_layout);
				is_vertex_declaration_dirty = TRUE;
			}
		}

		return is_vertex_declaration_dirty;
	}

	typedef struct
	{
		PiBool is_vertex_declaration_dirty;
		D3D9RenderLayout *d3d9_layout;
	} VertexUpdateData;

	static PiSelectR PI_API _update_vertex(void *user_data, void *value)
	{
		VertexUpdateData *v_data = (VertexUpdateData *)user_data;
		D3D9RenderLayout *d3d9_layout = v_data->d3d9_layout;
		VertexElement *elem = (VertexElement *)value;

		if (elem->is_dirty)
		{
			v_data->is_vertex_declaration_dirty |= _renderlayout_set_vertex(d3d9_layout, elem->semantic, elem->usage, elem->type, elem->num, elem->data, elem->size);
			elem->is_dirty = FALSE;
		}
		return SELECT_NEXT;
	}

	PiBool PI_API render_layout_update(PiRenderMesh *mesh)
	{
		PiRenderData *data = &mesh->mesh->data;
		PiRenderLayout *layout = (PiRenderLayout *)mesh->gpu_data;

		if (layout == NULL)
		{
			mesh->gpu_data = layout = pi_renderlayout_new();
		}
		if (((D3D9RenderLayout*)layout->impl)->primitive_type == 0 && !data->is_dirty)
		{
			pi_log_print(LOG_WARNING, "rendermesh init failed");
		}
		if (data->is_dirty)
		{
			VertexUpdateData v_data;
			D3D9RenderLayout *d3d9_layout;

			d3d9_layout = (D3D9RenderLayout *)layout->impl;

			d3d9_layout->primitive_type = d3d9_primitive_type_get(data->type);

			if (data->idata.num > 0)
			{
				d3d9_layout->primitive_count = d3d9_indexed_primitive_count_get(data->type, data->idata.num);
			}
			else
			{
				d3d9_layout->primitive_count = d3d9_primitive_count_get(data->type, data->vertex_num);
			}

			d3d9_layout->num_vertices = data->vertex_num;

			if (data->idata.is_dirty)
			{
				_renderlayout_set_index(d3d9_layout, data->idata.usage, data->idata.type, data->idata.num, data->idata.data);
				data->idata.is_dirty = FALSE;
			}

			v_data.is_vertex_declaration_dirty = FALSE;
			v_data.d3d9_layout = d3d9_layout;

			pi_dhash_foreach(&data->vertex_map, _update_vertex, &v_data);

			if (v_data.is_vertex_declaration_dirty)
			{
				uint i, stream_index = 0;
				D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
				D3D9Context *context = d3d9_system->context;
				D3DVERTEXELEMENT9 decl[EVS_NUM + 1];
				if (d3d9_layout->vertex_declaration != NULL)
				{
					IDirect3DVertexDeclaration9_Release(d3d9_layout->vertex_declaration);
				}

				for (i = 0; i < EVS_NUM; ++i)
				{
					if (d3d9_layout->elements[i].vertex_buffer != NULL)
					{
						decl[stream_index] = d3d9_layout->elements[i].decl;
						stream_index++;
					}
				}

				decl[stream_index] = D3DDECL_END();

				IDirect3DDevice9_CreateVertexDeclaration(context->device, decl, &d3d9_layout->vertex_declaration);
			}

			data->is_dirty = FALSE;
		}

		return TRUE;
	}

	PiBool d3d9_renderlayout_draw(PiRenderMesh *mesh, PiRenderMesh* skinedMesh, uint num)
	{
		uint i;
		PiRenderLayout *layout = (PiRenderLayout *)mesh->gpu_data;
		D3D9RenderLayout *d3d9_layout = (D3D9RenderLayout *)layout->impl;
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9RenderState *d3d9_state = &d3d9_system->state;
		D3D9Context *context = d3d9_system->context;

		PiRenderLayout *skinedLayout = NULL;
		D3D9RenderLayout *skined_d3d9_layout = NULL;
		if (skinedMesh)
		{
			skinedLayout = (PiRenderLayout*)skinedMesh->gpu_data;
			skined_d3d9_layout = (D3D9RenderLayout *)skinedLayout->impl;
		}

		for (i = 0; i < EVS_NUM; ++i)
		{
			if ((i == EVS_POSITION || i == EVS_NORMAL) && skinedMesh != NULL)
			{
				if (d3d9_state->curr_stream_source[i] != skined_d3d9_layout->elements[i].vertex_buffer)
				{
					d3d9_state->curr_stream_source[i] = skined_d3d9_layout->elements[i].vertex_buffer;
					if (skined_d3d9_layout->elements[i].vertex_buffer != NULL)
					{
						IDirect3DDevice9_SetStreamSource(context->device, i, skined_d3d9_layout->elements[i].vertex_buffer, 0, skined_d3d9_layout->elements[i].stride);
					}
					else
					{
						IDirect3DDevice9_SetStreamSource(context->device, i, NULL, 0, 0);
					}
				}
			}
			else
			{
				if (d3d9_state->curr_stream_source[i] != d3d9_layout->elements[i].vertex_buffer)
				{
					d3d9_state->curr_stream_source[i] = d3d9_layout->elements[i].vertex_buffer;
					if (d3d9_layout->elements[i].vertex_buffer != NULL)
					{
						IDirect3DDevice9_SetStreamSource(context->device, i, d3d9_layout->elements[i].vertex_buffer, 0, d3d9_layout->elements[i].stride);
					}
					else
					{
						IDirect3DDevice9_SetStreamSource(context->device, i, NULL, 0, 0);
					}
				}
				if (num > 0)
				{
					if (i != EVS_INSTANCE)
					{
						IDirect3DDevice9_SetStreamSourceFreq(context->device, i, D3DSTREAMSOURCE_INDEXEDDATA | num);
					}
					else
					{
						IDirect3DDevice9_SetStreamSourceFreq(context->device, i, D3DSTREAMSOURCE_INSTANCEDATA | 1ul);
					}
				}
			}
		}

		if (d3d9_layout->vertex_declaration != d3d9_state->curr_vertex_declaration)
		{
			d3d9_state->curr_vertex_declaration = d3d9_layout->vertex_declaration;
			IDirect3DDevice9_SetVertexDeclaration(context->device, d3d9_layout->vertex_declaration);
		}
		
		if (d3d9_layout->d3d9_ib == NULL)
		{
			if (d3d9_layout->d3d9_ib != d3d9_state->curr_ib)
			{
				d3d9_state->curr_ib = d3d9_layout->d3d9_ib;
				IDirect3DDevice9_SetIndices(context->device, NULL);
			}

			IDirect3DDevice9_DrawPrimitive(context->device, d3d9_layout->primitive_type, 0, d3d9_layout->primitive_count);
		}
		else
		{
			if (d3d9_layout->d3d9_ib != d3d9_state->curr_ib)
			{
				d3d9_state->curr_ib = d3d9_layout->d3d9_ib;
				IDirect3DDevice9_SetIndices(context->device, d3d9_layout->d3d9_ib);
			}
			IDirect3DDevice9_DrawIndexedPrimitive(context->device, d3d9_layout->primitive_type, 0, 0, d3d9_layout->num_vertices, 0, d3d9_layout->primitive_count);
		}

		if (num > 0)
		{
			for (i = 0; i < EVS_NUM; ++i)
			{
				IDirect3DDevice9_SetStreamSourceFreq(context->device, i, 1);
			}
		}

		return TRUE;
	}

	PiBool PI_API render_layout_update_by_buffers(PiRenderMesh* mesh, EGeometryType type, const IndexData* index_data, uint num_verts, uint num_vert_buffers, VertexElement** vertex_buffers)
	{
		PiRenderLayout *layout = (PiRenderLayout *)mesh->gpu_data;
		D3D9IndexElement* index_buffer = (D3D9IndexElement*)index_data->impl;
		D3D9RenderLayout *d3d9_layout;
		uint i;
		if (layout == NULL)
		{
			mesh->gpu_data = layout = pi_renderlayout_new();
		}
		d3d9_layout = (D3D9RenderLayout*)layout->impl;
		d3d9_layout->primitive_type = d3d9_primitive_type_get(type);
		d3d9_layout->buffer_size = index_buffer->buffer_size;
		d3d9_layout->d3d9_ib = index_buffer->d3d9_ib;
		d3d9_layout->d3d9_index_format = index_buffer->d3d9_index_format;
		d3d9_layout->d3d9_index_lock_flags = index_buffer->d3d9_index_lock_flags;
		d3d9_layout->d3d9_index_pool = index_buffer->d3d9_index_pool;
		d3d9_layout->d3d9_index_usage = index_buffer->d3d9_index_usage;
		d3d9_layout->num_vertices = num_verts;
		if (index_data)
		{
		
			d3d9_layout->primitive_count = d3d9_indexed_primitive_count_get(type, index_buffer->index_num);
		}
		else
		{
			d3d9_layout->primitive_count = d3d9_primitive_count_get(type, num_verts);
		}
		for (i = 0; i < num_vert_buffers; ++i)
		{
			VertexElement* vertex_element = vertex_buffers[i];
			D3D9VertexElement* src = (D3D9VertexElement*)vertex_element->impl;
			D3D9VertexElement* dst = &d3d9_layout->elements[vertex_element->semantic];
			dst->buffer_size = src->buffer_size;
			dst->d3d9_lock_flags = src->d3d9_lock_flags;
			dst->d3d9_pool = src->d3d9_pool;
			dst->d3d9_usage = src->d3d9_usage;
			dst->decl = src->decl;
			dst->stride = src->stride;
			dst->vertex_buffer = src->vertex_buffer;
		}
		{
			uint i, stream_index = 0;
			D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
			D3D9Context *context = d3d9_system->context;
			D3DVERTEXELEMENT9 decl[EVS_NUM + 1];
			if (d3d9_layout->vertex_declaration != NULL)
			{
				IDirect3DVertexDeclaration9_Release(d3d9_layout->vertex_declaration);
			}

			for (i = 0; i < EVS_NUM; ++i)
			{
				if (d3d9_layout->elements[i].vertex_buffer != NULL)
				{
					decl[stream_index] = d3d9_layout->elements[i].decl;
					stream_index++;
				}
			}

			decl[stream_index] = D3DDECL_END();
			IDirect3DDevice9_CreateVertexDeclaration(context->device, decl, &d3d9_layout->vertex_declaration);
		}
		return TRUE;
	}

}
