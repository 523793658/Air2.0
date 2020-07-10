#include "pi_spatial.h"

static uint aabb_size = sizeof(PiAABBBox);

static PiBool PI_API _vector_remove_func(void *user_data, const void *data)
{
	return user_data == data;
}

static void _update_callback(PiSpatial* spatial, PiBool mask_flag)
{
	int i, size;
	size = pi_vector_size(&spatial->on_update);
	for (i = 0; i < size; ++i)
	{
		_SpatialUpdate *update_node = (_SpatialUpdate *)pi_vector_get(&spatial->on_update, i);
		if (update_node->ignore_mask || mask_flag)
		{
			update_node->update_fun(spatial, update_node->user_data);
		}
	}
}

static void _type_check(PiSpatial* spatial, PiSpatialType type)
{
	PI_ASSERT(spatial->type == type, "Spatial type error! a func limited for 'Geometry/Node' been called with a wrong type value!");
}

static void _attach_type_check(PiSpatial* spatial, PiSpatialAttachType type)
{
	PI_ASSERT(spatial->attach_type == type, "Spatial type error! a func limited for 'Geometry/Node' been called with a wrong type value!");
}

static PiBool _is_bound_need_update(PiSpatial* spatial)
{
	return (spatial->update_mask & _UPDATE_MASK_BOUND) != _UPDATE_MASK_DEFAULT;
}

static PiBool _is_transfom_need_update(PiSpatial* spatial)
{
	return (spatial->update_mask & _UPDATE_MASK_TRANSFORM) != _UPDATE_MASK_DEFAULT;
}

static void _set_bound_update(PiSpatial* spatial)
{
	spatial->update_mask |= _UPDATE_MASK_BOUND;

	// 同步节点不影响parent的bound
	if (spatial->attach_type != ESAT_SYNC && spatial->parent != NULL) 
	{
		_set_bound_update(spatial->parent);
	}
}

static void _set_transform_update(PiSpatial* spatial)
{
	uint i, size;
	PiSpatial* child;

	spatial->update_mask |= _UPDATE_MASK_TRANSFORM;
	_set_bound_update(spatial);

	size = pi_vector_size(&spatial->children);
	for (i = 0; i < size; ++i)
	{
		child = (PiSpatial*)pi_vector_get(&spatial->children, i);
		if (child->attach_type != ESAT_SYNC) 
		{
			_set_transform_update(child);
		}		
	}
}

static void _set_parent(PiSpatial* spatial, PiSpatial* parent, PiSpatialAttachType type)
{
	spatial->parent = parent;
	spatial->attach_type = type;
	_set_transform_update(spatial);
}

//该函数给骨骼绑定更新使用，主要区别在于不计算aabb的合成，只更新aabb的world_transform
static void _simple_update_bound(PiSpatial* spatial)
{
	PiBool aabb_enable = (spatial->update_mask & _UPDATE_MASK_AABB) != 0 || spatial->has_basic_aabb;
	if (aabb_enable)
	{
		pi_aabb_copy(&spatial->world_aabb, &spatial->local_aabb);
		pi_aabb_transform(&spatial->world_aabb, &spatial->world_transform_matrix);
	}
	spatial->update_mask &= ~_UPDATE_MASK_BOUND;
}

static void _update_bound(PiSpatial* spatial) 
{
	spatial->update_bound_ptr(spatial);
	spatial->update_mask &= ~_UPDATE_MASK_BOUND;
}

static void _geometry_update_bound(PiSpatial* spatial) 
{
	_Geometry* geometry_impl = (_Geometry*)spatial->impl;
	PiBool obb_enable = (spatial->update_mask & _UPDATE_MASK_OBB) != 0;
	PiBool aabb_enable = (spatial->update_mask & _UPDATE_MASK_AABB) != 0;

	if (!aabb_enable && obb_enable)
	{
		pi_obb_get_aabb(&geometry_impl->local_obb, &spatial->local_aabb);
	}
		
	if (aabb_enable)
	{
		pi_aabb_copy(&spatial->world_aabb, &spatial->local_aabb);
		pi_aabb_transform(&spatial->world_aabb, &spatial->world_transform_matrix);
	}
		
	if (obb_enable) 
	{
		pi_obb_copy(&geometry_impl->world_obb, &geometry_impl->local_obb);
		pi_obb_transform(&geometry_impl->world_obb, &spatial->world_transform_matrix);
	}
}

static void _node_update_bound(PiSpatial* spatial) 
{
	//TODO:此处可以考虑直接使用子节点的WorldAABB进行合并，同时去掉Node类型只保留pi_get_world_aabb方法以节约开销，因为实际使用中node的局部aabb信息并无太大的实际使用价值
	uint i, size = pi_vector_size(&spatial->children);
	PiSpatial* child;
	PiAABBBox tmp_aabb;
	PiBool aabb_updated = spatial->has_basic_aabb;
	pi_memcpy_inline(&spatial->local_aabb, &spatial->basic_aabb, aabb_size);
	for(i = 0; i < size; i++) 
	{
		child = (PiSpatial*)pi_vector_get(&spatial->children, i);
		//if ((child->update_mask & _UPDATE_MASK_AABB) != 0)
		if (child->attach_type != ESAT_SYNC)
		{
			aabb_updated = TRUE;
			pi_aabb_copy(&tmp_aabb, &child->local_aabb);
			pi_aabb_transform(&tmp_aabb, &child->local_transform_matrix);
			pi_aabb_merge(&spatial->local_aabb, &spatial->local_aabb, &tmp_aabb);
		}
	}
	if (aabb_updated)
	{
		pi_aabb_copy(&spatial->world_aabb, &spatial->local_aabb);
		pi_aabb_transform(&spatial->world_aabb, &spatial->world_transform_matrix);
	}
	else
	{
		pi_aabb_init(&spatial->world_aabb);
	}
}

static void _update_transform(PiSpatial* spatial)
{
	PiVector3 *parent_translation, *parent_scaling;
	PiQuaternion* parent_rotation;
	
	if (spatial->parent != NULL) {
		_update_transform(spatial->parent);
	}

	if ((spatial->update_mask & _UPDATE_MASK_TRANSFORM) == _UPDATE_MASK_DEFAULT)
	{
		return;
	}

	pi_mat4_build_transform(&spatial->local_transform_matrix, &spatial->local_translation, &spatial->local_scaling, &spatial->local_rotation);

	switch (spatial->attach_type)
	{
	case ESAT_NONE:
		pi_vec3_copy(&spatial->world_translation, &spatial->local_translation);
		pi_quat_copy(&spatial->world_rotation, &spatial->local_rotation);
		pi_vec3_copy(&spatial->world_scaling, &spatial->local_scaling);
		pi_mat4_copy(&spatial->world_transform_matrix, &spatial->local_transform_matrix);
		break;
	case ESAT_NODE:
		parent_translation = pi_spatial_get_world_translation(spatial->parent);
		parent_rotation = pi_spatial_get_world_rotation(spatial->parent);
		parent_scaling = pi_spatial_get_world_scaling(spatial->parent);
		//Scaling
		pi_vec3_mul(&spatial->world_scaling, &spatial->local_scaling, parent_scaling);
		//Rotation
		pi_quat_mul(&spatial->world_rotation, parent_rotation, &spatial->local_rotation);
		//Translation
		pi_vec3_mul(&spatial->world_translation, &spatial->local_translation, parent_scaling);
		pi_quat_rotate_vec3(&spatial->world_translation, &spatial->world_translation, parent_rotation);
		pi_vec3_add(&spatial->world_translation, parent_translation, &spatial->world_translation);
		pi_mat4_build_transform(&spatial->world_transform_matrix, &spatial->world_translation, &spatial->world_scaling, &spatial->world_rotation);
		break;
	default:
		break;
	}
	spatial->update_mask &= ~_UPDATE_MASK_TRANSFORM;
}

static void _spatial_update_sync(PiSpatial *spatial)
{
	pi_vec3_copy(&spatial->local_translation, &spatial->parent->local_translation);
	pi_quat_copy(&spatial->local_rotation, &spatial->parent->local_rotation);
	pi_vec3_copy(&spatial->local_scaling, &spatial->parent->local_scaling);

	pi_vec3_copy(&spatial->world_translation, &spatial->parent->world_translation);
	pi_quat_copy(&spatial->world_rotation, &spatial->parent->world_rotation);
	pi_vec3_copy(&spatial->world_scaling, &spatial->parent->world_scaling);

	pi_mat4_copy(&spatial->local_transform_matrix, &spatial->parent->local_transform_matrix);
	pi_mat4_copy(&spatial->world_transform_matrix, &spatial->parent->world_transform_matrix);

	// 约定：同步不更新OBB
	if (spatial->type == EST_GEOMETRY)
	{
		if (!(spatial->update_mask & _UPDATE_MASK_AABB))
		{
			pi_aabb_copy(&spatial->local_aabb, &spatial->parent->local_aabb);
			pi_aabb_copy(&spatial->world_aabb, &spatial->parent->world_aabb);
		}
	}
}

static void _simple_update(PiSpatial* spatial)
{
	PiSpatial *child;
	PiBool b = (spatial->update_mask & (_UPDATE_MASK_BOUND | _UPDATE_MASK_TRANSFORM)) != _UPDATE_MASK_DEFAULT;
	uint i, size = pi_vector_size(&spatial->children);

	_update_transform(spatial);

	// 更新附加子节点
	for (i = 0; i < size; i++)
	{
		child = (PiSpatial*)pi_vector_get(&spatial->children, i);
		if (child->attach_type != ESAT_SYNC)
		{
			_simple_update(child);
		}
	}

	// 更新包围盒位置信息
	if ((spatial->update_mask & _UPDATE_MASK_BOUND) != _UPDATE_MASK_DEFAULT)
	{
		_simple_update_bound(spatial);
	}

	// 更新同步节点
	for (i = 0; i < size; i++)
	{
		child = (PiSpatial*)pi_vector_get(&spatial->children, i);
		if (child->attach_type == ESAT_SYNC)
		{
			_spatial_update_sync(child);
			_simple_update(child);
		}
	}

	//以下更新是主要的消耗点
	_update_callback(spatial, b);

}

static void _spatial_update(PiSpatial* spatial)
{
	PiSpatial *child;
	uint i, size = pi_vector_size(&spatial->children);

	// 更新变换矩阵
	_update_transform(spatial);
	
	// 更新附加子节点
	for (i = 0; i < size; i++)
	{
		child = (PiSpatial*)pi_vector_get(&spatial->children, i);
		if (child->attach_type != ESAT_SYNC)
		{
			pi_spatial_update(child);
		}		
	}

	// 更新包围盒
	if ((spatial->update_mask & _UPDATE_MASK_BOUND) != _UPDATE_MASK_DEFAULT)
	{
		_update_bound(spatial);
	}

	// 更新同步节点
	for (i = 0; i < size; i++)
	{
		child = (PiSpatial*)pi_vector_get(&spatial->children, i);
		if (child->attach_type == ESAT_SYNC)
		{
			_spatial_update_sync(child);
			_spatial_update(child);
			_update_callback(child, TRUE);
		}
	}
}

static PiAABBBox* _geometry_get_local_aabb(PiSpatial* spatial)
{
	return &spatial->local_aabb;
}

static PiAABBBox* _node_get_local_aabb(PiSpatial* spatial)
{
	if (_is_bound_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->local_aabb;
}

static void _spatial_init(PiSpatial* spatial)
{
	pi_vector_init(&spatial->children);
	pi_vector_init(&spatial->on_update);
	pi_vec3_copy(&spatial->local_translation, pi_vec3_get_zero());
	pi_quat_copy(&spatial->local_rotation, pi_quat_get_unit());
	pi_vec3_copy(&spatial->local_scaling, pi_vec3_get_scale_unit());
	pi_aabb_init(&spatial->local_aabb);
	pi_aabb_init(&spatial->basic_aabb);
	pi_aabb_init(&spatial->world_aabb);
	spatial->parent = NULL;
	
	spatial->update_mask |= _UPDATE_MASK_TRANSFORM;
	pi_spatial_update(spatial);
}

static void _spatial_free(PiSpatial* spatial) 
{
	int i, size;
	size = pi_vector_size(&spatial->on_update);
	for (i = 0; i < size; ++i)
	{
		_SpatialUpdate *update_node = (_SpatialUpdate *)pi_vector_pop(&spatial->on_update);
		pi_free(update_node);
	}

	pi_spatial_detach_children(spatial);
}

PiSpatial* PI_API pi_spatial_geometry_create()
{
	PiSpatial* spatial = (PiSpatial*)pi_malloc0(sizeof(PiSpatial));
	_Geometry* geometry_impl = (_Geometry*)pi_malloc0(sizeof(_Geometry));
	spatial->type = EST_GEOMETRY;
	spatial->impl = geometry_impl;
	spatial->update_bound_ptr = _geometry_update_bound;
	spatial->get_local_aabb_ptr = _geometry_get_local_aabb;
	spatial->free_ptr = _spatial_free;
	 
	_spatial_init(spatial);

	return spatial;
}

PiSpatial* PI_API pi_spatial_node_create()
{
	PiSpatial* spatial = (PiSpatial*)pi_malloc0(sizeof(PiSpatial));
	_Node* node_impl = (_Node*)pi_malloc0(sizeof(_Node));
	spatial->type = EST_NODE;
	spatial->impl = node_impl;
	spatial->update_bound_ptr = _node_update_bound;
	spatial->get_local_aabb_ptr = _node_get_local_aabb;
	spatial->free_ptr = _spatial_free;

	_spatial_init(spatial);

	return spatial;
}

void PI_API pi_spatial_destroy(PiSpatial* spatial)
{
	if(spatial != NULL)
	{
		pi_spatial_detach_from_parent(spatial);
		spatial->free_ptr(spatial);
		pi_vector_clear(&spatial->children, TRUE);
		pi_vector_clear(&spatial->on_update, TRUE);
		pi_free(spatial->impl);
		pi_free(spatial);
	}
}

PiSpatialType PI_API pi_spatial_get_type(PiSpatial* spatial)
{
	return spatial->type;
}

void PI_API pi_spatial_attach_sync(PiSpatial *dst, PiSpatial *src)
{
	if (src->parent != dst)
	{
		PI_ASSERT(src->parent == NULL, "sync, Spatial has already been attached to a parent.");

		_set_parent(src, dst, ESAT_SYNC);
		pi_vector_push(&dst->children, src);
		_set_bound_update(dst);
	}
}

void PI_API pi_spatial_detach_sync(PiSpatial *dst, PiSpatial *src)
{
	_attach_type_check(src, ESAT_SYNC);
	PI_ASSERT(src->parent == dst, "sync, 'Child' has not been attached to this parent.");
	pi_vector_remove_if(&dst->children, _vector_remove_func, src);
	_set_parent(src, NULL, ESAT_NONE);
	_set_bound_update(dst);
}

void PI_API pi_spatial_set_local_translation(PiSpatial* spatial, float x, float y, float z)
{
	spatial->local_translation.x = x;
	spatial->local_translation.y = y;
	spatial->local_translation.z = z;
	_set_transform_update(spatial);
}

void PI_API pi_spatial_set_local_rotation(PiSpatial* spatial, float w, float x, float y, float z)
{
	spatial->local_rotation.w = w;
	spatial->local_rotation.x = x;
	spatial->local_rotation.y = y;
	spatial->local_rotation.z = z;
	_set_transform_update(spatial);
}

void PI_API pi_spatial_set_local_scaling(PiSpatial* spatial, float x, float y, float z)
{
	spatial->local_scaling.x = x;
	spatial->local_scaling.y = y;
	spatial->local_scaling.z = z;
	_set_transform_update(spatial);
}

PiVector3* PI_API pi_spatial_get_local_translation(PiSpatial* spatial)
{
	return &spatial->local_translation;
}

PiQuaternion* PI_API pi_spatial_get_local_rotation(PiSpatial* spatial)
{
	return &spatial->local_rotation;
}

PiVector3* PI_API pi_spatial_get_local_scaling(PiSpatial* spatial)
{
	return &spatial->local_scaling;
}

PiMatrix4* PI_API pi_spatial_get_local_transform(PiSpatial* spatial)
{
	if (_is_transfom_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->local_transform_matrix;
}

PiAABBBox* PI_API pi_spatial_get_local_aabb(PiSpatial* spatial)
{
	return spatial->get_local_aabb_ptr(spatial);
}

PiVector3* PI_API pi_spatial_get_world_translation(PiSpatial* spatial)
{
	if (_is_transfom_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->world_translation;
}

PiQuaternion* PI_API pi_spatial_get_world_rotation(PiSpatial* spatial)
{
	if (_is_transfom_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->world_rotation;
}

PiVector3* PI_API pi_spatial_get_world_scaling(PiSpatial* spatial)
{
	if (_is_transfom_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->world_scaling;
}

PiMatrix4* PI_API pi_spatial_get_world_transform(PiSpatial* spatial)
{
	if (_is_transfom_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->world_transform_matrix;
}

PiAABBBox* PI_API pi_spatial_get_world_aabb(PiSpatial* spatial)
{
	if (_is_bound_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	return &spatial->world_aabb;
}

PiSpatial* PI_API pi_spatial_get_parent(PiSpatial* spatial)
{
	return spatial->parent;
}

void PI_API pi_spatial_detach_from_parent(PiSpatial* spatial)
{
	switch (spatial->attach_type)
	{
	case ESAT_NONE:
		break;
	case ESAT_NODE:
		pi_node_detach_child(spatial->parent, spatial);
		break;
	case ESAT_SYNC:
		pi_spatial_detach_sync(spatial->parent, spatial);
		break;
	default:
		break;
	}
}

void PI_API pi_spatial_update(PiSpatial* spatial)
{
	PiBool b = (spatial->update_mask & (_UPDATE_MASK_BOUND | _UPDATE_MASK_TRANSFORM)) != _UPDATE_MASK_DEFAULT;

	_spatial_update(spatial);

	_update_callback(spatial, b);
}

void PI_API pi_spatial_set_update_operation(PiSpatial* spatial, OnUpdateFunc operation_func, void* user_data, PiBool is_check)
{
	_SpatialUpdate *update_node = pi_new0(_SpatialUpdate, 1);
	update_node->update_fun = operation_func;
	update_node->user_data = user_data;
	update_node->ignore_mask = !is_check;
	pi_vector_push(&spatial->on_update, update_node);
}

void PI_API pi_spatial_remove_update_operation(PiSpatial* spatial, void* user_data) 
{
	int i, size;
	size = pi_vector_size(&spatial->on_update);
	for (i = size - 1; i >= 0; i--)
	{
		_SpatialUpdate *update_node = (_SpatialUpdate *)pi_vector_get(&spatial->on_update, i);
		if (update_node->user_data == user_data)
		{
			pi_vector_remove(&spatial->on_update, i);
			pi_free(update_node);
			return;
		}
	}
}

void PI_API pi_geometry_set_local_aabb(PiSpatial* spatial, PiAABBBox* aabb)
{	
	_type_check(spatial, EST_GEOMETRY);
	if(aabb == NULL) 
	{
		spatial->update_mask &= ~_UPDATE_MASK_AABB;
		pi_aabb_init(&spatial->local_aabb);
		pi_aabb_init(&spatial->world_aabb);
	}
	else
	{
		spatial->update_mask |= _UPDATE_MASK_AABB;
		pi_aabb_copy(&spatial->local_aabb, aabb);
	}
	
	_set_bound_update(spatial);
}

void PI_API pi_spatial_join_basic_aabb(PiSpatial* spatial, PiAABBBox* aabb)
{
	spatial->update_mask |= _UPDATE_MASK_AABB;
	pi_aabb_merge(&spatial->basic_aabb, &spatial->basic_aabb, aabb);
	spatial->has_basic_aabb = TRUE;
	_set_bound_update(spatial);
}

void PI_API pi_geometry_set_local_obb(PiSpatial* spatial, PiOBBBox* obb)
{
	_Geometry* geometry_impl;
	_type_check(spatial, EST_GEOMETRY);
	if(obb == NULL) {
		spatial->update_mask &= ~_UPDATE_MASK_OBB;
	}
	else
	{
		spatial->update_mask |= _UPDATE_MASK_OBB;
		geometry_impl = (_Geometry*)spatial->impl;
		pi_obb_copy(&geometry_impl->local_obb, obb);
	}
	_set_bound_update(spatial);
}

PiOBBBox* PI_API pi_geometry_get_local_obb(PiSpatial* spatial)
{
	_Geometry* geometry_impl;
	_type_check(spatial, EST_GEOMETRY);
	if((spatial->update_mask & _UPDATE_MASK_OBB) == 0) {
		return NULL;
	}
	else
	{
		geometry_impl = (_Geometry*)spatial->impl;
		return &geometry_impl->local_obb;
	}
}

PiOBBBox* PI_API pi_geometry_get_world_obb(PiSpatial* spatial)
{
	_Geometry* geometry_impl;
	_type_check(spatial, EST_GEOMETRY);
	
	if (_is_bound_need_update(spatial)) {
		pi_spatial_update(spatial);
	}
	if((spatial->update_mask & _UPDATE_MASK_OBB) == 0) {
		return NULL;
	}
	else
	{
		geometry_impl = (_Geometry*)spatial->impl;
		return &geometry_impl->world_obb;
	}
}

void PI_API pi_node_attach_child(PiSpatial* spatial, PiSpatial* child)
{
	if (child->parent != spatial)
	{
		_type_check(spatial, EST_NODE);

		PI_ASSERT(child->parent == NULL, "Spatial has already been attached to a parent.");
		_set_parent(child, spatial, ESAT_NODE);
		pi_vector_push(&spatial->children, child);
		_set_bound_update(spatial);
	}	
}
 
void PI_API pi_node_detach_child(PiSpatial* spatial, PiSpatial* child)
{
	_type_check(spatial, EST_NODE);
	PI_ASSERT(child->parent == spatial, "'Child' has not been attached to this parent.");
	pi_vector_remove_if(&spatial->children, _vector_remove_func, child);
	_set_parent(child, NULL, ESAT_NONE);
	_set_bound_update(spatial);
}

void PI_API pi_spatial_detach_children(PiSpatial* spatial)
{
	uint i, size = pi_vector_size(&spatial->children);
	for(i = 0; i < size; i++) 
	{
		PiSpatial *child = (PiSpatial*)pi_vector_pop(&spatial->children);
		_set_parent(child, NULL, ESAT_NONE);
	}
	_set_bound_update(spatial);
}

PiVector* PI_API pi_node_get_children(PiSpatial* spatial)
{
	_type_check(spatial, EST_NODE);
	return &spatial->children;
}

void PI_API pi_spatial_update_simple(PiSpatial* spatial)
{
	_set_transform_update(spatial);
	_simple_update(spatial);
}
