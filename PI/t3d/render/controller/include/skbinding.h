#ifndef INCLUDE_SK_BINDING_H
#define INCLUDE_SK_BINDING_H

#include <controller.h>
#include <pi_skeleton.h>
#include <pi_aabb.h>
#include <pi_spatial.h>

//作用类型
typedef enum
{
	SYNC_NONE,
	SYNC_BOTH,
	SYNC_TRANSLATION,
	SYNC_SCALING,
	SYNC_ROTATION,
} BindType;

typedef struct
{
	PiSpatial *spatial; /* 绑定的spatial */
	BindType type;  /* 绑定的类型 */
	PiBool offset_enable; /* 偏移开关 */
	PiVector3 offset_translation; /* 绑定的移动 */
	PiQuaternion offset_rotation; /* 绑定的旋转 */
	PiVector3 offset_scaling; /* 绑定的缩放 */
	PiMatrix4 offset_matrix; /* 绑定的偏移矩阵 */
} BindingNode;

/**
 * 骨头绑定控制器
 */

PI_BEGIN_DECLS

/**
 * 创建
 */
PiController* PI_API pi_skbinding_new();

/**
 * 释放
 */
void PI_API pi_skbinding_free(PiController *c);

/**
 * 设置骨头
 */
PiBool PI_API pi_skbinding_set_bone(PiController *c, PiController *skanim, const char *bone_name, PiSpatial *spatial, PiMatrix4 *local_matrix);

/**
* 新建一个绑定节点
*/
BindingNode* PI_API pi_binding_node_new(PiSpatial* spatial, BindType type);

/**
* 释放一个绑定节点
*/
void PI_API pi_binding_free(BindingNode* bindNode);

/**
* 设置绑定节点的绑定偏移
*/
void PI_API pi_set_binding_offset_translation(BindingNode* node, float x, float y, float z);

/**
* 设置绑定节点的缩放偏移
*/
void PI_API pi_set_binding_offset_scale(BindingNode* node, float x, float y, float z);

/**
* 设置绑定节点的旋转
*/
void PI_API pi_set_binding_offset_rotation(BindingNode* node, float x, float y, float z, float w);

/**
* 应用绑定节点的偏移矩阵
*/
void PI_API pi_apply_binding_offset(BindingNode* node);

PI_END_DECLS

#endif /* INCLUDE_SK_BINDING_H */