#ifndef __INCLUDE_OCTREE_H__
#define __INCLUDE_OCTREE_H__

#include <pi_lib.h>
#include <pi_aabb.h>
#include <pi_obb.h>
#include <pi_intersection.h>

typedef PiHandle S3dOTObjHandle;
typedef PiHandle _S3dOTNodeHandle;

typedef struct
{
	_S3dOTNodeHandle root; // 八叉树根节点
	PiHandleAllocator *objAlloc; // 物件对象分配器
	PiHandleAllocator *nodeAlloc; // 八叉树节点分配器
} S3dOctree;

typedef EIntersectState (*NodeFilter)(PiAABBBox *aabb, void *query_obj, void *user_data);
typedef PiBool (*ObjFilter)(S3dOTObjHandle obj, void *query_obj, void *user_data);
typedef void *(*ResultOperation)(S3dOTObjHandle obj, void *user_data);

PI_BEGIN_DECLS

/**
 * 使用指定的大小，创建八叉树
 * @param min_x 场景X方向最小值
 * @param min_y 场景Y方向最小值
 * @param min_z 场景Z方向最小值
 * @param max_x 场景X方向最大值
 * @param max_y 场景Y方向最大值
 * @param max_z 场景Z方向最大值
 * @return 八叉树指针
 */
S3dOctree *ot_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);
void ot_destroy(S3dOctree *ot);
void ot_update(S3dOctree *ot);
void ot_copy(S3dOctree *dst, S3dOctree *src);

/**
 * 创建一个指定大小的八叉树物件，注意：必须在执行Insert后物件才会被放入八叉树中
 * @param ot 八叉树指针
 * @param user_data 物件的附加用户信息
 * @param min_x 物件AABB的X分量最小值
 * @param min_y 物件AABB的Y分量最小值
 * @param min_z 物件AABB的Z分量最小值
 * @param max_x 物件AABB的X分量最大值
 * @param max_y 物件AABB的Y分量最大值
 * @param max_z 物件AABB的Z分量最大值
 * @return 物件句柄
 */
S3dOTObjHandle ot_obj_create(S3dOctree *ot, void *user_data, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * 销毁指定的八叉树物件，如果物件位于八叉树中会先执行移除操作
 * @param ot 八叉树指针
 * @param id 物件句柄
 */
void ot_obj_destroy(S3dOctree *ot, S3dOTObjHandle id);

/**
 * 将指定的物件插入至八叉树中
 * @param ot 八叉树指针
 * @param id 物件句柄
 */
void ot_obj_insert(S3dOctree *ot, S3dOTObjHandle id);

/**
 * 将指定的物件由八叉树中移除
 * @param ot 八叉树指针
 * @param id 物件句柄
 */
void ot_obj_remove(S3dOctree *ot, S3dOTObjHandle id);

/**
 * 更新指定的物件的空间信息
 * @param ot 八叉树指针
 * @param id 物件句柄
 * @param min_x 物件AABB的X分量最小值
 * @param min_y 物件AABB的Y分量最小值
 * @param min_z 物件AABB的Z分量最小值
 * @param max_x 物件AABB的X分量最大值
 * @param max_y 物件AABB的Y分量最大值
 * @param max_z 物件AABB的Z分量最大值
 */
void ot_obj_update(S3dOctree *ot, S3dOTObjHandle id, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * 获取指定物件的AABB空间信息
 * @param ot 八叉树指针
 * @param id 物件句柄
 * @return 物件AABB包围盒指针
 */
PiAABBBox *ot_obj_get_aabb(S3dOctree *ot, S3dOTObjHandle id);

/**
 * 获取指定物件的附加用户信息
 * @param ot 八叉树指针
 * @param id 物件句柄
 * @return 物件的附加用户信息指针
 */
void *ot_obj_get_user_data(S3dOctree *ot, S3dOTObjHandle id);

/**
 * 获取指定物件的查询掩码
 * @param ot 八叉树指针
 * @param id 物件句柄
 * @return 物件的查询掩码
 */
uint ot_obj_get_mask(S3dOctree *ot, S3dOTObjHandle id);

/**
 * 设置指定物件的物件的查询掩码
 * @param ot 八叉树指针
 * @param id 物件句柄
 * @param mask 物件的查询掩码
 */
void ot_obj_set_mask(S3dOctree *ot, S3dOTObjHandle id, uint mask);

/**
 * 使用指定的查询体查询八叉树
 * @param ot 八叉树指针
 * @param node_filter 节点相交测试函数指针
 * @param obj_filter 物件相交测试函数指针
 * @param mask 物件的查询掩码
 * @param list 查询结果缓存
 * @param query_obj 查询体指针
 * @param operation 对查询到的物件的处理操作，此操作的返回结果放入查询结果集
 * @param user_data 物件句柄
 * @return 查询完成后结果集的大小
 */
uint ot_query(S3dOctree *ot, NodeFilter node_filter, ObjFilter obj_filter, uint mask, PiVector *list, void *query_obj, ResultOperation operation, void *user_data);

PI_END_DECLS

#endif
