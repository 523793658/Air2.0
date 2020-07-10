#ifndef _RenderSceneLinker_H_
#define _RenderSceneLinker_H_
#include "pi_lib.h"
#include "entity.h"
typedef enum{
	ROT0,
	ROT1,
	ROT2,
	ROT3,
	ROT4,
	ROT5,
	ROT6,
	ROT7,
	ROT8,
	ROT9,
	ROT10,
	ROT11,
	ROT12,
	ROT13,
	ROT14,
	ROT15,
	ROT16,
	ROT17,
	ROT18,
	ROT19,
	ROT_NUM
}RenderObjType;

typedef struct  
{
	PiVector* render_lists[ROT_NUM];
}LinkerManager;


typedef struct  
{
	int contain_type_num;
	RenderObjType contain_type[ROT_NUM];
	PiVector* entity_lists[ROT_NUM];
}RenderSceneLinker;

PI_BEGIN_DECLS

RenderSceneLinker* pi_render_scene_linker_new();

void pi_render_scene_linker_add_link_obj(RenderSceneLinker* linker, void* entity, RenderObjType type);

void pi_render_scene_linker_remove_link_obj(RenderSceneLinker* linker, void* entity, RenderObjType type);




LinkerManager* pi_linker_manager_new();

void pi_linker_manager_free(LinkerManager* manager);

void pi_linker_manager_add_list(LinkerManager* manager, PiVector* list, RenderObjType type);

void pi_linker_manager_dispatch(LinkerManager* manager, PiVector* list, uint mask);

PI_END_DECLS
#endif