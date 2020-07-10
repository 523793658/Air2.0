#ifndef _RenderScene_H_
#define _RenderScene_H_
#include "pi_lib.h"
#include "s3d.h"
#include "renderpipeline_ext.h"
#include "renderlist.h"
#include "controllermanager.h"
#include "particle_manager.h"
#include "pi_spatial.h"

#define MAX_GROUP_NUM 10

typedef struct{
	PiVector* list;
	PiVector lights;
	PiCamera* camera;
	uint mask;
}LightRule;


typedef struct
{
	char* key;
	PiCamera* camera;
	SortType sort_type;
	uint mask;
	PiVector* target_list;
	PiBool renderable;
	PiBool multi_light;
	PiCompareFunc sort_func;
}Rule;



typedef struct  
{
	uint mask;
	void* obj;
	PiVector* target_list;
}SubRenderSceneObj;

typedef struct
{
	PiCamera* camera;
	uint mask;
	PiVector* rule_list;
}RuleGroup;

typedef struct
{
	S3dScene* s3d_scene;
	LightRule* light_rule;

	PiVector* rule_list;
	uint rule_count;

	PiVector* pipeline_list;
	uint pipeline_count;
	PiFrustum* query_obj;

	PiVector* sub_render_obj_pool;
	RuleGroup* rule_groups[MAX_GROUP_NUM];
	uint rule_group_num;

	PiVector* query_list;

	ParticleManager* particle_manager;

}RenderScene;

typedef struct  
{
	uint s3d_obj_handle;
	RenderScene* parent;
	PiVector* sub_Render_scene_obj_list;
	uint mask;
	PiSpatial* spatial;
	PiBool is_visible;
}RenderSceneObj;






PI_BEGIN_DECLS
RenderScene* PI_API pi_render_scene_create(float minx, float miny, float minz, float maxx, float maxy, float maxz);

void PI_API pi_render_scene_destroy(RenderScene* scene);

RenderSceneObj* PI_API pi_render_scene_create_obj(RenderScene* scene, PiSpatial* spatial, uint mask);

void PI_API pi_render_scene_destroy_obj(RenderScene* scene, RenderSceneObj* obj);

void PI_API pi_render_scene_obj_add_sub_obj(RenderSceneObj *obj, void* data, uint mask, PiVector* target_list);

int PI_API pi_render_scene_obj_remove_sub_obj(RenderSceneObj *obj, void* data, uint mask);

void PI_API pi_render_scene_change_sub_obj_target_list(RenderSceneObj* obj, void* data, uint mask, PiVector* target_list);

void PI_API pi_render_scene_add_light_rule(RenderScene* scene, LightRule* rule);

void PI_API pi_render_scene_remove_light_rule(RenderScene* scene);

void PI_API pi_render_scene_add_rule(RenderScene* scene, Rule* rule);

void PI_API pi_render_scene_remove_rule(RenderScene* scene, char* key);

RenderScene* PI_API pi_render_scene_clone(RenderScene* scene);

void PI_API pi_render_scene_add_pipeline(RenderScene* scene, PiRenderPipelineExt* pipeline);

void PI_API pi_render_scene_remove_pipeline(RenderScene* scene, PiRenderPipelineExt* pipeline);

void PI_API pi_render_scene_draw(RenderScene* scene, float tpf);

void PI_API pi_render_scene_run(RenderScene* scene, ControllerManager* cmgr, float tpf);

void PI_API pi_render_scene_group_rule(RenderScene* scene);

void PI_API pi_render_scene_add_particle_manager(RenderScene* scene, ParticleManager* pmgr);

LightRule* PI_API pi_light_rule_create(PiCamera* camera, uint mask, PiVector* targetList);

Rule* PI_API pi_rule_create(char* key, PiCamera* camera, SortType type, uint mask, PiVector* targetList, PiBool renderable, PiBool multiLight);

void PI_API pi_rule_change_camera(Rule* rule, PiCamera* camera);

void PI_API pi_rule_free();

PI_END_DECLS


#endif