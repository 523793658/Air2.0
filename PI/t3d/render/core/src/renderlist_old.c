#include <renderlist.h>

#include "renderlistentry.h"

void PI_API pi_renderlist_init(PiRenderList *list, uint block)
{
	uint size = block + sizeof(RenderListEntry);

	pi_memset(list, 0, sizeof(PiRenderList));

	list->block = block;
	list->instance = pi_malloc0(size);
	pi_dvector_init(&list->entries, size);	
}

void PI_API pi_renderlist_clear(PiRenderList *list)
{
	pi_free(list->instance);
	pi_dvector_clear(&list->entries, TRUE);
	pi_memset(list, 0, sizeof(PiRenderList));
}

void PI_API pi_renderlist_add(PiRenderList *list, PiEntity *entity, void *data, uint state_env, uint shader_env)
{
	RenderListEntry *entry = list->instance;
	
	entry->entity = entity;
	entry->state_env = state_env;
	entry->shader_env = shader_env;
	if(list->block > 0)
	{
		pi_memcpy(entry->data, data, list->block);
	}
	pi_dvector_push(&list->entries, list->instance);
}

static PiBool PI_API _list_find_entity(const void *user_data, const void *data)
{
	return user_data == data;
}

void PI_API pi_renderlist_delete(PiRenderList *list, PiEntity *entity)
{
	pi_dvector_remove_if(&list->entries, _list_find_entity, entity);
}

void PI_API pi_renderlist_set_dirty(PiRenderList *list)
{
	list->dirty = TRUE;
}

void PI_API pi_renderlist_sort(PiRenderList *list)
{
	if(list->sort_func != NULL)
		pi_dvector_sort(&list->entries, list->sort_func);
}

void PI_API pi_renderlist_set_sort_func(PiRenderList *list, PiCompareFunc func)
{
	list->sort_func = func;
}