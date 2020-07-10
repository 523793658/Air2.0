#include "s3d.h"

#define _OVERRIDE_MASK_DEFAULT 0x0
#define _OVERRIDE_MASK_POINT 0x1
#define _OVERRIDE_MASK_AABB 0x2
#define _OVERRIDE_MASK_OBB 0x4

//只有群碰撞执行时才会使用此Mask
#define _COLLISION_OVER_MASK 0x8

//反复释放检测
#define _REPEAT_FREE_MASK 0x10

#define _SCENE_OBJ_SIZE sizeof(_S3dObj)

#define _SCENE_OBJ(handle) ((_S3dObj*)(pi_ha_ptr(mgr->obj_alloc, handle)))

typedef PiBool (PI_API *_IntersectFunc)(void *obj_p, void *obj_i);

typedef struct
{
	S3dScene *scene;
	S3dObjHandle id;
} SpatialUpdateData;

typedef struct
{
	PiVector3 point;
	PiAABBBox aabb;
	PiOBBBox obb;
	S3dOTObjHandle octree_entity;
	uint internal_mask;
	void *link;

	PiSpatial *spatial;						/* 空间属性，用于和entity等模块关联 */
	SpatialUpdateData *spatial_update_data;	/* spatial更新时的用户数据 */
} _S3dObj;

typedef struct
{
	S3dQueryAccuracy accuracy;
	S3dScene *scene;
	_IntersectFunc point_filter;
	_IntersectFunc aabb_filter;
	_IntersectFunc obb_filter;
} _QueryUserData;

typedef struct
{
	S3dQueryAccuracy accuracy;
	S3dScene *scene;
	S3dObjHandle collision_obj;

	_IntersectFunc point_filter;
	_IntersectFunc aabb_filter;
	_IntersectFunc obb_filter;
} _CollisionUserData;

static S3dQueryAccuracy _obj_get_lowest_accuracy(_S3dObj *obj)
{
	if ((obj->internal_mask & _OVERRIDE_MASK_OBB) != 0)
	{
		return EQA_OBB;
	}
	else if ((obj->internal_mask & _OVERRIDE_MASK_AABB) != 0)
	{
		return EQA_AABB;
	}
	else if ((obj->internal_mask & _OVERRIDE_MASK_POINT) != 0)
	{
		return EQA_POINT;
	}
	else
	{
		PI_ASSERT(TRUE, "Object spatial attribute error!");
		return EQA_AABB;
	}
}

static void *_octree_collision_result_operation(S3dOTObjHandle obj, void *user_data)
{
	_CollisionUserData *query_user_data = (_CollisionUserData *)user_data;
	S3dScene *mgr = query_user_data->scene;
	return ot_obj_get_user_data(mgr->octree, obj);
}

static PiBool _collision_obj_filter(S3dOTObjHandle obj, void *query_obj, void *user_data)
{
	_CollisionUserData *collision_user_data = (_CollisionUserData *)user_data;
	S3dScene *mgr = collision_user_data->scene;
	S3dObjHandle s3d_obj_handle = (S3dObjHandle)(intptr_t)ot_obj_get_user_data(mgr->octree, obj);
	_S3dObj *s3d_obj = _SCENE_OBJ(s3d_obj_handle);
	S3dQueryAccuracy query_accuracy = collision_user_data->accuracy;
	S3dQueryAccuracy obj_accuracy = _obj_get_lowest_accuracy(s3d_obj);
	S3dQueryAccuracy accuracy = MIN(query_accuracy, obj_accuracy);
	PiBool result = FALSE;
	_S3dObj *collision_obj_ptr = _SCENE_OBJ(collision_user_data->collision_obj);
	if (collision_user_data->collision_obj == s3d_obj_handle || (collision_obj_ptr->internal_mask & _COLLISION_OVER_MASK))
	{
		result = FALSE;
	}
	else
	{
		switch (accuracy)
		{
		case EQA_POINT:
			result = collision_user_data->point_filter(&s3d_obj->point, query_obj);
			break;
		case EQA_AABB:
			result = collision_user_data->aabb_filter(&s3d_obj->aabb, query_obj);
			break;
		case EQA_OBB:
			if ((s3d_obj->internal_mask & _OVERRIDE_MASK_OBB) == 0)
			{
				result = collision_user_data->aabb_filter(&s3d_obj->aabb, query_obj);
			}
			else
			{
				result = collision_user_data->obb_filter(&s3d_obj->obb, query_obj);
			}
			break;
		}
	}

	return result;
}

static void *_octree_query_result_operation(S3dOTObjHandle obj, void *user_data)
{
	_QueryUserData *query_user_data = (_QueryUserData *)user_data;
	S3dScene *mgr = query_user_data->scene;
	return ot_obj_get_user_data(mgr->octree, obj);
}

static void *_octree_query_link_result_operation(S3dOTObjHandle obj, void *user_data)
{
	_QueryUserData *query_user_data = (_QueryUserData *)user_data;
	S3dScene *mgr = query_user_data->scene;
	uint sobj_id = (uint)(uintptr_t)ot_obj_get_user_data(mgr->octree, obj);
	return s3d_obj_get_link(mgr, sobj_id);
}

static EIntersectState _mask_node_filter(PiAABBBox *aabb, void *query_obj, void *user_data)
{
	return EIS_INSIDE;
}

static EIntersectState _line_node_filter(PiAABBBox *aabb, void *query_obj, void *user_data)
{
	return line2aabb_i(aabb, (PiLineSegment *)query_obj);
}

static EIntersectState _frustum_node_filter(PiAABBBox *aabb, void *query_obj, void *user_data)
{
	return frustum2aabb_i(aabb, (PiFrustum *)query_obj);
}

static EIntersectState _sphere_node_filter(PiAABBBox *aabb, void *query_obj, void *user_data)
{
	return sphere2aabb_i(aabb, (PiSphere *)query_obj);
}

static EIntersectState _aabb_node_filter(PiAABBBox *aabb, void *query_obj, void *user_data)
{
	return aabb2aabb_i(aabb, (PiAABBBox *)query_obj);
}

static EIntersectState _obb_node_filter(PiAABBBox *aabb, void *query_obj, void *user_data)
{
	return obb2aabb_i(aabb, (PiOBBBox *)query_obj);
}

static PiBool _mask_obj_filter(S3dOTObjHandle obj, void *query_obj, void *user_data)
{
	return TRUE;
}

static PiBool PI_API _obb2aabb(PiAABBBox *aabb, PiOBBBox *obb)
{
	return aabb2obb(obb, aabb);
}

static PiBool _query_obj_flter(S3dOTObjHandle obj, void *query_obj, void *user_data)
{
	_QueryUserData *query_user_data = (_QueryUserData *)user_data;
	S3dScene *mgr = query_user_data->scene;
	_S3dObj *s3d_obj = _SCENE_OBJ((S3dObjHandle)(intptr_t)ot_obj_get_user_data(mgr->octree, obj));
	S3dQueryAccuracy query_accuracy = query_user_data->accuracy;
	S3dQueryAccuracy obj_accuracy = _obj_get_lowest_accuracy(s3d_obj);
	S3dQueryAccuracy accuracy = MIN(query_accuracy, obj_accuracy);
	switch (accuracy)
	{
	case EQA_POINT:
		return query_user_data->point_filter(&s3d_obj->point, query_obj);
	case EQA_AABB:
		return query_user_data->aabb_filter(&s3d_obj->aabb, query_obj);
	case EQA_OBB:
		if ((s3d_obj->internal_mask & _OVERRIDE_MASK_OBB) == 0)
		{
			return query_user_data->aabb_filter(&s3d_obj->aabb, query_obj);
		}
		else
		{
			return query_user_data->obb_filter(&s3d_obj->obb, query_obj);
		}
	default:
		return FALSE;
	}
}

static void _obj_update(S3dScene *mgr, S3dObjHandle id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	if (obj->internal_mask == _OVERRIDE_MASK_DEFAULT)
	{
		s3d_obj_remove(mgr, id);
	}
	if ((obj->internal_mask & _OVERRIDE_MASK_AABB) == 0)
	{
		pi_obb_get_aabb(&obj->obb, &obj->aabb);
	}
	if ((obj->internal_mask & _OVERRIDE_MASK_POINT) == 0)
	{
		pi_aabb_get_center(&obj->aabb, &obj->point);
	}
	if ((obj->internal_mask & _OVERRIDE_MASK_POINT) != 0 && (obj->internal_mask & _OVERRIDE_MASK_AABB) == 0 && (obj->internal_mask & _OVERRIDE_MASK_OBB) == 0)
	{
		obj->aabb.minPt.x = obj->aabb.maxPt.x = obj->point.x;
		obj->aabb.minPt.y = obj->aabb.maxPt.y = obj->point.y;
		obj->aabb.minPt.z = obj->aabb.maxPt.z = obj->point.z;
	}
	ot_obj_update(mgr->octree, obj->octree_entity, obj->aabb.minPt.x, obj->aabb.minPt.y, obj->aabb.minPt.z, obj->aabb.maxPt.x, obj->aabb.maxPt.y, obj->aabb.maxPt.z);
	//ot_obj_set_point(mgr->octree, obj->octree_entity, &obj->point);
	//ot_obj_set_obb(mgr->octree, obj->octree_entity, &obj->obb);
}

S3dScene *PI_API s3d_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	S3dScene *scene = (S3dScene *)pi_malloc0(sizeof(S3dScene));
	scene->octree = ot_create(min_x, min_y, min_z, max_x, max_y, max_z);
	scene->obj_alloc = pi_ha_new(_SCENE_OBJ_SIZE);

	return scene;
}

void PI_API s3d_destroy(S3dScene *mgr)
{
	//直接销毁物件句柄分配器
	ot_destroy(mgr->octree);
	pi_ha_free(mgr->obj_alloc);
	pi_free(mgr);
}

void PI_API s3d_copy(S3dScene *dst, S3dScene *src)
{
	ot_copy(dst->octree, src->octree);
	pi_ha_copy(dst->obj_alloc, src->obj_alloc, TRUE);
}

void PI_API s3d_update(S3dScene *mgr)
{
	ot_update(mgr->octree);
}

static void _spatial_update(struct PiSpatial *spatial, void *user_data)
{
	SpatialUpdateData *data = (SpatialUpdateData *)user_data;
	s3d_obj_set_aabb(data->scene, data->id, pi_spatial_get_world_aabb(spatial));
}

uint PI_API s3d_obj_create(S3dScene *mgr, PiSpatial* spatial)
{
	S3dObjHandle obj_handle = pi_ha_create0(mgr->obj_alloc);

	_S3dObj *obj = _SCENE_OBJ(obj_handle);

	if (spatial != NULL) 
	{
		obj->spatial = spatial;
		obj->spatial_update_data = pi_new0(SpatialUpdateData, 1);
		obj->spatial_update_data->scene = mgr;
		obj->spatial_update_data->id = obj_handle;
		pi_spatial_set_update_operation(obj->spatial, _spatial_update, obj->spatial_update_data, FALSE);
	}

	obj->octree_entity = ot_obj_create(mgr->octree, (void *)(intptr_t)obj_handle, MAX_FLOAT, MAX_FLOAT, MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT);
	obj->link = NULL;
	obj->internal_mask = _OVERRIDE_MASK_DEFAULT | _REPEAT_FREE_MASK;

	return obj_handle;
}

void PI_API s3d_obj_destroy(S3dScene *mgr, uint id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	if ((obj->internal_mask & _REPEAT_FREE_MASK) == 0)
	{
		pi_log_print(LOG_INFO, "s3d object destroy once more.");
		return;
	}

	if (obj->spatial != NULL)
	{
		pi_spatial_remove_update_operation(obj->spatial, obj->spatial_update_data);
	}

	if (obj->spatial_update_data != NULL)
	{
		pi_free(obj->spatial_update_data);
	}

	obj->internal_mask = _OVERRIDE_MASK_DEFAULT;
	ot_obj_destroy(mgr->octree, obj->octree_entity);
	pi_ha_delete(mgr->obj_alloc, id);
}

void PI_API s3d_obj_insert(S3dScene *mgr, uint id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	PI_ASSERT((obj->internal_mask & _REPEAT_FREE_MASK), "s3d operation on a destroyed object!");
	PI_ASSERT(obj->internal_mask != _OVERRIDE_MASK_DEFAULT, "Could not insert an object without any spatial attribute.");
	ot_obj_insert(mgr->octree, obj->octree_entity);
}

void PI_API s3d_obj_remove(S3dScene *mgr, uint id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	PI_ASSERT((obj->internal_mask & _REPEAT_FREE_MASK), "s3d operation on a destroyed object!");
	ot_obj_remove(mgr->octree, obj->octree_entity);
}

void PI_API s3d_obj_get_point(S3dScene *mgr, uint id, float *pos)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	pos[0] = obj->point.x;
	pos[1] = obj->point.y;
	pos[2] = obj->point.z;
}

PiSpatial *PI_API s3d_obj_get_spatial(S3dScene *mgr, uint id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	return obj->spatial;
}

void PI_API s3d_obj_get_aabb(S3dScene *mgr, uint id, PiAABBBox *aabb)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	pi_aabb_copy(aabb, &obj->aabb);
}

void PI_API s3d_obj_get_obb(S3dScene *mgr, uint id, PiOBBBox *obb)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	pi_obb_copy(obb, &obj->obb);
}

void PI_API s3d_obj_set_point(S3dScene *mgr, uint id, PiVector3 *position)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	PI_ASSERT((obj->internal_mask & _REPEAT_FREE_MASK), "s3d operation on a destroyed object!");
	if (position == NULL)
	{
		obj->internal_mask &= ~_OVERRIDE_MASK_POINT;
	}
	else
	{
		obj->internal_mask |= _OVERRIDE_MASK_POINT;
		obj->point.x = position->x;
		obj->point.y = position->y;
		obj->point.z = position->z;
	}
	_obj_update(mgr, id);
}

void PI_API s3d_obj_set_aabb(S3dScene *mgr, uint id, PiAABBBox *aabb)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	PI_ASSERT((obj->internal_mask & _REPEAT_FREE_MASK), "s3d operation on a destroyed object!");
	if (aabb == NULL)
	{
		obj->internal_mask &= ~_OVERRIDE_MASK_AABB;
	}
	else
	{
		obj->internal_mask |= _OVERRIDE_MASK_AABB;
		pi_aabb_copy(&obj->aabb, aabb);
	}
	_obj_update(mgr, id);
}

void PI_API s3d_obj_set_obb(S3dScene *mgr, uint id, PiOBBBox *obb)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	PI_ASSERT((obj->internal_mask & _REPEAT_FREE_MASK), "s3d operation on a destroyed object!");
	if (obb == NULL)
	{
		obj->internal_mask &= ~_OVERRIDE_MASK_OBB;
	}
	else
	{
		obj->internal_mask |= _OVERRIDE_MASK_OBB;
		pi_obb_copy(&obj->obb, obb);
	}
	_obj_update(mgr, id);
}

uint PI_API s3d_obj_get_mask(S3dScene *mgr, uint id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	return ot_obj_get_mask(mgr->octree, obj->octree_entity);
}

void PI_API s3d_obj_set_mask(S3dScene *mgr, uint id, uint mask)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	ot_obj_set_mask(mgr->octree, obj->octree_entity, mask);
}

void *PI_API s3d_obj_get_link(S3dScene *mgr, uint id)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	return obj->link;
}

void PI_API s3d_obj_set_link(S3dScene *mgr, uint id, void *link)
{
	_S3dObj *obj = _SCENE_OBJ(id);
	obj->link = link;
}

void PI_API s3d_query(S3dScene *mgr, uint mask, PiVector *list)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	ot_query(mgr->octree, _mask_node_filter, _mask_obj_filter, mask, list, NULL, _octree_query_result_operation, &query_user_data);
}

void PI_API s3d_query_line(S3dScene *mgr, uint mask, PiVector *list, PiLineSegment *line, S3dQueryAccuracy accuracy)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	query_user_data.accuracy = accuracy;
	query_user_data.point_filter = (_IntersectFunc)line2point;
	query_user_data.aabb_filter = (_IntersectFunc)line2aabb;
	query_user_data.obb_filter = (_IntersectFunc)line2obb;
	ot_query(mgr->octree, _line_node_filter, _query_obj_flter, mask, list, line, _octree_query_result_operation, &query_user_data);
}

void PI_API s3d_query_frustum(S3dScene *mgr, uint mask, PiVector *list, PiFrustum *frustum, S3dQueryAccuracy accuracy)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	query_user_data.accuracy = accuracy;
	query_user_data.point_filter = (_IntersectFunc)frustum2point;
	query_user_data.aabb_filter = (_IntersectFunc)frustum2aabb;
	query_user_data.obb_filter = (_IntersectFunc)frustum2obb;
	ot_query(mgr->octree, _frustum_node_filter, _query_obj_flter, mask, list, frustum, _octree_query_result_operation, &query_user_data);
}

void PI_API s3d_query_link_frustum(S3dScene *mgr, uint mask, PiVector *list, PiFrustum *frustum, S3dQueryAccuracy accuracy)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	query_user_data.accuracy = accuracy;
	query_user_data.point_filter = (_IntersectFunc)frustum2point;
	query_user_data.aabb_filter = (_IntersectFunc)frustum2aabb;
	query_user_data.obb_filter = (_IntersectFunc)frustum2obb;
	ot_query(mgr->octree, _frustum_node_filter, _query_obj_flter, mask, list, frustum, _octree_query_link_result_operation, &query_user_data);

}

void PI_API s3d_query_sphere(S3dScene *mgr, uint mask, PiVector *list, PiSphere *sphere, S3dQueryAccuracy accuracy)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	query_user_data.accuracy = accuracy;
	query_user_data.point_filter = (_IntersectFunc)sphere2point;
	query_user_data.aabb_filter = (_IntersectFunc)sphere2aabb;
	query_user_data.obb_filter = (_IntersectFunc)sphere2obb;
	ot_query(mgr->octree, _sphere_node_filter, _query_obj_flter, mask, list, sphere,_octree_query_result_operation, &query_user_data);
}

void PI_API s3d_query_aabb(S3dScene *mgr, uint mask, PiVector *list, PiAABBBox *aabb, S3dQueryAccuracy accuracy)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	query_user_data.accuracy = accuracy;
	query_user_data.point_filter = (_IntersectFunc)aabb2point;
	query_user_data.aabb_filter = (_IntersectFunc)aabb2aabb;
	query_user_data.obb_filter = (_IntersectFunc)aabb2obb;
	ot_query(mgr->octree, _aabb_node_filter, _query_obj_flter, mask, list, aabb, _octree_query_result_operation, &query_user_data);
}

void PI_API s3d_query_obb(S3dScene *mgr, uint mask, PiVector *list, PiOBBBox *obb, S3dQueryAccuracy accuracy)
{
	_QueryUserData query_user_data;
	query_user_data.scene = mgr;
	query_user_data.accuracy = accuracy;
	query_user_data.point_filter = (_IntersectFunc)obb2point;
	query_user_data.aabb_filter = (_IntersectFunc)_obb2aabb;
	query_user_data.obb_filter = (_IntersectFunc)obb2obb;
	ot_query(mgr->octree, _obb_node_filter, _query_obj_flter, mask, list, obb, _octree_query_result_operation, &query_user_data);
}

void PI_API s3d_collision(S3dScene *mgr, uint mask, PiVector *list,  uint id, S3dQueryAccuracy accuracy)
{
	_CollisionUserData collision_user_data;
	_S3dObj *s3d_obj = _SCENE_OBJ(id);
	S3dQueryAccuracy obj_accuracy = _obj_get_lowest_accuracy(s3d_obj);
	S3dQueryAccuracy quary_accuracy = MIN(accuracy, obj_accuracy);
	void *query_obj = NULL;
	NodeFilter node_filter = NULL;
	collision_user_data.accuracy = quary_accuracy;
	collision_user_data.scene = mgr;
	collision_user_data.collision_obj = id;
	switch (quary_accuracy)
	{
	case EQA_POINT:
		//TODO:支持PointQuary
		return;
	case EQA_AABB:
		query_obj = &s3d_obj->aabb;
		node_filter = _aabb_node_filter;
		collision_user_data.point_filter = (_IntersectFunc)aabb2point;
		collision_user_data.aabb_filter = (_IntersectFunc)aabb2aabb;
		collision_user_data.obb_filter = (_IntersectFunc)aabb2obb;
		break;
	case EQA_OBB:
		query_obj = &s3d_obj->obb;
		node_filter = _obb_node_filter;
		collision_user_data.point_filter = (_IntersectFunc)obb2point;
		collision_user_data.aabb_filter = (_IntersectFunc)_obb2aabb;
		collision_user_data.obb_filter = (_IntersectFunc)obb2obb;
		break;
	default:
		return;
	}

	ot_query(mgr->octree, node_filter, _collision_obj_filter, mask, list, query_obj, _octree_collision_result_operation, &collision_user_data);
}

void PI_API s3d_collision_group(S3dScene *mgr, uint mask, PiVector *list,  PiVector *id_list, S3dQueryAccuracy accuracy)
{
	uint i, size = pi_vector_size(id_list);
	for (i = 0; i < size; i++)
	{
		uint id = (uint)(uintptr_t)pi_vector_get(id_list, i);
		_S3dObj *obj = _SCENE_OBJ(id);
		pi_vector_push(list, (void *)(intptr_t)id);
		s3d_collision(mgr, mask, list,  id, accuracy);
		pi_vector_push(list, NULL);
		obj->internal_mask |= _COLLISION_OVER_MASK;
	}

	for (i = 0; i < size; i++)
	{
		uint id = (uint)(uintptr_t)pi_vector_get(id_list, i);
		_S3dObj *obj = _SCENE_OBJ(id);
		obj->internal_mask &= ~_COLLISION_OVER_MASK;
	}
}

PiLineSegment *PI_API s3d_query_obj_line_create()
{
	return pi_new0(PiLineSegment, 1);
}

PiFrustum *PI_API s3d_query_obj_frustum_create()
{
	return pi_new0(PiFrustum, 1);
}

PiSphere *PI_API s3d_query_obj_sphere_create()
{
	return pi_new0(PiSphere, 1);
}

PiAABBBox *PI_API s3d_query_obj_aabb_create()
{
	return pi_new0(PiAABBBox, 1);
}

PiOBBBox *PI_API s3d_query_obj_obb_create()
{
	return pi_new0(PiOBBBox, 1);
}

void PI_API s3d_query_obj_line_update(void *queryObj, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z)
{
	PiLineSegment *line_ptr = (PiLineSegment *)queryObj;
	line_ptr->start.x = start_x;
	line_ptr->start.y = start_y;
	line_ptr->start.z = start_z;
	line_ptr->end.x = end_x;
	line_ptr->end.y = end_y;
	line_ptr->end.z = end_z;
}

void PI_API s3d_query_obj_frustum_update_matrix(void *queryObj, PiMatrix4 *viewMat, PiMatrix4 *projMat)
{
	pi_frustum_update((PiFrustum *)queryObj, viewMat, projMat);
}

void PI_API s3d_query_obj_frustum_update(void *queryObj, PiVector3 *pos, PiVector3 *dir, PiVector3 *up, float left, float right, float bottom, float top, float near, float far, PiBool is_ortho)
{
	PiMatrix4 view_matrix, projection_matrix, rotation;
	PiVector3 direction, up_vec, left_vec;

	if (is_ortho)
	{
		pi_mat4_ortho_rh(&projection_matrix, left, right, bottom, top, near, far);
	}
	else
	{
		pi_mat4_frustum_rh(&projection_matrix, left, right, bottom, top, near, far);
	}

	pi_vec3_normalise(&direction, dir);
	pi_vec3_normalise(&up_vec, up);
	pi_vec3_cross(&left_vec, &direction, &up_vec);
	pi_vec3_normalise(&left_vec, &left_vec);
	pi_vec3_cross(&up_vec, &left_vec, &direction);

	pi_mat4_set_identity(&rotation);
	rotation.m[0][0] = left_vec.x;
	rotation.m[0][1] = left_vec.y;
	rotation.m[0][2] = left_vec.z;
	rotation.m[1][0] = up_vec.x;
	rotation.m[1][1] = up_vec.y;
	rotation.m[1][2] = up_vec.z;
	rotation.m[2][0] = -direction.x;
	rotation.m[2][1] = -direction.y;
	rotation.m[2][2] = -direction.z;

	pi_mat4_set_identity(&view_matrix);
	view_matrix.m[0][0] = rotation.m[0][0];
	view_matrix.m[0][1] = rotation.m[0][1];
	view_matrix.m[0][2] = rotation.m[0][2];
	view_matrix.m[0][3] = -(rotation.m[0][0] * pos->x + rotation.m[0][1] * pos->y + rotation.m[0][2] * pos->z);
	view_matrix.m[1][0] = rotation.m[1][0];
	view_matrix.m[1][1] = rotation.m[1][1];
	view_matrix.m[1][2] = rotation.m[1][2];
	view_matrix.m[1][3] = -(rotation.m[1][0] * pos->x + rotation.m[1][1] * pos->y + rotation.m[1][2] * pos->z);
	view_matrix.m[2][0] = rotation.m[2][0];
	view_matrix.m[2][1] = rotation.m[2][1];
	view_matrix.m[2][2] = rotation.m[2][2];
	view_matrix.m[2][3] = -(rotation.m[2][0] * pos->x + rotation.m[2][1] * pos->y + rotation.m[2][2] * pos->z);

	pi_frustum_update((PiFrustum *)queryObj, &view_matrix, &projection_matrix);
}

void PI_API s3d_query_obj_sphere_update(void *queryObj, float pos_x, float pos_y, float pos_z, float radius)
{
	PiSphere *sphere_ptr = (PiSphere *)queryObj;
	sphere_ptr->pos.x = pos_x;
	sphere_ptr->pos.y = pos_y;
	sphere_ptr->pos.z = pos_z;
	sphere_ptr->radius = radius;
}

void PI_API s3d_query_obj_aabb_update(void *queryObj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	PiAABBBox *cube_ptr = (PiAABBBox *)queryObj;
	cube_ptr->minPt.x = min_x;
	cube_ptr->minPt.y = min_y;
	cube_ptr->minPt.z = min_z;
	cube_ptr->maxPt.x = max_x;
	cube_ptr->maxPt.y = max_y;
	cube_ptr->maxPt.z = max_z;
}

void PI_API s3d_query_obj_obb_update(void *queryObj, float center_x, float center_y, float center_z, float axis_0_x, float axis_0_y, float axis_0_z, float axis_1_x, float axis_1_y, float axis_1_z, float axis_2_x, float axis_2_y, float axis_2_z, float extent_0, float extent_1, float extent_2)
{
	PiOBBBox *cube_ptr = (PiOBBBox *)queryObj;
	cube_ptr->center.x = center_x;
	cube_ptr->center.y = center_y;
	cube_ptr->center.z = center_z;
	cube_ptr->axis[0].x = axis_0_x;
	cube_ptr->axis[0].y = axis_0_y;
	cube_ptr->axis[0].z = axis_0_z;
	cube_ptr->axis[1].x = axis_1_x;
	cube_ptr->axis[1].y = axis_1_y;
	cube_ptr->axis[1].z = axis_1_z;
	cube_ptr->axis[2].x = axis_2_x;
	cube_ptr->axis[2].y = axis_2_y;
	cube_ptr->axis[2].z = axis_2_z;
	cube_ptr->extent[0] = extent_0;
	cube_ptr->extent[1] = extent_1;
	cube_ptr->extent[2] = extent_2;
}

void PI_API s3d_query_obj_destroy(void *queryObj)
{
	pi_free(queryObj);
}
