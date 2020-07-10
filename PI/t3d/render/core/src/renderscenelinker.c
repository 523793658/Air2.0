#include "renderscenelinker.h"

RenderSceneLinker* pi_render_scene_linker_new()
{
	return pi_new0(RenderSceneLinker, 1);
}

void pi_render_scene_linker_add_link_obj(RenderSceneLinker* linker, PiEntity* entity, RenderObjType type)
{
	if (linker->entity_lists[type] == NULL)
	{
		linker->entity_lists[type] = pi_vector_new();
		linker->contain_type[linker->contain_type_num] = type;
		linker->contain_type_num++;
	}
	pi_vector_push(linker->entity_lists[type], entity);
}

void pi_render_scene_linker_remove_link_obj(RenderSceneLinker* linker, PiEntity* entity, RenderObjType type)
{
	uint size = pi_vector_size(linker->entity_lists[type]);
	uint i;
	for (i = 0; i < size; i++)
	{
		if (pi_vector_get(linker->entity_lists[type], i) == entity)
		{
			pi_vector_remove(linker->entity_lists[type], i);
			return;
		}
	}
}

void pi_render_scene_linker_free(RenderSceneLinker* linker)
{
	uint i;
	for (i = 0; i < linker->contain_type_num; ++i)
	{
		pi_vector_free(linker->entity_lists[linker->contain_type[i]]);
	}
	pi_free(linker);
}

LinkerManager* pi_linker_manager_new()
{
	return pi_new0(LinkerManager, 1);
}

void pi_linker_manager_add_list(LinkerManager* manager, PiVector* list, RenderObjType type)
{
	manager->render_lists[type] = list;
}