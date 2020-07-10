#include <pi_renderdata.h>
#include <pi_mesh.h>

static void compute_aabb(PiRenderData *data)
{
	VertexElement elem;
	VertexElement *r = NULL;
	elem.semantic = EVS_POSITION;

	if(pi_dhash_lookup(&data->vertex_map, &elem, &r))
	{
		uint32 i;
		PiVector3 *pos = (PiVector3 *)r->data;
		pi_aabb_init(&data->box);
		for(i = 0; i < data->vertex_num; ++i)
		{
			pi_aabb_add_point(&data->box, &pos[i]);
		}
	}
}

static PiSelectR PI_API clear_vertex_element(void *user_data, void *value)
{
	VertexElement *elem = (VertexElement *)value;
	PI_USE_PARAM(user_data);

	if(elem->is_copy)
	{
		pi_free(elem->data);
	}
	return SELECT_NEXT;
}

static uint32 get_vertex_element_size(EVertexType type)
{
	uint32 size = 0;
	switch(type)
	{
	case EVT_BYTE:
	case EVT_UNSIGNED_BYTE:
		size = 1;
		break;
	case EVT_SHORT:
	case EVT_UNSIGNED_SHORT:
		size = 2;
		break;
	case EVT_INT:
	case EVT_UNSIGNED_INT:
		size = 4;
		break;
	case EVT_FLOAT:
		size = 4;
		break;
	case EVT_DOUBLE:
		size = 8;
		break;
	default:
		PI_ASSERT(FALSE, "type isn't valid, type = %d", type);
		break;
	}
	return size;
}

static PiSelectR PI_API copy_vertex_element(void *user_data, void *value)
{
	uint32 size;
	VertexElement elem;
	VertexElement *src = (VertexElement *)value;
	PiRenderData *dst = (PiRenderData *)user_data;

	elem.is_copy = TRUE;
	elem.is_dirty = TRUE;
	elem.num = src->num;
	elem.size = src->size;
	elem.type = src->type;
	elem.usage = src->usage;
	elem.semantic = src->semantic;
	size = elem.num * dst->vertex_num * get_vertex_element_size(src->type);
	if(src->data != NULL)
	{
		elem.data = pi_malloc(size);
		pi_memcpy_inline(elem.data, src->data, size);
	}
	pi_dhash_insert(&dst->vertex_map, &elem, NULL);

	return SELECT_NEXT;
}

static PiSelectR PI_API set_vertex_element(void *user_data, void *value)
{
	VertexElement elem;
	VertexElement *src = (VertexElement *)value;
	PiRenderData *dst = (PiRenderData *)user_data;

	elem.is_copy = FALSE;
	elem.data = src->data;
	elem.num = src->num;
	elem.size = src->size;
	elem.type = src->type;
	elem.usage = src->usage;
	elem.is_dirty = src->is_dirty;
	elem.semantic = src->semantic;
	pi_dhash_insert(&dst->vertex_map, &elem, NULL);
	return SELECT_NEXT;
}

static void PI_API set_idata(IndexData *dst, IndexData *src, PiBool is_copy)
{
	uint32 size;

	dst->is_dirty = TRUE;
	dst->num = src->num;
	dst->type = src->type;
	dst->is_copy = is_copy;
	dst->usage = src->usage;

	if(is_copy)
	{
		size = (dst->type == EINDEX_16BIT) ? 2 : 4;
		size *= dst->num;
		if(src->data != NULL)
		{
			dst->data = pi_malloc(size);
			pi_memcpy_inline(dst->data, src->data, size);
		}
		else
		{
			dst->data = NULL;                 
		}
	}
	else
	{
		dst->data = src->data;
	}
}

void PI_API pi_renderdata_init(PiRenderData *data, EGeometryType type)
{
	pi_memset_inline(data, 0, sizeof(PiRenderData));
	data->type = type;
	data->is_dirty = TRUE;
	pi_aabb_init(&data->box);
	pi_dhash_init(&data->vertex_map, sizeof(VertexElement), 0.75f, pi_kv_direct_hash, pi_kv_direct_equal);
}

void PI_API pi_renderdata_clear(PiRenderData *data)
{
	if(data->idata.is_copy)
	{
		pi_free(data->idata.data);
	}

	pi_aabb_clear(&data->box);
	pi_dhash_foreach (&data->vertex_map, clear_vertex_element, NULL);
	pi_dhash_clear(&data->vertex_map, TRUE);
}

PiRenderData* PI_API pi_renderdata_new(EGeometryType type)
{
	PiRenderData *data = pi_new(PiRenderData, 1);
	pi_renderdata_init(data, type);
	return data;
}

void PI_API pi_renderdata_free(PiRenderData *data)
{
	pi_renderdata_clear(data);
	pi_free(data);
}

void PI_API pi_renderdata_set(PiRenderData *dst, PiRenderData *src, PiBool is_copy)
{
	pi_renderdata_clear(dst);
	pi_renderdata_init(dst, src->type);

	dst->vertex_num = src->vertex_num;
	pi_aabb_copy(&dst->box, &src->box);

	set_idata(&dst->idata, &src->idata, is_copy);

	if(is_copy)
	{
		pi_dhash_foreach (&src->vertex_map, copy_vertex_element, dst);
	}
	else
	{
		pi_dhash_foreach (&src->vertex_map, set_vertex_element, dst);
	}
	dst->is_dirty = TRUE;
}

void PI_API pi_renderdata_set_vertex_num(PiRenderData *data, uint32 vertex_num)
{
	data->is_dirty = TRUE;
	data->vertex_num = vertex_num;
}


void PI_API pi_renderdata_set_index(PiRenderData *data, 
									PiBool is_copy, uint32 num, EIndexType type, EBufferUsage usage, void *index)
{
	uint32 new_size = num;
	IndexData *idata = &data->idata;
	uint32 old_size = idata->num;

	new_size *= (type == EINDEX_16BIT) ? 2 : 4;
	old_size *= (idata->type == EINDEX_16BIT) ? 2 : 4;

	/* 这段代码的逻辑，是根据现在要不要拷贝，和之前的数据要不要拷贝来实现 */
	if(!is_copy || index == NULL)
	{
		if(idata->is_copy)
		{/* 现在不需要拷贝，原来拷贝的数据需要释放掉 */
			pi_free(idata->data);
			idata->data = index;
		}
	}
	else
	{
		if(!idata->is_copy)
		{
			/* 现在需要拷贝，原来不需要拷贝，就要分配新的空间 */
			idata->data = pi_malloc(new_size);
		}
		else if(new_size != old_size)
		{/* 原来也是拷贝，但是新拷贝的大小和原来的不同，需要重新分配空间 */
			pi_free(idata->data);
			idata->data = pi_malloc(new_size);
		}

		/* 需要拷贝 */
		pi_memcpy_inline(idata->data, index, new_size);
	}

	idata->num = num;
	idata->type = type;
	idata->usage = usage;
	idata->is_dirty = TRUE;
	idata->is_copy = is_copy;
	data->is_dirty = TRUE;
}

void PI_API pi_renderdata_update_vertex(PiRenderData* data, uint32 offset, uint32_t vertex_num, VertexSemantic semantic, uint32_t srcStride, const void* src_data)
{
	VertexElement *r;
	VertexElement elem;
	pi_memset_inline(&elem, 0, sizeof(elem));
	elem.semantic = semantic;
	PI_ASSERT(vertex_num < data->vertex_num, "vertex's num is bigger then the init");
	if (!pi_dhash_lookup(&data->vertex_map, &elem, &r))
	{
		return;
	}
	data->is_dirty = TRUE;
	if (!r->is_copy)
	{
		void* oldData = r->data;
		r->data = pi_malloc(r->size);
		pi_memcpy_inline(r->data, oldData, r->size);
	}
	r->is_copy = TRUE;
	uint32_t dst_stride = r->num * get_vertex_element_size(r->type);
	void* dstData = (byte*)r->data + offset * dst_stride;
	for (uint32_t j = 0; j < vertex_num; j++)
	{
		pi_memcpy_inline(dstData, src_data, srcStride);
		dstData = ((byte*)dstData) + dst_stride;
		src_data = ((byte*)src_data) + srcStride;
	}

}

void PI_API pi_renderdata_set_vertex(PiRenderData *data, uint32 vertex_num,
									 PiBool is_copy, VertexSemantic semantic, uint32 num, EVertexType type, EBufferUsage usage, void *vertex)
{
	VertexElement *r;
	VertexElement elem;
	uint32 old_size = 0;
	uint32 new_size = 0;

	pi_memset_inline(&elem, 0, sizeof(elem));
	elem.semantic = semantic;

	PI_ASSERT(semantic == EVS_INSTANCE || vertex_num == data->vertex_num, "vertex's num isn't same to the init");

	if(!pi_dhash_lookup(&data->vertex_map, &elem, &r))
	{
		pi_dhash_insert(&data->vertex_map, &elem, NULL);
		pi_dhash_lookup(&data->vertex_map, &elem, &r);
	}

	data->is_dirty = TRUE;

	new_size = vertex_num * num * get_vertex_element_size(type);
	if(r->num > 0)
	{
		old_size = vertex_num * r->num * get_vertex_element_size(r->type);
	}

	/* 这段代码的逻辑，是根据现在要不要拷贝，和之前的数据要不要拷贝来实现 */
	if(!is_copy || vertex == NULL)
	{
		if(r->is_copy)
		{/* 现在不需要拷贝，原来拷贝的数据需要释放掉 */
			pi_free(r->data);
		}
		r->data = vertex;
	}
	else
	{
		if(!r->is_copy)
		{
			/* 现在需要拷贝，原来不需要拷贝，就要分配新的空间 */
			r->data = pi_malloc(new_size);
		}
		else if(new_size != old_size)
		{/* 原来也是拷贝，但是新拷贝的大小和原来的不同，需要重新分配空间 */
			pi_free(r->data);
			r->data = pi_malloc(new_size);
		}

		/* 需要拷贝 */
		pi_memcpy_inline(r->data, vertex, new_size);
	}
	if(vertex != NULL)
	{
		r->num = num;
		r->type = type;
		r->usage = usage;
		r->size = new_size;
		r->is_dirty = TRUE;
		r->is_copy = is_copy;
		r->semantic = semantic;
		if(semantic == EVS_POSITION && vertex != NULL)
		{
			compute_aabb(data);
		}
	}
	else
	{
		pi_dhash_delete(&data->vertex_map, &elem, NULL);		
	}
}

uint PI_API pi_get_vertex_type_size(EVertexType type, uint num)
{
	uint size = 0;
	switch (type)
	{
	case EVT_BYTE:
	case EVT_UNSIGNED_BYTE:
		size = 1;
		break;
	case EVT_SHORT:
	case EVT_UNSIGNED_SHORT:
		size = 2;
		break;
	case EVT_INT:
	case EVT_UNSIGNED_INT:
	case EVT_FLOAT:
		size = 4;
		break;
	case EVT_DOUBLE:
		size = 8;
		break;
	default:
		break;
	}
	return size * num;
}

VertexElement* pi_renderdata_get_vertex(PiRenderData *data, VertexSemantic semantic)
{
	VertexElement elem;
	VertexElement *r = NULL;
	elem.semantic = semantic;

	pi_dhash_lookup(&data->vertex_map, &elem, &r);
	return r;
}