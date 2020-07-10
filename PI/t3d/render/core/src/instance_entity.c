#include "instance_entity.h"
#include "rendersystem.h"

extern PiRenderSystem *g_rsystem;

static InstanceData* instance_layout = NULL;

void PI_API pi_instance_entity_set_render_entity(InstanceEntity* instance_entity, PiEntity* entity, PiVector* private_uniforms, PiVector* public_uniforms, char* world_matrix_array_name)
{
	instance_entity->entity = entity;
	instance_entity->private_uniforms = private_uniforms;
	instance_entity->public_uniforms = public_uniforms;
	instance_entity->world_matrix_array_name = pi_str_dup(world_matrix_array_name);
}

void* PI_API pi_instance_entity_get_instance_data()
{
	if (instance_layout == NULL)
	{
		uint i;
		instance_layout = pi_new0(InstanceData, MAX_INSTANCE_NUM);
		for (i = 0; i < MAX_INSTANCE_NUM; i++)
		{
			instance_layout[i].InstanceId = (uint16)i;
		}
	}
	return instance_layout;
}

void PI_API pi_instance_entity_draw(InstanceEntity* instance)
{
	uint sub_entity_num, drawed_count = 0, instance_count;
	PiEntity* sub_entity;
	sub_entity_num = pi_vector_size(instance->subEntity);
	if (sub_entity_num == 0)
	{
		return;
	}
	if (sub_entity_num == 1)
	{
		sub_entity = pi_vector_get(instance->subEntity, 0);
		pi_entity_draw(sub_entity);
	}
	else
	{
		uint num;
		PiVector* uniforms_name;
		uint i, j;
		Uniform* uniform;
		uint size = sizeof(float) * 12;

		if (instance->public_uniforms)
		{
			sub_entity = pi_vector_get(instance->subEntity, 0);
			uniforms_name = instance->public_uniforms;
			num = pi_vector_size(uniforms_name);
			for (i = 0; i < num; i++)
			{
				InstanceUniform* instance_uniform = (InstanceUniform*)pi_vector_get(uniforms_name, i);
				uniform = pi_material_get_uniform(sub_entity->material, instance_uniform->name);
				pi_material_set_uniform(instance->entity->material, instance_uniform->array_name, instance_uniform->type, 1, uniform->value, FALSE);
			}
		}
		
		uniforms_name = instance->private_uniforms;
		num = uniforms_name == NULL ? 0 : pi_vector_size(uniforms_name);
		while (sub_entity_num > drawed_count)
		{
			instance_count = MIN(MAX_INSTANCE_NUM, sub_entity_num - drawed_count);
			for (i = 0; i < num; i++)
			{
				InstanceUniform* instance_uniform = (InstanceUniform*)pi_vector_get(uniforms_name, i);
				for (j = 0; j < instance_count; j++)
				{
					sub_entity = (PiEntity*)pi_vector_get(instance->subEntity, drawed_count + j);
					uniform = pi_material_get_uniform(sub_entity->material, instance_uniform->name);
					pi_memcpy_inline(instance->uniform_buffer + j * instance_uniform->value_size, uniform->value, instance_uniform->value_size);
				}
				pi_material_set_uniform(instance->entity->material, instance_uniform->array_name, instance_uniform->type, instance_count, instance->uniform_buffer, TRUE);
			}
			for (i = 0; i < instance_count; i++)
			{
				sub_entity = (PiEntity*)pi_vector_get(instance->subEntity, i + drawed_count);
				pi_memcpy_inline(instance->uniform_buffer + i * size, pi_entity_get_world_matrix(sub_entity), size);
			}
			pi_material_set_uniform_pack_flag(instance->entity->material, instance->world_matrix_array_name, UT_MATRIX4x3, instance_count, instance->uniform_buffer, FALSE, TRUE);
			pi_entity_draw_instance(instance->entity, instance_count);
			drawed_count += instance_count;
		}
	}
	pi_vector_clear(instance->subEntity, FALSE);
}

InstanceEntity* PI_API pi_instance_entity_create()
{
	InstanceEntity* instance = pi_new0(InstanceEntity, 1);
	instance->subEntity = pi_vector_new();
	return instance;
}

void PI_API pi_instance_entity_free(InstanceEntity* entity)
{
	uint i, size;
	if (entity->entity)
	{
		pi_material_free(entity->entity->material);
		pi_entity_free(entity->entity);
	}
	pi_vector_free(entity->subEntity);

	if (entity->private_uniforms)
	{
		size = pi_vector_size(entity->private_uniforms);
		for (i = 0; i < size; i++)
		{
			InstanceUniform* uniform = pi_vector_get(entity->private_uniforms, i);
			pi_free(uniform);
		}
		pi_vector_free(entity->private_uniforms);
	}

	if (entity->public_uniforms)
	{
		size = pi_vector_size(entity->public_uniforms);
		for (i = 0; i < size; i++)
		{
			InstanceUniform* uniform = pi_vector_get(entity->public_uniforms, i);
			pi_free(uniform);
		}
		pi_vector_free(entity->public_uniforms);
	}

	if (entity->world_matrix_array_name)
	{
		pi_free(entity->world_matrix_array_name);
	}
	
	pi_free(entity);
}