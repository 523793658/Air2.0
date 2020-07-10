#ifndef __PI_SPATIAL_H__
#define __PI_SPATIAL_H__

#include <pi_vector3.h>
#include <pi_aabb.h>
#include <pi_obb.h>
#include <pi_quaternion.h>
#include <pi_matrix4.h>

/**
 * 本文件定义了空间几何以及空间节点
 * 空间物体提供了空间变换的功能，同时由于对自身几何描述的不同，分为2类：
 * 1.几何体（Geometry）：自身拥有几何包围盒属性
 * 2.节点（Node）：自身无法拥有几何包围盒信息，但可以通过
 * 向其添加包含了几何信息的Child的方式计算出所有Child共同组合出的空间信息
 */

typedef enum 
{
	EST_GEOMETRY,
	EST_NODE
}PiSpatialType;

/**
 * 附加类型，parent的附加类型
 */
typedef enum
{
	ESAT_NONE,	/* 无附加类型 */
	ESAT_NODE,	/* 通过node附加 */
	ESAT_SYNC,	/* 通过同步附加 */
}PiSpatialAttachType;

#define _UPDATE_MASK_DEFAULT 0x0
#define _UPDATE_MASK_TRANSFORM 0x1
#define _UPDATE_MASK_BOUND 0x2
#define _UPDATE_MASK_OBB 0x4
#define _UPDATE_MASK_AABB 0x8

struct PiSpatial;

typedef void (*OnUpdateFunc)(struct PiSpatial* spatial, void* user_data);

typedef void (*_UpdateBound) (struct PiSpatial* spatial);
typedef PiAABBBox* (*_GetLocalAABB) (struct PiSpatial* spatial);
typedef void (*_Free) (struct PiSpatial* spatial);

typedef struct PiSpatial
{
	PiVector3 local_translation;
	PiQuaternion local_rotation;
	PiVector3 local_scaling;
	PiMatrix4 local_transform_matrix;
	PiAABBBox local_aabb;

	PiAABBBox basic_aabb;
	PiBool has_basic_aabb;
	PiVector3 world_translation;
	PiQuaternion world_rotation;
	PiVector3 world_scaling;
	PiMatrix4 world_transform_matrix;
	PiAABBBox world_aabb;

	uint update_mask;

	PiVector children;					/* 元素类型是PiSpatial* */
	struct PiSpatial *parent;			/* 父亲 */
	PiSpatialAttachType attach_type;	/* 附加的类型 */

	PiSpatialType type;

	PiVector on_update;

	void* impl;
	_UpdateBound update_bound_ptr;
	_GetLocalAABB get_local_aabb_ptr;
	_Free free_ptr;
}PiSpatial;

typedef struct _SpatialUpdate
{
	OnUpdateFunc update_fun;
	void* user_data;
	PiBool ignore_mask;
}_SpatialUpdate;

typedef struct _Geometry
{
	PiOBBBox local_obb;
	PiOBBBox world_obb;
}_Geometry;

typedef struct _Node
{
	char dummy;	/* 预留成员，无用的哑元 */
}_Node;

PI_BEGIN_DECLS

/**
 * 创建一个空间几何体
 * @return 几何体指针
 */
PiSpatial* PI_API pi_spatial_geometry_create();

/**
 * 创建一个空间物件
 * @return 空间物件指针
 */
PiSpatial* PI_API pi_spatial_node_create();

/**
 * 销毁一个空间节点
 * @return 空间节点指针
 */
void PI_API pi_spatial_destroy(PiSpatial* spatial);

/**
 * 返回该空间物体的类型
 * @param spatial 指定的空间物体指针
 * @return 该空间物体的类型
 */
PiSpatialType PI_API pi_spatial_get_type(PiSpatial* spatial);

/**
 * 附加同步类型，当src跟随dst后，当更新dst时，src就会拷贝dst计算过后的数据
 * 注：一旦src同步于dst，则src的平移，旋转，缩放，矩阵，AABB完全和dst一样；
 * 注：一旦src同步于dst，则src的OBB不起作用!
 */
void PI_API pi_spatial_attach_sync(PiSpatial *dst, PiSpatial *src);

/**
* 解除src对dst的同步
*/
void PI_API pi_spatial_detach_sync(PiSpatial *dst, PiSpatial *src);

/*为node节点设置初始化aabb*/
void PI_API pi_spatial_join_basic_aabb(PiSpatial* spatial, PiAABBBox* aabb);

/**
* 移除指定空间所有的子空间物件
* @param spatial 指定的空间节点指针
*/
void PI_API pi_spatial_detach_children(PiSpatial* spatial);

/**
 * 设置指定空间物体的局部空间平移
 * @param spatial 指定的空间物体指针
 * @param x 平移的X分量
 * @param y 平移的Y分量
 * @param z 平移的Z分量
 */
void PI_API pi_spatial_set_local_translation(PiSpatial* spatial, float x, float y, float z);

/**
 * 设置指定空间物体的局部空间旋转
 * 注意：传入的四元数分量需要保证已经归一化
 * @param spatial 指定的空间物体指针
 * @param w 旋转四元数的W分量
 * @param x 旋转四元数的X分量
 * @param y 旋转四元数的Y分量
 * @param z 旋转四元数的Z分量
 */
void PI_API pi_spatial_set_local_rotation(PiSpatial* spatial, float w, float x, float y, float z);

/**
 * 设置指定空间物体的局部空间缩放
 * @param spatial 指定的空间物体指针
 * @param x 缩放的X分量
 * @param y 缩放的Y分量
 * @param z 缩放的Z分量
 */
void PI_API pi_spatial_set_local_scaling(PiSpatial* spatial, float x, float y, float z);

/**
 * 获取指定空间物件的局部空间平移
 * @param spatial 指定的空间物体指针
 * @return 局部空间平移
 */
PiVector3* PI_API pi_spatial_get_local_translation(PiSpatial* spatial);

/**
 * 获取指定空间物件的局部空间旋转
 * @param spatial 指定的空间物体指针
 * @return 局部空间旋转
 */
PiQuaternion* PI_API pi_spatial_get_local_rotation(PiSpatial* spatial);

/**
 * 获取指定空间物件的局部空间缩放
 * @param spatial 指定的空间物体指针
 * @return 局部空间缩放
 */
PiVector3* PI_API pi_spatial_get_local_scaling(PiSpatial* spatial);

/**
 * 获取指定空间物件的局部空间变换矩阵
 * 注意：如果该物件的空间变换已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间物体指针
 * @return 局部空间变换矩阵
 */
PiMatrix4* PI_API pi_spatial_get_local_transform(PiSpatial* spatial);

/**
 * 获取指定空间物件位于局部空间的AABB包围盒
 * @param spatial 指定的空间物体指针
 * @return 局部空间AABB包围盒
 */
PiAABBBox* PI_API pi_spatial_get_local_aabb(PiSpatial* spatial);

/**
 * 获取指定空间物件的世界空间平移
 * 注意：如果该物件的空间变换已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间物体指针
 * @return 世界空间平移
 */
PiVector3* PI_API pi_spatial_get_world_translation(PiSpatial* spatial);

/**
 * 获取指定空间物件的世界空间旋转
 * 注意：如果该物件的空间变换已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间物体指针
 * @return 世界空间旋转
 */
PiQuaternion* PI_API pi_spatial_get_world_rotation(PiSpatial* spatial);

/**
 * 获取指定空间物件的世界空间缩放
 * 注意：如果该物件的空间变换已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间物体指针
 * @return 世界空间缩放
 */
PiVector3* PI_API pi_spatial_get_world_scaling(PiSpatial* spatial);

/**
 * 获取指定空间物件的世界空间变换矩阵
 * 注意：如果该物件的空间变换已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间物体指针
 * @return 世界空间变换矩阵
 */
PiMatrix4* PI_API pi_spatial_get_world_transform(PiSpatial* spatial);

/**
 * 获取指定空间物件位于世界空间的AABB包围盒
 * 注意：如果该物件的空间变换或包围盒信息已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间物体指针
 * @return 世界空间AABB包围盒
 */
PiAABBBox* PI_API pi_spatial_get_world_aabb(PiSpatial* spatial);

/**
 * 获取指定空间物件的父节点
 * @param spatial 指定的空间物体指针
 * @return 父节点
 */
PiSpatial* PI_API pi_spatial_get_parent(PiSpatial* spatial);

/**
 * 将指定的空间物件从其父亲节点上移除
 * @param spatial 指定的空间物体指针
 */
void PI_API pi_spatial_detach_from_parent(PiSpatial* spatial);

/**
 * 更新指定的空间物件，按需重新计算其变换以及包围盒信息
 * 注意：部分get方法执行前需要保证更新已经被正确执行后方可调用
 * @param spatial 指定的空间物体指针
 */
void PI_API pi_spatial_update(PiSpatial* spatial);

/**
 * 设置此空间物件在Update被执行后的附加操作
 * @param spatial 指定的空间物体指针
 * @param operation_func 执行的操作的函数指针
 * @param user_data 用户数据
 * @param is_check 是否检查mask
 */
void PI_API pi_spatial_set_update_operation(PiSpatial* spatial, OnUpdateFunc operation_func, void* user_data, PiBool is_check);

/**
* 删除此空间物件在Update被执行后的附加操作
* @param spatial 指定的空间物体指针
* @param user_data 用户数据（通过用户数据指针判断是那一个）
*/
void PI_API pi_spatial_remove_update_operation(PiSpatial* spatial, void* user_data);

/**
 * 设置指定的空间几何体的局部空间AABB包围盒，如果设置为NULL，启用OBB时会使用OBB计算结果覆盖AABB设置
 * 注意：Geometry类型空间物件专用方法
 * @param spatial 指定的空间几何体指针
 * @param aabb 指定的aabb包围盒
 */
void PI_API pi_geometry_set_local_aabb(PiSpatial* spatial, PiAABBBox* aabb);

/**
 * 设置指定的空间几何体的局部空间OBB包围盒,设置为NULL表示忽略OBB设置，可以通过设置AABB为NULL让OBB结果覆盖AABB
 * 注意：Geometry类型空间物件专用方法
 * @param spatial 指定的空间几何体指针
 * @param obb 指定的OBB包围盒
 */
void PI_API pi_geometry_set_local_obb(PiSpatial* spatial, PiOBBBox* obb);

/**
 * 获取指定空间物件位于局部空间的OBB包围盒
 * 注意：Geometry类型空间物件专用方法
 * @param spatial 指定的空间几何体指针
 * @return 局部空间OBB包围盒
 */
PiOBBBox* PI_API pi_geometry_get_local_obb(PiSpatial* spatial);

/**
 * 获取指定空间物件位于世界空间的OBB包围盒
 * 注意：Geometry类型空间物件专用方法
 * 注意：如果该物件的空间变换或包围盒信息已经发生改变，在此方法调用前应调用pi_spatial_update方法
 * @param spatial 指定的空间几何体指针
 * @return 世界空间OBB包围盒
 */
PiOBBBox* PI_API pi_geometry_get_world_obb(PiSpatial* spatial);

/**
 * 向指定空间节点中添加一个子空间物件
 * 注意：Node类型空间节点专用方法
 * @param spatial 指定的空间节点指针
 * @param spatial 指定的子空间物件指针
 */
void PI_API pi_node_attach_child(PiSpatial* spatial, PiSpatial* child);

/**
 * 从指定空间节点中移除指定的子空间物件
 * 注意：Node类型空间节点专用方法
 * @param spatial 指定的空间节点指针
 * @param spatial 指定的子空间物件指针
 */
void PI_API pi_node_detach_child(PiSpatial* spatial, PiSpatial* child);

/**
 * 获取指定空间物件的子空间物件列表
 * 注意：Node类型空间物件专用方法
 * @param spatial 指定的空间节点指针
 * @return 子空间物件列表
 */
PiVector* PI_API pi_node_get_children(PiSpatial* spatial);

/**
* 简单的spatial更新
* 注意：该方法目前仅仅用于绑定物体的更新，
*       且消耗并没有降低多少，后续有好的方案可以考虑废弃或重写
* @param spatial 指定的空间节点指针
*/
void PI_API pi_spatial_update_simple(PiSpatial* spatial);

PI_END_DECLS

#endif /* __PI_SPATIAL_H__ */

