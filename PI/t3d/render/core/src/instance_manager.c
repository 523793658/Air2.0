#include "instance_manager.h"
#include "instance_entity.h"

static InstanceCache* instance_cache = NULL;
static uint PI_API _hash_func(const void*  key)
{
	return (uint)key;
}


static PiBool PI_API _hash_equal(const void*		a, const void*		b)
{
	return a == b;
}

InstanceCache* PI_API pi_instance_manager_get_cache()
{
	if (instance_cache == NULL)
	{
		instance_cache = pi_new0(InstanceCache, 1);
		instance_cache->instance_list = pi_vector_new();
		instance_cache->instance_map = pi_hash_new(1.6, (PiHashFunc)_hash_func, (PiEqualFunc)_hash_equal);
	}
	return instance_cache;
}


uint PI_API pi_instance_manager_generate_id()
{
	static uint id = 0;
	return ++id;
}

InstanceManager* PI_API pi_instance_manager_new()
{
	InstanceManager* manager = pi_new0(InstanceManager, 1);
	manager->InstanceEntityMap = pi_hash_new(1.6, (PiHashFunc)_hash_func, (PiEqualFunc)_hash_equal);
	return manager;
}

PiSelectR PI_API _clear_mamanger(
	void*			user_data,
	void*			value)
{
	PiKeyValue* kv = (PiKeyValue*)value;
	pi_instance_entity_free(kv->value);
	pi_free(kv->key);
	return SELECT_NEXT;
}

void PI_API pi_instance_manager_free(InstanceManager* manager)
{
	pi_hash_foreach(manager->InstanceEntityMap, (PiSelectFunc)_clear_mamanger, NULL);
	pi_hash_free(manager->InstanceEntityMap);
}


InstanceEntity* PI_API pi_instance_manager_get_instance_entity(InstanceManager* manager, uint instanceId)
{
	InstanceEntity* instance;
	if (!instanceId)
	{
		return NULL;
	}
	if (!pi_hash_lookup(manager->InstanceEntityMap, (void*)instanceId, &instance))
	{
		instance = pi_instance_entity_create();
		instance->id = instanceId;
		pi_hash_insert(manager->InstanceEntityMap, (void*)instanceId, instance);
	}
	instance->reference_count++;
	return instance;
}

void PI_API pi_instance_manager_release_instance_entity(InstanceManager* manager, uint instanceId)
{
	InstanceEntity* instance;
	if (!instanceId)
	{
		return;
	}
	if (pi_hash_lookup(manager->InstanceEntityMap, (void*)instanceId, &instance))
	{
		instance->reference_count--;
		if (instance->reference_count == 0)
		{
			pi_hash_delete(manager->InstanceEntityMap, (void*)instanceId, NULL);
			pi_instance_entity_free(instance);
		}
	}
}
