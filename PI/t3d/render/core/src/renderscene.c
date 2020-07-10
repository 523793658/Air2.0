#include "renderscene.h"
#include "lights.h"
static SubRenderSceneObj* _get_sub_obj_instance(RenderScene* scene)
{
	SubRenderSceneObj* obj = pi_vector_pop(scene->sub_render_obj_pool);
	if (obj == NULL)
	{
		obj = pi_new0(SubRenderSceneObj, 1);
	}
	return obj;
}

static void _retrieve_sub_obj(RenderScene* scene, SubRenderSceneObj* sub_obj)
{
	pi_vector_push(scene->sub_render_obj_pool, sub_obj);
}

void PI_API pi_render_scene_group_rule(RenderScene* scene)
{
	uint i, j;
	RuleGroup* group;
	Rule *rule;
	//释放旧的group
	for (i = 0; i < scene->rule_group_num; i++)
	{
		group = scene->rule_groups[i];
		scene->rule_groups[i] = NULL;
		pi_vector_free(group->rule_list);
		pi_free(group);
	}
	scene->rule_group_num = 0;



	for (i = 0; i < scene->rule_count; i++)
	{
		RuleGroup* group = NULL;
		rule = pi_vector_get(scene->rule_list, i);
		for (j = 0; j < scene->rule_group_num; j++)
		{
			if (scene->rule_groups[j]->camera == rule->camera)
			{
				group = scene->rule_groups[j];
				break;
			}
		}
		if (group == NULL)
		{
			group = pi_new0(RuleGroup, 1);
			group->camera = rule->camera;
			group->rule_list = pi_vector_new();
			scene->rule_groups[scene->rule_group_num++] = group;
			
		}
		group->mask |= rule->mask;
		pi_vector_push(group->rule_list, rule);
	}
}

RenderScene* PI_API pi_render_scene_create(float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
	RenderScene* scene = pi_new0(RenderScene, 1);
	scene->s3d_scene = s3d_create(minx, miny, minz, maxx, maxy, maxz);
	scene->rule_list = pi_vector_new();
	scene->pipeline_list = pi_vector_new();
	scene->query_obj = s3d_query_obj_frustum_create();
	scene->sub_render_obj_pool = pi_vector_new();
	scene->query_list = pi_vector_new();
	return scene;
}

void PI_API pi_render_scene_destroy(RenderScene* scene)
{
	uint i, size;
	SubRenderSceneObj* obj;
	pi_vector_free(scene->query_list);
	size = pi_vector_size(scene->sub_render_obj_pool);
	for (i = 0; i < size; i++)
	{
		obj = pi_vector_get(scene->sub_render_obj_pool, i);
		pi_free(obj);
	}
	pi_vector_free(scene->sub_render_obj_pool);
	pi_free(scene->query_obj);
	pi_vector_free(scene->pipeline_list);
	pi_vector_free(scene->rule_list);
	s3d_destroy(scene->s3d_scene);
	pi_free(scene);
}

RenderSceneObj* PI_API pi_render_scene_create_obj(RenderScene* scene, PiSpatial* spatial, uint mask)
{
	RenderSceneObj* obj = pi_new0(RenderSceneObj, 1);
	obj->s3d_obj_handle = s3d_obj_create(scene->s3d_scene, spatial);
	s3d_obj_set_link(scene->s3d_scene, obj->s3d_obj_handle, obj);
	s3d_obj_insert(scene->s3d_scene, obj->s3d_obj_handle);
	obj->is_visible = TRUE;
	obj->parent = scene;
	obj->sub_Render_scene_obj_list = pi_vector_new();
	obj->spatial = spatial;
	return obj;
}

void PI_API pi_render_scene_obj_set_visible(RenderSceneObj* obj, PiBool visible)
{
	obj->is_visible = visible;
	if (obj->is_visible)
	{
		s3d_obj_set_mask(obj->parent->s3d_scene, obj->s3d_obj_handle, obj->mask);
	}
	else
	{
		s3d_obj_set_mask(obj->parent->s3d_scene, obj->s3d_obj_handle, 0);
	}
}

void PI_API pi_render_scene_change_sub_obj_target_list(RenderSceneObj* obj, void* data, uint mask, PiVector* target_list)
{
	uint i, size;
	size = pi_vector_size(obj->sub_Render_scene_obj_list);
	for (i = 0; i < size; i++)
	{
		SubRenderSceneObj* sub_obj = pi_vector_get(obj->sub_Render_scene_obj_list, i);
		if (sub_obj->obj == data)
		{
			sub_obj->target_list = target_list;
			sub_obj->mask = mask;
			obj->mask |= mask;
			if (obj->is_visible){
				s3d_obj_set_mask(obj->parent->s3d_scene, obj->s3d_obj_handle, obj->mask);
			}
			return;
		}
	}
}


void PI_API pi_render_scene_obj_add_sub_obj(RenderSceneObj *obj, void* data, uint mask, PiVector* target_list)
{
	SubRenderSceneObj* sub_obj = _get_sub_obj_instance(obj->parent);
	sub_obj->mask = mask;
	sub_obj->obj = data;
	sub_obj->target_list = target_list;
	obj->mask |= mask;

	pi_vector_push(obj->sub_Render_scene_obj_list, sub_obj);
	if (obj->is_visible){
		s3d_obj_set_mask(obj->parent->s3d_scene, obj->s3d_obj_handle, obj->mask);
	}
}

int PI_API pi_render_scene_obj_remove_sub_obj(RenderSceneObj *obj, void* data, uint mask)
{
	uint i, size;
	SubRenderSceneObj* subObj;
	size = pi_vector_size(obj->sub_Render_scene_obj_list);
	for (i = 0; i < size; i++)
	{
		subObj = pi_vector_get(obj->sub_Render_scene_obj_list, i);
		if (subObj->obj == data)
		{
			pi_vector_remove_unorder(obj->sub_Render_scene_obj_list, i);
			_retrieve_sub_obj(obj->parent, subObj);
			return size - 1;
		}
	}
	return size;
}

void PI_API pi_render_scene_obj_remove_sub_obj_by_mask(RenderSceneObj* obj, uint mask)
{
	uint i, size;
	SubRenderSceneObj* sub_obj;
	size = pi_vector_size(obj->sub_Render_scene_obj_list);
	for (i = size - 1; i >= 0; i--)
	{
		sub_obj = pi_vector_get(obj->sub_Render_scene_obj_list, i);
		if (sub_obj->mask == mask)
		{

			pi_vector_remove_unorder(obj->sub_Render_scene_obj_list, i);
			_retrieve_sub_obj(obj->parent, sub_obj);
			//s3d_obj_set_mask(obj->parent->s3d_scene, obj->s3d_obj_handle, obj->mask);
		}
	}
}

void PI_API pi_render_scene_destroy_obj(RenderScene* scene, RenderSceneObj* obj)
{
	if (obj->parent == scene)
	{
		uint i, count;
		SubRenderSceneObj* sub_obj;
		count = pi_vector_size(obj->sub_Render_scene_obj_list);
		for (i = 0; i < count; i++)
		{
			sub_obj = pi_vector_get(obj->sub_Render_scene_obj_list, i);
			_retrieve_sub_obj(scene, sub_obj);
		}
		s3d_obj_destroy(scene->s3d_scene, obj->s3d_obj_handle);
		pi_vector_free(obj->sub_Render_scene_obj_list);
		pi_free(obj);
	}
}

void PI_API pi_render_scene_add_light_rule(RenderScene* scene, LightRule* rule)
{
	if (scene->light_rule != NULL)
	{
		pi_free(scene->light_rule);
	}
	scene->light_rule = rule;
}

void PI_API pi_render_scene_remove_light_rule(RenderScene* scene)
{
	if (scene->light_rule != NULL)
	{
		pi_free(scene->light_rule);
		scene->light_rule = NULL;
	}
}

void PI_API pi_render_scene_add_rule(RenderScene* scene, Rule* rule_new)
{
	uint i;
	rule_new->sort_func = pi_renderlist_get_sort_func(rule_new->sort_type);
	for (i = 0; i < scene->rule_count; i++)
	{
		Rule* rule = pi_vector_get(scene->rule_list, i);
		if (pi_str_equal(rule->key, rule_new->key, FALSE))
		{
			pi_vector_remove_unorder(scene->rule_list, i);
			pi_vector_push(scene->rule_list, rule_new);
			return;
		}
	}
	pi_vector_push(scene->rule_list, rule_new);
	scene->rule_count++;
}

void PI_API pi_render_scene_remove_rule(RenderScene* scene, char* key)
{
	uint i;
	for (i = 0; i < scene->rule_count; i++)
	{
		Rule* rule = pi_vector_get(scene->rule_list, i);
		if (pi_str_equal(rule->key, key, FALSE))
		{
			pi_vector_remove(scene->rule_list, i);
			scene->rule_count --;
		}
	}
}

RenderScene* PI_API pi_render_scene_clone(RenderScene* scene)
{
	//暂不实现	
	return NULL;
}

void PI_API pi_render_scene_add_pipeline(RenderScene* scene, PiRenderPipelineExt* pipeline_new)
{
	uint i;
	for (i = 0; i < scene->pipeline_count; i++)
	{
		PiRenderPipelineExt* pipeline = pi_vector_get(scene->pipeline_list, i);
		if (pipeline == pipeline_new)
		{
			break;
		}
	}
	if (i == scene->pipeline_count)
	{
		pi_vector_push(scene->pipeline_list, pipeline_new);
		scene->pipeline_count++;
	}
}

void PI_API pi_render_scene_remove_pipeline(RenderScene* scene, PiRenderPipelineExt* pipeline)
{
	uint i;
	for (i = 0; i < scene->pipeline_count; i++)
	{
		PiRenderPipelineExt* pipe = pi_vector_get(scene->pipeline_list, i);
		if (pipe == pipeline)
		{
			break;
		}
	}
	if (i < scene->pipeline_count)
	{
		pi_vector_remove(scene->pipeline_list, i);
	}
}

void PI_API pi_render_scene_draw(RenderScene* scene, float tpf)
{

}

void PI_API _query_and_dispatch(RenderScene* scene)
{
	uint i, j, k, size, num;
	RuleGroup* group;
	PiCamera* camera;
	PiMatrix4* viewMatrix;
	PiMatrix4* projMatrix;
	RenderSceneObj* render_scene_obj;
	SubRenderSceneObj* sub_render_obj;
	Rule* rule;
	for (i = 0; i < scene->rule_group_num; i++)
	{

		group = scene->rule_groups[i];
		num = pi_vector_size(group->rule_list);
		for (k = 0; k < num; k++)
		{
			rule = pi_vector_get(group->rule_list, k);
			pi_vector_clear(rule->target_list, FALSE);
		}


		camera = group->camera;
		viewMatrix = pi_camera_get_view_matrix(camera);
		projMatrix = pi_camera_get_projection_matrix(camera);
		pi_vector_clear(scene->query_list, FALSE);
		s3d_query_obj_frustum_update_matrix(scene->query_obj, viewMatrix, projMatrix);
		s3d_query_link_frustum(scene->s3d_scene, group->mask, scene->query_list, scene->query_obj, EQA_AABB);

		size = pi_vector_size(scene->query_list);
		for (j = 0; j < size; j++)
		{
			render_scene_obj = pi_vector_get(scene->query_list, j);
			num = pi_vector_size(render_scene_obj->sub_Render_scene_obj_list);
			for (k = 0; k < num; k++)
			{
				sub_render_obj = pi_vector_get(render_scene_obj->sub_Render_scene_obj_list, k);
				if (sub_render_obj->mask & group->mask)
				{
					pi_vector_push(sub_render_obj->target_list, sub_render_obj->obj);
				}
			}
		}
	}
}

static PiSelectR PI_API _set_visible_flag(void *user_data, PiController *value)
{
	value->need_update = TRUE;
	return SELECT_NEXT;
}

static void _update_controller(ControllerManager* cmgr, float tpf)
{
	ControllerNode* node;
	uint i, j, size, apply_num;
	PiVector* controllerData;
	ControllerApplyData* data;
	pi_vector_foreach(cmgr->query_list, _set_visible_flag, NULL);
	controllerData = pi_controller_manager_get(cmgr);
	size = pi_vector_size(controllerData);
	for (i = 0; i < size; i++)
	{
		node = pi_vector_get(controllerData, i);
		node->data->delta_time += tpf;
		if (node->data->need_update)
		{
			pi_controller_update(node->data, node->data->delta_time);
			node->data->delta_time = 0.0f;
			apply_num = pi_vector_size(&node->apply_array);
			for (j = 0; j < apply_num; j++)
			{
				data = pi_vector_get(&node->apply_array, j);
				pi_controller_apply(node->data, data->type, data->apply_obj);
			}
			node->data->need_update = FALSE;
		}
	}
}

static void _apply_multi_light(RenderScene* scene)
{
	Rule* rule;
	uint i, j, k, num, size;
	PiAABBBox* aabb;
	PiVector* entity_light_list;
	size = pi_vector_size(&scene->light_rule->lights);
	for (i = 0; i < scene->rule_count; i++)
	{
		rule = pi_vector_get(scene->rule_list, i);
		if (rule->multi_light)
		{
			num = pi_vector_size(rule->target_list);
			for (j = 0; j < num; j++)
			{
				PiEntity* entity = pi_vector_get(rule->target_list, j);
				aabb = pi_entity_get_world_aabb(entity);
				entity_light_list = pi_entity_get_bind(entity, EBT_LIGHT_LIST);
				pi_vector_clear(entity_light_list, FALSE);
				for (k = 0; k < size; k++)
				{
					PointLight* light = pi_vector_get(&scene->light_rule->lights, k);
					if (sphere2aabb(aabb, (PiSphere*)light))
					{
						pi_vector_push(entity_light_list, light);
					}
				}
			}
		}
	}
}

static void _query_light(RenderScene* scene)
{
	PiCamera* camera = scene->light_rule->camera;
	PiMatrix4* viewMatrix = pi_camera_get_view_matrix(camera);
	PiMatrix4* projMatrix = pi_camera_get_projection_matrix(camera);
	uint size, i, j, num;
	RenderSceneObj* obj;
	SubRenderSceneObj* sobj;
	pi_vector_clear(scene->light_rule->list, FALSE);
	s3d_query_obj_frustum_update_matrix(scene->query_obj, viewMatrix, projMatrix);
	s3d_query_link_frustum(scene->s3d_scene, scene->light_rule->mask, scene->light_rule->list, scene->query_obj, EQA_AABB);
	size = pi_vector_size(scene->light_rule->list);
	pi_vector_clear(&scene->light_rule->lights, FALSE);
	for (i = 0; i < size; i++){
		obj = pi_vector_get(scene->light_rule->list, i);
		num = pi_vector_size(obj->sub_Render_scene_obj_list);
		for (j = 0; j < num; j++)
		{
			sobj = pi_vector_get(obj->sub_Render_scene_obj_list, j);
			if (sobj->mask == scene->light_rule->mask)
			{
				pi_vector_push(&scene->light_rule->lights, sobj->obj);
			}
		}
	}
}

void PI_API pi_render_scene_add_particle_manager(RenderScene* scene, ParticleManager* pmgr)
{
	scene->particle_manager = pmgr;
}

static void _update_particle(ParticleManager* pmgr,PiCamera* camera, float tpf)
{
	uint size, i;
	ParticleSystem* particle_system;
	pmgr->time += tpf;
	pi_particle_manager_sort(pmgr, camera);
	if (pmgr->normal_avaliable)
	{
		pi_vector_clear(pmgr->normal_entity_list, FALSE);
	}
	if (pmgr->flucatuation_avaliable)
	{
		pi_vector_clear(pmgr->flucatuation_entity_list, FALSE);
	}
	size = pi_vector_size(pmgr->query_list);
	for (i = 0; i < size; i++)
	{
		particle_system = pi_vector_get(pmgr->query_list, i);
		//更新粒子并获取entity
		pi_particle_system_update(particle_system, pmgr->time);
		if (pmgr->normal_avaliable)
		{
			pi_particle_system_get_entity(particle_system, CT_NORMAL, pmgr->normal_entity_list);
		}
		if (pmgr->flucatuation_avaliable)
		{
			pi_particle_system_get_entity(particle_system, CT_FLUCTUATION, pmgr->flucatuation_entity_list);
		}
	}
}

void PI_API pi_render_scene_run(RenderScene* scene, ControllerManager* cmgr, float tpf)
{
	uint i, size;
	Rule* rule;
	//查询分发
	_query_and_dispatch(scene);

	//更新控制器
	_update_controller(cmgr, tpf);

	//更新粒子
	if (scene->particle_manager){
		_update_particle(scene->particle_manager, scene->particle_manager->camera, tpf);
	}

	if (scene->light_rule != NULL)
	{
		//查询点光源
		_query_light(scene);

		//点光源交叉检测
		_apply_multi_light(scene);
	}
	
	size = pi_vector_size(scene->rule_list);
	for (i = 0; i < size; i++)
	{
		rule = pi_vector_get(scene->rule_list, i);
		if (rule->renderable)
		{
			pi_renderlist_sort(rule->target_list, rule->sort_func, rule->camera);
		}
	}
	for (i = 0; i < scene->pipeline_count; i++)
	{
		PiRenderPipelineExt* pipeline = pi_vector_get(scene->pipeline_list, i);
		pi_renderpipeline_ext_draw(pipeline, tpf);
	}
}

 
LightRule* PI_API pi_light_rule_create(PiCamera* camera, uint mask, PiVector* targetList)
{
	LightRule* rule = pi_new0(LightRule, 1);
	rule->camera = camera;
	rule->mask = mask;
	rule->list = targetList;
	return rule;
}

Rule* PI_API pi_rule_create(char* key, PiCamera* camera, SortType type, uint mask, PiVector* targetList, PiBool renderable, PiBool multiLight)
{
	Rule* rule = pi_new0(Rule, 1);
	rule->key = pi_str_dup(key);
	rule->camera = camera;
	rule->sort_type = type;
	rule->mask = mask;
	rule->target_list = targetList;
	rule->renderable = renderable;
	rule->multi_light = multiLight;
	return rule;
}

void PI_API pi_rule_change_camera(Rule* rule, PiCamera* camera)
{
	rule->camera = camera;
}

void PI_API pi_rule_free(Rule* rule)
{
	pi_free(rule->key);
	pi_free(rule);
}