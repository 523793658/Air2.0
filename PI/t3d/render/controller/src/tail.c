#include "tail.h"

typedef struct
{
	PiVector3 pos;
	PiVector3 pos_t;
	float time;
} RecordData;

typedef struct
{
	uint sample_step;
	float width;
	float life_time;
	PiBool head_follow;
	PiQueue* record_cache;
	RecordData* record_data;
	RecordData head_data;
	PiVector3 pre_pos;
	ETailType type;

	PiSpatial* spatial;

	float* vertex_buffer;
	float* count_buffer;
	float* strength_buffer;
	float* direct_buffer;

	PiRenderMesh* mesh;

	float time;
	float last_time;

	uint foreach_index;
} Tail;

static void _refresh(PiController *c)
{
	uint i;
	PiMesh *mesh = NULL;
	Tail* impl = (Tail*)c->impl;
	
	pi_free(impl->count_buffer);
	pi_free(impl->direct_buffer);
	pi_free(impl->vertex_buffer);
	pi_free(impl->strength_buffer);
	pi_free(impl->record_data);
	if (impl->mesh != NULL)
	{
		pi_mesh_free(impl->mesh->mesh);
		pi_rendermesh_free(impl->mesh);
	}

	pi_queue_clear(impl->record_cache, FALSE);

	if(impl->type == ETT_LINE)
	{
		mesh = pi_mesh_create(EGOT_LINE_STRIP, EINDEX_32BIT, impl->sample_step + 1, NULL, NULL, NULL, NULL, 0, NULL);
		impl->vertex_buffer = pi_new0(float, (impl->sample_step + 1) * 3);
		impl->strength_buffer = pi_new0(float, impl->sample_step + 1);
		impl->record_data = pi_new0(RecordData, impl->sample_step);

		for(i = 0; i < impl->sample_step; i++)
		{
			pi_queue_push_tail(impl->record_cache, &impl->record_data[i]);
		}
	}
	else
	{
		mesh = pi_mesh_create(EGOT_TRIANGLE_STRIP, EINDEX_32BIT, (impl->sample_step + 1) * 2, NULL, NULL, NULL, NULL, 0, NULL);
		impl->vertex_buffer = pi_new0(float, (impl->sample_step + 1) * 2 * 3);
		impl->strength_buffer = pi_new0(float, (impl->sample_step + 1) * 2);
		impl->count_buffer = pi_new0(float, (impl->sample_step + 1) * 2);

		for(i = 0; i < (impl->sample_step + 1) * 2; i++)
		{
			impl->count_buffer[i] = (float)i;
		}

		pi_mesh_set_vertex(mesh, (impl->sample_step + 1) * 2, TRUE, EVS_TEXCOORD_1, 1, EVT_FLOAT, EVU_DYNAMIC_DRAW, impl->count_buffer);

		if(impl->type == ETT_RIBBON_FACING_CAMERA)
		{
			impl->direct_buffer = pi_new0(float, (impl->sample_step + 1) * 2 * 3);
		}

		impl->record_data = pi_new0(RecordData, impl->sample_step);

		for(i = 0; i < impl->sample_step; i++)
		{
			pi_queue_push_tail(impl->record_cache, &impl->record_data[i]);
		}
	}

	impl->mesh = pi_rendermesh_new(mesh, TRUE);
}

PiSelectR PI_API _update_line_foreach_func(void* user_data, void* value)
{
	Tail* impl = (Tail*)user_data;
	RecordData* data = (RecordData*)value;

	if(impl->type == ETT_LINE)
	{
		impl->vertex_buffer[impl->foreach_index * 3] = data->pos.x;
		impl->vertex_buffer[impl->foreach_index * 3 + 1] = data->pos.y;
		impl->vertex_buffer[impl->foreach_index * 3 + 2] = data->pos.z;
		impl->strength_buffer[impl->foreach_index] = 1.0f - (impl->time - data->time) / impl->life_time;
	}
	else if(impl->type == ETT_RIBBON)
	{
		impl->vertex_buffer[impl->foreach_index * 6] = data->pos.x;
		impl->vertex_buffer[impl->foreach_index * 6 + 1] = data->pos.y;
		impl->vertex_buffer[impl->foreach_index * 6 + 2] = data->pos.z;
		impl->vertex_buffer[impl->foreach_index * 6 + 3] = data->pos_t.x;
		impl->vertex_buffer[impl->foreach_index * 6 + 4] = data->pos_t.y;
		impl->vertex_buffer[impl->foreach_index * 6 + 5] = data->pos_t.z;
		impl->strength_buffer[impl->foreach_index * 2] = data->time;
		impl->strength_buffer[impl->foreach_index * 2] = impl->strength_buffer[impl->foreach_index * 2 + 1] = 1.0f - (impl->time - data->time) / impl->life_time;
	}

	if(impl->type == ETT_RIBBON_FACING_CAMERA)
	{
		uint pre_index = impl->foreach_index - 1;
		RecordData* pre_data;

		if(pre_index == 0)
		{
			pre_data = &impl->head_data;
		}
		else
		{
			pre_data = (RecordData*)pi_queue_get(impl->record_cache, pre_index - 1);
		}

		impl->vertex_buffer[impl->foreach_index * 6] = impl->vertex_buffer[impl->foreach_index * 6 + 3] = data->pos.x;
		impl->vertex_buffer[impl->foreach_index * 6 + 1] = impl->vertex_buffer[impl->foreach_index * 6 + 4] = data->pos.y;
		impl->vertex_buffer[impl->foreach_index * 6 + 2] = impl->vertex_buffer[impl->foreach_index * 6 + 5] = data->pos.z;

		impl->direct_buffer[impl->foreach_index * 6] = impl->direct_buffer[impl->foreach_index * 6 + 3] = pre_data->pos.x - data->pos.x;
		impl->direct_buffer[impl->foreach_index * 6 + 1] = impl->direct_buffer[impl->foreach_index * 6 + 4]  = pre_data->pos.y - data->pos.y;
		impl->direct_buffer[impl->foreach_index * 6 + 2] = impl->direct_buffer[impl->foreach_index * 6 + 5] = pre_data->pos.z - data->pos.z;

		if(pi_vec3_len_square((PiVector3*)&impl->direct_buffer[impl->foreach_index * 6]) == 0)
		{
			impl->direct_buffer[impl->foreach_index * 6] = impl->direct_buffer[impl->foreach_index * 6 + 3] = impl->direct_buffer[(impl->foreach_index - 1) * 6];
			impl->direct_buffer[impl->foreach_index * 6 + 1] = impl->direct_buffer[impl->foreach_index * 6 + 4] = impl->direct_buffer[(impl->foreach_index - 1) * 6 + 1];
			impl->direct_buffer[impl->foreach_index * 6 + 2] = impl->direct_buffer[impl->foreach_index * 6 + 5] = impl->direct_buffer[(impl->foreach_index - 1) * 6 + 2];
		}

		impl->strength_buffer[impl->foreach_index * 2] = impl->strength_buffer[impl->foreach_index * 2 + 1] = 1.0f - (impl->time - data->time) / impl->life_time;
	}

	impl->foreach_index++;
	return SELECT_NEXT;
}

static PiBool _update(struct PiController *c, float tpf)
{
	Tail* impl = (Tail*)c->impl;
	float period = impl->life_time / impl->sample_step;
	RecordData* data;
	impl->time += tpf;

	if(impl->type == ETT_LINE)
	{
		if(impl->head_follow)
		{
			pi_vec3_copy(&impl->head_data.pos, pi_spatial_get_world_translation(impl->spatial));
		}
		else
		{
			pi_vec3_copy(&impl->head_data.pos, pi_spatial_get_local_translation(impl->spatial));
		}

		impl->head_data.time = impl->time;

		if((impl->time - impl->last_time) > period)
		{
			data = (RecordData*)pi_queue_pop_tail(impl->record_cache);
			pi_vec3_copy(&data->pos, &impl->head_data.pos);
			data->time = impl->time;
			pi_queue_push_head(impl->record_cache, data);
			impl->last_time = impl->time;
		}

		impl->vertex_buffer[0] = impl->head_data.pos.x;
		impl->vertex_buffer[1] = impl->head_data.pos.y;
		impl->vertex_buffer[2] = impl->head_data.pos.z;
		impl->strength_buffer[0] = 1;
		impl->foreach_index = 1;
		pi_queue_foreach(impl->record_cache, _update_line_foreach_func, impl);

		pi_mesh_set_vertex(impl->mesh->mesh, impl->sample_step + 1, TRUE, EVS_POSITION, 3, EVT_FLOAT, EVU_DYNAMIC_DRAW, impl->vertex_buffer);
		pi_mesh_set_vertex(impl->mesh->mesh, impl->sample_step + 1, TRUE, EVS_TEXCOORD_3, 1, EVT_FLOAT, EVU_DYNAMIC_DRAW, impl->strength_buffer);
	}
	else
	{
		PiMatrix4* transform;
		float y_offset = 0;

		if(impl->head_follow)
		{
			transform = pi_spatial_get_world_transform(impl->spatial);
		}
		else
		{
			transform = pi_spatial_get_local_transform(impl->spatial);
		}

		if(impl->type == ETT_RIBBON)
		{
			y_offset = impl->width / 2.0f;
		}

		impl->head_data.time = impl->time;
		pi_vec3_set(&impl->pre_pos, impl->head_data.pos.x, impl->head_data.pos.y, impl->head_data.pos.z);
		pi_vec3_set(&impl->head_data.pos, 0, -y_offset, 0);
		pi_vec3_set(&impl->head_data.pos_t, 0, y_offset, 0);
		pi_mat4_apply_point(&impl->head_data.pos, &impl->head_data.pos, transform);
		pi_mat4_apply_point(&impl->head_data.pos_t, &impl->head_data.pos_t, transform);

		if((impl->time - impl->last_time) > period)
		{
			data = (RecordData*)pi_queue_pop_tail(impl->record_cache);
			pi_vec3_copy(&data->pos_t, &impl->head_data.pos_t);
			pi_vec3_copy(&data->pos, &impl->head_data.pos);
			data->time = impl->time;
			pi_queue_push_head(impl->record_cache, data);
			impl->last_time = impl->time;
		}

		impl->vertex_buffer[0] = impl->head_data.pos.x;
		impl->vertex_buffer[1] = impl->head_data.pos.y;
		impl->vertex_buffer[2] = impl->head_data.pos.z;
		impl->vertex_buffer[3] = impl->head_data.pos_t.x;
		impl->vertex_buffer[4] = impl->head_data.pos_t.y;
		impl->vertex_buffer[5] = impl->head_data.pos_t.z;
		impl->strength_buffer[0] = 1.0f;
		impl->strength_buffer[1] = 1.0f;

		if(impl->type == ETT_RIBBON_FACING_CAMERA)
		{
			impl->direct_buffer[0] = impl->direct_buffer[3] = impl->head_data.pos.x - impl->pre_pos.x;
			impl->direct_buffer[1] = impl->direct_buffer[4] = impl->head_data.pos.y - impl->pre_pos.y;
			impl->direct_buffer[2] = impl->direct_buffer[5] = impl->head_data.pos.z - impl->pre_pos.z;
		}

		impl->foreach_index = 1;
		pi_queue_foreach(impl->record_cache, _update_line_foreach_func, impl);

		pi_mesh_set_vertex(impl->mesh->mesh, (impl->sample_step + 1) * 2, TRUE, EVS_POSITION, 3, EVT_FLOAT, EVU_DYNAMIC_DRAW, impl->vertex_buffer);
		pi_mesh_set_vertex(impl->mesh->mesh, (impl->sample_step + 1) * 2, TRUE, EVS_TEXCOORD_3, 1, EVT_FLOAT, EVU_DYNAMIC_DRAW, impl->strength_buffer);

		if(impl->type == ETT_RIBBON_FACING_CAMERA)
		{
			pi_mesh_set_vertex(impl->mesh->mesh, (impl->sample_step + 1) * 2, TRUE, EVS_TEXCOORD_2, 3, EVT_FLOAT, EVU_DYNAMIC_DRAW, impl->direct_buffer);
		}

		pi_rendermesh_update(impl->mesh);
	}

	pi_rendermesh_update(impl->mesh);

	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	Tail *impl = (Tail*)c->impl;
	PiEntity* entity = (PiEntity*)obj;

	pi_entity_set_mesh(entity, impl->mesh);

	return TRUE;
}

PiController* PI_API pi_tail_new(ETailType type, uint sample_step, float width, float life_time, PiBool head_follow)
{
	Tail *impl = pi_new0(Tail, 1);
	PiController *c = pi_controller_new(CT_TAIL, _apply, _update, impl);

	impl->sample_step = sample_step;
	impl->head_follow = head_follow;
	impl->life_time = life_time;
	impl->width = width;
	impl->record_cache = pi_queue_new();
	impl->type = type;
	impl->mesh = NULL;

	_refresh(c);

	return c;
}

void PI_API pi_tail_free(PiController *c)
{
	Tail *impl = (Tail*)c->impl;

	pi_free(impl->record_data);
	pi_queue_free(impl->record_cache);
	pi_free(impl->vertex_buffer);
	pi_free(impl->count_buffer);
	pi_free(impl->direct_buffer);
	pi_free(impl->strength_buffer);
	if (impl->mesh != NULL)
	{ 
		pi_mesh_free(impl->mesh->mesh);
		pi_rendermesh_free(impl->mesh);
	}
	
	pi_free(impl);
	pi_controller_free(c);
}

void PI_API pi_tail_set_spatial(PiController *c, PiSpatial* spatial)
{
	Tail *impl = (Tail*)c->impl;

	impl->spatial = spatial;
}

PiSelectR PI_API _reset_foreach_func(void* user_data, void* value)
{
	Tail *impl = (Tail*)user_data;
	RecordData* data = (RecordData*)value;

	if(impl->type != ETT_RIBBON)
	{
		if(impl->head_follow)
		{
			data->pos.x = data->pos_t.x = impl->spatial->world_translation.x;
			data->pos.y = data->pos_t.y = impl->spatial->world_translation.y;
			data->pos.z = data->pos_t.z = impl->spatial->world_translation.z;
		}
		else
		{
			data->pos.x = data->pos_t.x = impl->spatial->local_translation.x;
			data->pos.y = data->pos_t.y = impl->spatial->local_translation.y;
			data->pos.z = data->pos_t.z = impl->spatial->local_translation.z;
		}
	}
	else
	{
		float y_offset = impl->width / 2.0f;
		PiMatrix4* transform;

		if(impl->head_follow)
		{
			transform = pi_spatial_get_world_transform(impl->spatial);
		}
		else
		{
			transform = pi_spatial_get_local_transform(impl->spatial);
		}

		pi_vec3_set(&data->pos, 0, -y_offset, 0);
		pi_vec3_set(&data->pos_t, 0, y_offset, 0);
		pi_mat4_apply_point(&data->pos, &data->pos, transform);
		pi_mat4_apply_point(&data->pos_t, &data->pos_t, transform);
	}

	data->time = 0;

	return SELECT_NEXT;
}

void PI_API pi_tail_reset(PiController *c)
{
	Tail *impl = (Tail*)c->impl;

	impl->time = 0;
	impl->last_time = 0;
	pi_queue_foreach(impl->record_cache, _reset_foreach_func, impl);
}

