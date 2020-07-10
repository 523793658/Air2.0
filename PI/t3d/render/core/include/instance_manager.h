#ifndef _InstanceManager_H_
#define _InstanceManager_H_
#include "pi_lib.h"
#include "instance_entity.h"

typedef struct
{
	PiVector* instance_list;
	PiHash* instance_map;
}InstanceCache;

typedef struct
{
	PiHash* InstanceEntityMap;
}InstanceManager;


PI_BEGIN_DECLS

InstanceManager* PI_API pi_instance_manager_new();

void PI_API pi_instance_manager_free(InstanceManager* manager);

uint PI_API pi_instance_manager_generate_id();

InstanceEntity* PI_API pi_instance_manager_get_instance_entity(InstanceManager* manager, uint instanceId);

void PI_API pi_instance_manager_release_instance_entity(InstanceManager* manager, uint instanceId);

InstanceCache* PI_API pi_instance_manager_get_cache();

PI_END_DECLS










#endif