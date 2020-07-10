
#include <skbinding.h>
#include <skanim.h>
#include <pi_skeleton.h>
#include <particle_emitter.h>
#include "cpu_particles.h"

typedef struct
{
	PiAABBBox aabb;

	sint bone_id;			/* 骨头id */
	char *bone_name;		/* 骨头名字 */
	PiSpatial *spatial;		/* 被绑定的spatial */
	PiController *skanim;

	PiMatrix4 local_matrix;	/* 局部矩阵 */
	PiMatrix4 matrix;		/* 绑定后的世界矩阵 */

	PiMatrix4 cache_matrix; /* 用于计算的矩阵缓存 */
} SkeletonBinding;

static PiBool _update(struct PiController *c, float tpf)
{
	SkeletonBinding *impl = c->impl;

	if (impl->skanim != NULL)
	{
		if (impl->bone_id <= 0)
		{
			impl->bone_id = pi_skanim_get_bone_id(impl->skanim, impl->bone_name);
		}

		if (impl->bone_id >= 0)
		{
			pi_skanim_get_bone_matrix(impl->skanim, impl->bone_id, &impl->matrix);
		}
		else
		{
			pi_mat4_set_identity(&impl->matrix);
		}
	}

	/* 先偏移，再作用骨头矩阵 */
	pi_mat4_mul(&impl->matrix, &impl->matrix, &impl->local_matrix);

	return FALSE;
}

static void _apply_spatial(PiController *c, PiSpatial *spatial) 
{
	SkeletonBinding *impl = (SkeletonBinding *)c->impl;
	PiQuaternion rotation;
	PiVector3 translation, scaling;

	pi_mat4_decompose(&translation, &scaling, &rotation, &impl->cache_matrix);

	pi_spatial_set_local_translation(spatial, translation.x, translation.y, translation.z);
	pi_spatial_set_local_scaling(spatial, scaling.x, scaling.y, scaling.z);
	pi_spatial_set_local_rotation(spatial, rotation.w, rotation.x, rotation.y, rotation.z);

	pi_spatial_update(spatial);
}

static void _apply_list(PiController *c, PiVector* list)
{
	SkeletonBinding *impl = (SkeletonBinding *)c->impl;
	PiMatrix4 matrix;
	int i, size;
	size = pi_vector_size(list);
	for (i = 0; i < size; ++i)
	{
		BindingNode *update_node = (BindingNode *)pi_vector_get(list, i);
		//设置了偏移属性才去计算偏移矩阵，否则使用骨骼矩阵
		if (update_node->offset_enable && update_node->type != SYNC_TRANSLATION)
		{
			pi_mat4_mul(&matrix, &impl->cache_matrix, &update_node->offset_matrix);
		}
		else
		{
			matrix = impl->cache_matrix;
		}
		switch (update_node->type)
		{
		case SYNC_NONE:
		case SYNC_BOTH:
			//直接把矩阵拆到spatial的local属性中
			pi_mat4_decompose(&update_node->spatial->local_translation, &update_node->spatial->local_scaling, &update_node->spatial->local_rotation, &matrix);
			pi_spatial_update_simple(update_node->spatial);
			break;
		case SYNC_TRANSLATION:
			//只拆矩阵的translation属性
			pi_mat4_extract_translate(&update_node->spatial->local_translation, &matrix);
			if (update_node->offset_enable) {
				pi_vec3_add(&update_node->spatial->local_translation, &update_node->spatial->local_translation, &update_node->offset_translation);
			}
			pi_spatial_update_simple(update_node->spatial);
			break;
		default:
			pi_log_print(LOG_WARNING, "warning: ignore because the bind node's type isn't valid");
			break;
		}
	}
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	SkeletonBinding *impl = (SkeletonBinding *)c->impl;
	PiSpatial *spatial;
	PiVector* list;

	if (impl->skanim == NULL)
	{
		return TRUE;
	}

	/* 最后作用于被绑定物体的世界矩阵 */
	pi_mat4_mul(&impl->cache_matrix, &impl->spatial->world_transform_matrix, &impl->matrix);

	switch (type)
	{
		case CAT_ENTITY:
			spatial = pi_entity_get_spatial((PiEntity *)obj);
			_apply_spatial(c, spatial);
			break;

		case CAT_SPATIAL:
			spatial = (PiSpatial *)obj;
			_apply_spatial(c, spatial);
			break;

		case CAT_BINDING_LIST:
			list = (PiVector *)obj;
			_apply_list(c, list);
			break;

		default:
			pi_log_print(LOG_WARNING, "warning: ignore because the apply's type isn't valid");
			return FALSE;
	}
	return TRUE;
}

PiController *PI_API pi_skbinding_new()
{
	SkeletonBinding *impl = pi_new0(SkeletonBinding, 1);

	PiController *c = pi_controller_new(CT_SKBINDING, _apply, _update, impl);

	pi_mat4_set_identity(&impl->matrix);
	pi_mat4_set_identity(&impl->cache_matrix);
	return c;
}

void PI_API pi_skbinding_free(PiController *c)
{
	SkeletonBinding *impl = c->impl;

	pi_free(impl->bone_name);

	pi_free(impl);

	pi_controller_free(c);
}

PiBool PI_API pi_skbinding_set_bone(PiController *c, PiController *skanim, const char *bone_name, PiSpatial *spatial, PiMatrix4 *local_matrix)
{
	SkeletonBinding *impl = c->impl;
	PI_ASSERT(skanim->type == CT_SKANIM, "set bone failed, skanim is not skanim controller");

	impl->skanim = skanim;
	impl->spatial = spatial;

	if (local_matrix == NULL)
	{
		pi_mat4_set_identity(&impl->local_matrix);
	}
	else
	{
		pi_mat4_copy(&impl->local_matrix, local_matrix);
	}

	pi_mat4_set_identity(&impl->matrix);

	if (impl->bone_name) {
		pi_free(impl->bone_name);
	}
	impl->bone_name = pi_str_dup(bone_name);

	return TRUE;
}

BindingNode* PI_API pi_binding_node_new(PiSpatial* spatial, BindType type)
{
	BindingNode *bindNode = pi_new0(BindingNode, 1);
	pi_vec3_copy(&bindNode->offset_translation, pi_vec3_get_zero());
	pi_quat_copy(&bindNode->offset_rotation, pi_quat_get_unit());
	pi_vec3_copy(&bindNode->offset_scaling, pi_vec3_get_scale_unit());
	bindNode->type = type;
	bindNode->spatial = spatial;
	bindNode->offset_enable = FALSE;
	return bindNode;
}

void PI_API pi_binding_free(BindingNode* bindNode)
{
	pi_free(bindNode);
}

void PI_API pi_set_binding_offset_translation(BindingNode* node, float x, float y, float z)
{
	node->offset_translation.x = x;
	node->offset_translation.y = y;
	node->offset_translation.z = z;
	node->offset_enable = TRUE;
}

void PI_API pi_set_binding_offset_scale(BindingNode* node, float x, float y, float z)
{
	node->offset_scaling.x = x;
	node->offset_scaling.y = y;
	node->offset_scaling.z = z;
	node->offset_enable = TRUE;
}

void PI_API pi_set_binding_offset_rotation(BindingNode* node, float x, float y, float z, float w)
{
	node->offset_rotation.x = x;
	node->offset_rotation.y = y;
	node->offset_rotation.z = z;
	node->offset_rotation.w = w;
	node->offset_enable = TRUE;
}

void PI_API pi_apply_binding_offset(BindingNode* node)
{
	pi_mat4_build_transform(&node->offset_matrix, &node->offset_translation, &node->offset_scaling, &node->offset_rotation);
}
