#include "chain.h"

typedef struct
{
	EChainFacingType type;

	float time;
	float width;
	uint num_step;
	float step_time;
	float step_interval;
	PiVector *points;
	PiVector *points_offsets;
	PiVector *chain_heads;
	PiVector3 *chain_heads_data;
	PiRenderMesh *mesh;

	PiBool mesh_update;

	float *vertex_buffer;
	float *texcoord_buffer;
	uint *index_buffer;
	float *direct_buffer;
} Chain;

static void _refresh(PiController *c)
{
	uint i, j;
	PiMesh *mesh = NULL;
	Chain *impl = (Chain *)c->impl;
	uint cross_multiples = impl->type == ECFT_CROSS ? 2 : 1;

	if (impl->mesh_update)
	{
		if (impl->mesh != NULL)
		{
			pi_mesh_free(impl->mesh->mesh);
			pi_rendermesh_free(impl->mesh);
		}

		pi_free(impl->vertex_buffer);
		pi_free(impl->texcoord_buffer);
		pi_free(impl->index_buffer);
		pi_free(impl->chain_heads_data);
		impl->vertex_buffer = pi_new0(float, (impl->num_step - 1) * 4 * 3 * cross_multiples);
		impl->texcoord_buffer = pi_new0(float, (impl->num_step - 1) * 4 * 2 * cross_multiples);
		impl->index_buffer = pi_new0(uint, (impl->num_step - 1) * 6 * cross_multiples);
		impl->chain_heads_data = pi_new0(PiVector3, impl->num_step - 1);

		if (impl->type == ECFT_FACING_CAMERA)
		{
			pi_free(impl->direct_buffer);
			impl->direct_buffer = pi_new0(float, (impl->num_step - 1) * 4 * 3);
		}

		pi_vector_clear(impl->chain_heads, FALSE);

		for (i = 0; i < impl->num_step - 1; i++)
		{
			pi_vector_push(impl->chain_heads, &impl->chain_heads_data[i]);
			impl->index_buffer[i * 6] = i * 4;
			impl->index_buffer[i * 6 + 1] = i * 4 + 3;
			impl->index_buffer[i * 6 + 2] = i * 4 + 1;
			impl->index_buffer[i * 6 + 3] = i * 4;
			impl->index_buffer[i * 6 + 4] = i * 4 + 2;
			impl->index_buffer[i * 6 + 5] = i * 4 + 3;

			impl->texcoord_buffer[i * 8] = 0;
			impl->texcoord_buffer[i * 8 + 1] = 1;
			impl->texcoord_buffer[i * 8 + 2] = 0;
			impl->texcoord_buffer[i * 8 + 3] = 0;
			impl->texcoord_buffer[i * 8 + 4] = 1;
			impl->texcoord_buffer[i * 8 + 5] = 1;
			impl->texcoord_buffer[i * 8 + 6] = 1;
			impl->texcoord_buffer[i * 8 + 7] = 0;

			if (impl->type == ECFT_CROSS)
			{
				j = i + impl->num_step - 1;
				impl->index_buffer[j * 6] = j * 4;
				impl->index_buffer[j * 6 + 1] = j * 4 + 3;
				impl->index_buffer[j * 6 + 2] = j * 4 + 1;
				impl->index_buffer[j * 6 + 3] = j * 4;
				impl->index_buffer[j * 6 + 4] = j * 4 + 2;
				impl->index_buffer[j * 6 + 5] = j * 4 + 3;

				impl->texcoord_buffer[j * 8] = 0;
				impl->texcoord_buffer[j * 8 + 1] = 1;
				impl->texcoord_buffer[j * 8 + 2] = 0;
				impl->texcoord_buffer[j * 8 + 3] = 0;
				impl->texcoord_buffer[j * 8 + 4] = 1;
				impl->texcoord_buffer[j * 8 + 5] = 1;
				impl->texcoord_buffer[j * 8 + 6] = 1;
				impl->texcoord_buffer[j * 8 + 7] = 0;
			}
		}

		mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, (impl->num_step - 1) * 4 * cross_multiples, NULL, NULL, NULL, impl->texcoord_buffer, (impl->num_step - 1) * 6 * cross_multiples, impl->index_buffer);
		impl->mesh = pi_rendermesh_new(mesh, TRUE);

		impl->mesh_update = FALSE;
	}
}

static void _update_mesh_data(Chain *c)
{
	uint i, curr_index, step;
	PiVector3 *current, *current_offset, *next, *next_offset, *chain_head, displacement, displacement_offset;
	float process;

	step = c->num_step - 1;
	process = MIN((c->time + c->step_interval) / ((c->step_time + c->step_interval) * step), 1.0f);
	curr_index = (uint)(step * process);

	process = MAX((c->time - (c->step_time + c->step_interval) * curr_index), 0) / c->step_time;

	for (i = 0; i < step; i++)
	{
		current = (PiVector3 *)pi_vector_get(c->points, i);
		current_offset = (PiVector3 *)pi_vector_get(c->points_offsets, i);
		next = (PiVector3 *)pi_vector_get(c->points, i + 1);
		next_offset = (PiVector3 *)pi_vector_get(c->points_offsets, i + 1);

		chain_head = (PiVector3 *)pi_vector_get(c->chain_heads, i);

		if (i < curr_index)
		{
			pi_vec3_copy(chain_head, next);
			pi_vec3_add(chain_head, chain_head, next_offset);
		}
		else if (i > curr_index)
		{
			pi_vec3_copy(chain_head, current);
			pi_vec3_add(chain_head, chain_head, current_offset);
		}
		else
		{
			pi_vec3_sub(&displacement, next, current);
			displacement.x *= process;
			displacement.y *= process;
			displacement.z *= process;
			pi_vec3_sub(&displacement_offset, next_offset, current_offset);
			displacement_offset.x *= process;
			displacement_offset.y *= process;
			displacement_offset.z *= process;
			pi_vec3_copy(chain_head, current);
			pi_vec3_add(chain_head, &displacement, chain_head);
			pi_vec3_add(chain_head, chain_head, &displacement_offset);
		}
	}
}

static void _generate_mesh(Chain *c)
{
	uint i, j;
	PiVector3 *current, *chain_head, dir, left, up;
	PiBool ignore;
	uint cross_multiples = c->type == ECFT_CROSS ? 2 : 1;

	if (c->type != ECFT_HORIZONTAL)
	{
		pi_vec3_set(&up, 0, 0, 1);
	}
	else
	{
		pi_vec3_set(&up, 0, 1, 0);
	}

	for (i = 0; i < c->num_step - 1; i++)
	{
		current = (PiVector3 *)pi_vector_get(c->points, i);
		chain_head = (PiVector3 *)pi_vector_get(c->chain_heads, i);
		ignore = pi_vec3_is_equal(chain_head, current);

		if (!ignore)
		{
			if (c->type != ECFT_FACING_CAMERA)
			{
				pi_vec3_sub(&dir, chain_head, current);
				pi_vec3_normalise(&dir, &dir);
				pi_vec3_cross(&left, &dir, &up);
				pi_vec3_normalise(&left, &left);
				c->vertex_buffer[i * 4 * 3] = current->x + left.x * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 1] = current->y + left.y * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 2] = current->z + left.z * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 3] = current->x - left.x * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 4] = current->y - left.y * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 5] = current->z - left.z * c->width / 2.0f;

				c->vertex_buffer[i * 4 * 3 + 6] = chain_head->x + left.x * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 7] = chain_head->y + left.y * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 8] = chain_head->z + left.z * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 9] = chain_head->x - left.x * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 10] = chain_head->y - left.y * c->width / 2.0f;
				c->vertex_buffer[i * 4 * 3 + 11] = chain_head->z - left.z * c->width / 2.0f;

				if (c->type == ECFT_CROSS)
				{
					j = i + c->num_step - 1;
					pi_vec3_set(&up, up.x, up.z, up.y);
					pi_vec3_cross(&left, &dir, &up);
					pi_vec3_normalise(&left, &left);

					c->vertex_buffer[j * 4 * 3] = current->x + left.x * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 1] = current->y + left.y * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 2] = current->z + left.z * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 3] = current->x - left.x * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 4] = current->y - left.y * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 5] = current->z - left.z * c->width / 2.0f;

					c->vertex_buffer[j * 4 * 3 + 6] = chain_head->x + left.x * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 7] = chain_head->y + left.y * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 8] = chain_head->z + left.z * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 9] = chain_head->x - left.x * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 10] = chain_head->y - left.y * c->width / 2.0f;
					c->vertex_buffer[j * 4 * 3 + 11] = chain_head->z - left.z * c->width / 2.0f;
				}
			}
			else
			{
				pi_vec3_sub(&dir, chain_head, current);
				pi_vec3_normalise(&dir, &dir);

				c->vertex_buffer[i * 4 * 3] = current->x;
				c->vertex_buffer[i * 4 * 3 + 1] = current->y;
				c->vertex_buffer[i * 4 * 3 + 2] = current->z;
				c->vertex_buffer[i * 4 * 3 + 3] = current->x;
				c->vertex_buffer[i * 4 * 3 + 4] = current->y;
				c->vertex_buffer[i * 4 * 3 + 5] = current->z;

				c->vertex_buffer[i * 4 * 3 + 6] = chain_head->x;
				c->vertex_buffer[i * 4 * 3 + 7] = chain_head->y;
				c->vertex_buffer[i * 4 * 3 + 8] = chain_head->z;
				c->vertex_buffer[i * 4 * 3 + 9] = chain_head->x;
				c->vertex_buffer[i * 4 * 3 + 10] = chain_head->y;
				c->vertex_buffer[i * 4 * 3 + 11] = chain_head->z;

				c->direct_buffer[i * 4 * 3] = dir.x;
				c->direct_buffer[i * 4 * 3 + 1] = dir.y;
				c->direct_buffer[i * 4 * 3 + 2] = dir.z;
				c->direct_buffer[i * 4 * 3 + 3] = dir.x;
				c->direct_buffer[i * 4 * 3 + 4] = dir.y;
				c->direct_buffer[i * 4 * 3 + 5] = dir.z;

				c->direct_buffer[i * 4 * 3 + 6] = dir.x;
				c->direct_buffer[i * 4 * 3 + 7] = dir.y;
				c->direct_buffer[i * 4 * 3 + 8] = dir.z;
				c->direct_buffer[i * 4 * 3 + 9] = dir.x;
				c->direct_buffer[i * 4 * 3 + 10] = dir.y;
				c->direct_buffer[i * 4 * 3 + 11] = dir.z;
			}
		}
		else
		{
			c->vertex_buffer[i * 4 * 3] = current->x;
			c->vertex_buffer[i * 4 * 3 + 1] = current->y;
			c->vertex_buffer[i * 4 * 3 + 2] = current->z;
			c->vertex_buffer[i * 4 * 3 + 3] = current->x;
			c->vertex_buffer[i * 4 * 3 + 4] = current->y;
			c->vertex_buffer[i * 4 * 3 + 5] = current->z;

			c->vertex_buffer[i * 4 * 3 + 6] = current->x;
			c->vertex_buffer[i * 4 * 3 + 7] = current->y;
			c->vertex_buffer[i * 4 * 3 + 8] = current->z;
			c->vertex_buffer[i * 4 * 3 + 9] = current->x;
			c->vertex_buffer[i * 4 * 3 + 10] = current->y;
			c->vertex_buffer[i * 4 * 3 + 11] = current->z;

			if (c->type == ECFT_CROSS)
			{
				j = i + c->num_step - 1;
				c->vertex_buffer[j * 4 * 3] = current->x;
				c->vertex_buffer[j * 4 * 3 + 1] = current->y;
				c->vertex_buffer[j * 4 * 3 + 2] = current->z;
				c->vertex_buffer[j * 4 * 3 + 3] = current->x;
				c->vertex_buffer[j * 4 * 3 + 4] = current->y;
				c->vertex_buffer[j * 4 * 3 + 5] = current->z;

				c->vertex_buffer[j * 4 * 3 + 6] = current->x;
				c->vertex_buffer[j * 4 * 3 + 7] = current->y;
				c->vertex_buffer[j * 4 * 3 + 8] = current->z;
				c->vertex_buffer[j * 4 * 3 + 9] = current->x;
				c->vertex_buffer[j * 4 * 3 + 10] = current->y;
				c->vertex_buffer[j * 4 * 3 + 11] = current->z;
			}
		}
	}

	pi_mesh_set_vertex(c->mesh->mesh, (c->num_step - 1) * 4 * cross_multiples, TRUE, EVS_POSITION, 3, EVT_FLOAT, EVU_DYNAMIC_DRAW, c->vertex_buffer);

	if (c->type == ECFT_FACING_CAMERA)
	{
		pi_mesh_set_vertex(c->mesh->mesh, (c->num_step - 1) * 4, TRUE, EVS_TEXCOORD_2, 3, EVT_FLOAT, EVU_DYNAMIC_DRAW, c->direct_buffer);
	}

	pi_rendermesh_update(c->mesh);
}

static PiBool _update(PiController *c, float tpf)
{
	Chain *impl = (Chain *)c->impl;

	impl->time += tpf;

	PI_ASSERT(pi_vector_size(impl->points) == impl->num_step, "Number of step points must be same as specified on reset.");

	_update_mesh_data(impl);

	_generate_mesh(impl);

	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	Chain *impl = (Chain *)c->impl;
	PiEntity *entity = (PiEntity *)obj;
	pi_entity_set_mesh(entity, impl->mesh);
	return TRUE;
}

PiController *PI_API pi_chain_new(EChainFacingType type)
{
	Chain *impl = pi_new0(Chain, 1);
	PiController *c = pi_controller_new(CT_CHAIN, _apply, _update, impl);

	impl->width = 0.3f;
	impl->num_step = 2;
	impl->step_time = 1;
	impl->mesh = NULL;
	impl->type = type;
	impl->chain_heads = pi_vector_new();
	impl->mesh_update = TRUE;

	return c;
}

void PI_API pi_chain_free(PiController *c)
{
	Chain *impl = (Chain *)c->impl;

	if (impl->mesh != NULL)
	{
		pi_mesh_free(impl->mesh->mesh);
		pi_rendermesh_free(impl->mesh);
	}
	pi_free(impl->vertex_buffer);
	pi_free(impl->texcoord_buffer);
	pi_free(impl->index_buffer);
	pi_free(impl->chain_heads_data);
	pi_free(impl->direct_buffer);
	pi_vector_free(impl->chain_heads);
	pi_free(impl);
	pi_controller_free(c);
}

void PI_API pi_chain_set_step_points(PiController *c, PiVector *points, PiVector *offsets)
{
	Chain *impl = (Chain *)c->impl;

	impl->points = points;
	impl->points_offsets = offsets;
}

void PI_API pi_chain_reset(PiController *c, uint num_step, float step_time, float step_interval)
{
	Chain *impl = (Chain *)c->impl;

	if (num_step != impl->num_step)
	{
		impl->mesh_update = TRUE;
		impl->num_step = num_step;
	}

	impl->step_time = step_time;
	impl->step_interval = step_interval;
	impl->time = 0;

	_refresh(c);
}

void PI_API pi_chain_set_width(PiController *c, float width)
{
	Chain *impl = (Chain *)c->impl;

	impl->width = width;
}