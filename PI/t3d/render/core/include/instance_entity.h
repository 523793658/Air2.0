#ifndef _Instance_Entity_H_
#define _Instance_Entity_H_
#include "pi_lib.h"
#include "entity.h"
#define MAX_INSTANCE_NUM	30
typedef struct
{
	char* name;
	char* array_name;
	uint value_size;
	UniformType type;
}InstanceUniform;

typedef struct
{
	uint16 InstanceId;
	uint16 pading;
}InstanceData;

typedef struct  
{
	PiEntity* entity;			//”√”⁄‰÷»æ
	PiVector* private_uniforms;
	PiVector* public_uniforms;
	char* world_matrix_array_name;

	PiVector* subEntity;		//◊”engity
	uint reference_count;
	uint id;
	char uniform_buffer[MAX_INSTANCE_NUM * 4 * 4 * 4];

}InstanceEntity;



PI_BEGIN_DECLS

InstanceEntity* PI_API pi_instance_entity_create();

void* PI_API pi_instance_entity_get_instance_data();

void PI_API pi_instance_entity_set_render_entity(InstanceEntity* instance_entity, PiEntity* entity, PiVector* private_uniforms, PiVector* public_uniform, char* world_matrix_array_name);

void PI_API pi_instance_entity_draw(InstanceEntity* entity);

void PI_API pi_instance_entity_free(InstanceEntity* entity);

PI_END_DECLS
#endif