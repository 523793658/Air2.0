#ifndef __INCLUDE_SCENE_H__
#define __INCLUDE_SCENE_H__

#include <pi_obb.h>
#include <pi_spatial.h>

#include "octree.h"

/**
 * 本文件定义了一个基于八叉树的场景管理器，提供场景物件的信息管理以及快速查找
 * 场景物件可以包含3类空间信息：
 * 1.点（point），2.轴对齐包围盒（AABB），3.定向包围盒（OBB）
 * 其中前一项视为后一项的特殊子集，用户可以选择性的设置其中的一项或多项，未设置项会根据已设置信息自动得出
 * 以支持对应精度的查询正确
 * 例如：一个物件设置了AABB而未设置OBB和point，其同样可以以OBB或Point精度被正确的查询
 */

typedef PiHandle S3dObjHandle;

typedef enum
{
	EQA_POINT = 0,
	EQA_AABB = 1,
	EQA_OBB = 2
} S3dQueryAccuracy;

typedef struct
{
	S3dOctree *octree; // 八叉树

	//对象分配器
	PiHandleAllocator *obj_alloc;
} S3dScene;

PI_BEGIN_DECLS

/**
 * 使用指定的场景大小，创建场景管理器
 * @param min_x 场景X方向最小值
 * @param min_y 场景Y方向最小值
 * @param min_z 场景Z方向最小值
 * @param max_x 场景X方向最大值
 * @param max_y 场景Y方向最大值
 * @param max_z 场景Z方向最大值
 * @return 场景管理器指针
 */
S3dScene *PI_API s3d_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * 销毁指定的创建场景管理器
 * @param mgr 指定的场景管理器
 */
void PI_API s3d_destroy(S3dScene *mgr);

/**
 * 拷贝指定的创建场景管理器
 * @param dst 目标场景管理器
 * @param src 原始场景管理器
 */
void PI_API s3d_copy(S3dScene *dst, S3dScene *src);

/**
 * 更新指定的创建场景管理器，整理内部数据结构，推荐在大规模场景物件更新完成后，大规模查询开始前执行
 * @param mgr 目标场景管理器
 */
void PI_API s3d_update(S3dScene *mgr);

/**
 * 由指定的场景管理器中创建一个场景物件
 * 注意：创建出的物件并未立即加入场景管理其中管理，而仅仅分配其句柄和结构初始化
 * 实际加入管理器需调用s3d_obj_insert
 * @param mgr 目标场景管理器
 * @return 场景物件句柄
 */
uint PI_API s3d_obj_create(S3dScene *mgr, PiSpatial* spatial);

/**
 * 销毁指定的场景物件
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 */
void PI_API s3d_obj_destroy(S3dScene *mgr, uint id);

/**
 * 向场景中插入指定的场景物件,插入前需要保证物件的空间属性已被设置
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 */
void PI_API s3d_obj_insert(S3dScene *mgr, uint id);

/**
 * 从场景中移除指定物件
 * 注意：此方法并不销毁物件
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 */
void PI_API s3d_obj_remove(S3dScene *mgr, uint id);

/**
 * 获取物件的spatial
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 */
PiSpatial *PI_API s3d_obj_get_spatial(S3dScene *mgr, uint id);

/**
 * 获取场景物件的点（Point）类型空间信息
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param pos 接收物件点空间信息
 */
void PI_API s3d_obj_get_point(S3dScene *mgr, uint id, float *pos);

/**
 * 获取场景物件的轴对齐包围盒（AABB）类型空间信息
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param aabb 接收物件轴对齐包围盒空间信息
 */
void PI_API s3d_obj_get_aabb(S3dScene *mgr, uint id, PiAABBBox *args);

/**
 * 获取场景物件的定向包围盒（OBB）类型空间信息
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param obb 接收物件定向包围盒空间信息
 */
void PI_API s3d_obj_get_obb(S3dScene *mgr, uint id, PiOBBBox *obb);

/**
 * 设置场景物件的点（Point）类型空间信息，NULL为取消设置由管理器自动依据其他信息生成
 * 注意：如果已成功执行过s3d_obj_insert的物件，其Point/AABB/OBB均被后续设置为NULL将导致此物件被执行s3d_obj_remove
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param position 点空间信息
 */
void PI_API s3d_obj_set_point(S3dScene *mgr, uint id, PiVector3 *position);

/**
 * 设置场景物件的轴对齐包围盒（AABB）类型空间信息，NULL为取消设置由管理器自动依据其他信息生成
 * 注意：如果已成功执行过s3d_obj_insert的物件，其Point/AABB/OBB均被后续设置为NULL将导致此物件被执行s3d_obj_remove
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param position 轴对齐包围盒信息
 */
void PI_API s3d_obj_set_aabb(S3dScene *mgr, uint id, PiAABBBox *aabb);

/**
 * 设置场景物件的定向包围盒（OBB）类型空间信息，NULL为取消设置由管理器自动依据其他信息生成
 * 注意：如果已成功执行过s3d_obj_insert的物件，其Point/AABB/OBB均被后续设置为NULL将导致此物件被执行s3d_obj_remove
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param position 定向包围盒信息
 */
void PI_API s3d_obj_set_obb(S3dScene *mgr, uint id, PiOBBBox *obb);

/**
 * 获取场景物件的查询掩码
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @return 查询掩码
 */
uint PI_API s3d_obj_get_mask(S3dScene *mgr, uint id);

/**
 * 设置场景物件的查询掩码
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param mask 查询掩码
 */
void PI_API s3d_obj_set_mask(S3dScene *mgr, uint id, uint mask);

/**
 * 获取场景物件的链接信息指针
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @return 链接信息指针
 */
void *PI_API s3d_obj_get_link(S3dScene *mgr, uint id);

/**
 * 设置场景物件的链接信息指针
 * @param mgr 目标场景管理器
 * @param id 场景物件句柄
 * @param link 链接信息指针
 */
void PI_API s3d_obj_set_link(S3dScene *mgr, uint id, void *link);

/**
 * 在场景中执行掩码查询
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 */
void PI_API s3d_query(S3dScene *mgr, uint mask, PiVector *list);

/**
 * 在场景中执行线段查询
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 * @param line 线段查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_query_line(S3dScene *mgr, uint mask, PiVector *list, PiLineSegment *line, S3dQueryAccuracy accuracy);

/**
 * 在场景中执行视椎体查询
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 * @param frustum 视椎查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_query_frustum(S3dScene *mgr, uint mask, PiVector *list, PiFrustum *frustum, S3dQueryAccuracy accuracy);

/**
 * 在场景中执行视椎体查询，将link加入到list中
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器，放的是link指针 注意：方法不会主动清空该容器
 * @param frustum 视椎查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_query_link_frustum(S3dScene *mgr, uint mask, PiVector *list, PiFrustum *frustum, S3dQueryAccuracy accuracy);

/**
 * 在场景中执行球查询
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 * @param sphere 球查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_query_sphere(S3dScene *mgr, uint mask, PiVector *list, PiSphere *sphere, S3dQueryAccuracy accuracy);

/**
 * 在场景中执行AABB立方体查询
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 * @param aabb AABB立方查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_query_aabb(S3dScene *mgr, uint mask, PiVector *list,  PiAABBBox *aabb, S3dQueryAccuracy accuracy);

/**
 * 在场景中执行OBB立方体查询
 * @param mgr 目标场景管理器
 * @param mask 查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 * @param obb OBB立方查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_query_obb(S3dScene *mgr, uint mask, PiVector *list,  PiOBBBox *obb, S3dQueryAccuracy accuracy);

/**
 * 在场景中使用指定的物件进行碰撞检测
 * @param mgr 目标场景管理器
 * @param mask 参与碰撞检测的查询掩码
 * @param list 查询结果容器 注意：方法不会主动清空该容器
 * @param id 进行检测的查询体
 * @param accuracy 查询精度
 */
void PI_API s3d_collision(S3dScene *mgr, uint mask, PiVector *list,  uint id, S3dQueryAccuracy accuracy);

/**
 * 在场景中使用指定的物件列表进行群碰撞检测
 * @param mgr 目标场景管理器
 * @param mask 参与碰撞检测的查询掩码
 * @param list 查询结果容器 ,起始第一个元素为碰撞主体（id_list中的物件之一），其后物件为器碰撞检测结果，
 * 每个物件碰撞检测结束在队列中填入一个NULL，之后为下一个碰撞物件的碰撞结果，例如：{(a, e, f, c, null), (b, c, t, null)...}
 * 注意：方法不会主动清空该容器
 * @param obb OBB立方查询体指针
 * @param accuracy 查询精度
 */
void PI_API s3d_collision_group(S3dScene *mgr, uint mask, PiVector *list,  PiVector *id_list, S3dQueryAccuracy accuracy);

/**
 * 创建一个线段查询体
 * @return 查询体指针
 */
PiLineSegment *PI_API s3d_query_obj_line_create();

/**
 * 创建一个视椎查询体
 * @return 查询体指针
 */
PiFrustum *PI_API s3d_query_obj_frustum_create();

/**
 * 创建一个球查询体
 * @return 查询体指针
 */
PiSphere *PI_API s3d_query_obj_sphere_create();

/**
 * 创建一个AABB立方查询体
 * @return 查询体指针
 */
PiAABBBox *PI_API s3d_query_obj_aabb_create();

/**
 * 创建一个OBB立方查询体
 * @return 查询体指针
 */
PiOBBBox *PI_API s3d_query_obj_obb_create();


/**
 * 更新指定的线段查询体
 * @param queryObj 查询体指针
 * @param start_x 线段起点坐标X分量
 * @param start_y 线段起点坐标Y分量
 * @param start_z 线段起点坐标Z分量
 * @param end_x 线段终点坐标X分量
 * @param end_y 线段终点坐标Y分量
 * @param end_z 线段终点坐标Z分量
 */
void PI_API s3d_query_obj_line_update(void *queryObj, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z);

/**
 * 更新指定的视椎查询体
 * @param queryObj 查询体指针
 * @param viewMat 视椎体视图矩阵
 * @param projMat 视椎体投影矩阵
 */
void PI_API s3d_query_obj_frustum_update_matrix(void *queryObj, PiMatrix4 *viewMat, PiMatrix4 *projMat);

/**
 * 更新指定的视椎查询体
 * @param queryObj 查询体指针
 * @param pos 观察点
 * @param dir 观察方向
 * @param up 向上向量
 * @param left 近裁剪面left
 * @param right 近裁剪面right
 * @param bottom 近裁剪面bottom
 * @param top 近裁剪面top
 * @param near 近裁剪面距离
 * @param far 远裁剪面距离
 * @param is_ortho 是否以正交投影构建
 */
void PI_API s3d_query_obj_frustum_update(void *queryObj, PiVector3 *pos, PiVector3 *dir, PiVector3 *up, float left, float right, float bottom, float top, float near, float far, PiBool is_ortho);

/**
 * 更新指定的球查询体
 * @param queryObj 查询体指针
 * @param pos_x 球心位置坐标X分量
 * @param pos_y 球心位置坐标Y分量
 * @param pos_Z 球心位置坐标Z分量
 * @param radius 球半径
 */
void PI_API s3d_query_obj_sphere_update(void *queryObj, float pos_x, float pos_y, float pos_z, float radius);

/**
 * 更新指定的AABB立方查询体
 * @param queryObj 查询体指针
 * @param min_x 立方体负轴点坐标X分量
 * @param min_y 立方体负轴点坐标Y分量
 * @param min_z 立方体负轴点坐标Z分量
 * @param max_x 立方体正轴点坐标X分量
 * @param max_y 立方体正轴点坐标Y分量
 * @param max_z 立方体正轴点坐标Z分量
 */
void PI_API s3d_query_obj_aabb_update(void *queryObj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * 更新指定的OBB立方查询体
 * TODO:将参数拆为多个更新方法
 * @param queryObj 查询体指针
 * @param center_x OBB中心点坐标X分量
 * @param center_y OBB中心点坐标Y分量
 * @param center_z OBB中心点坐标Z分量
 * @param axis_0_x OBB第0号轴X分量
 * @param axis_0_y OBB第0号轴Y分量
 * @param axis_0_z OBB第0号轴Z分量
 * @param axis_1_x OBB第1号轴X分量
 * @param axis_1_y OBB第1号轴Y分量
 * @param axis_1_z OBB第1号轴Z分量
 * @param axis_2_x OBB第2号轴X分量
 * @param axis_2_y OBB第2号轴Y分量
 * @param axis_2_z OBB第2号轴Z分量
 * @param extent_0 OBB第0号半轴长
 * @param extent_1 OBB第1号半轴长
 * @param extent_2 OBB第2号半轴长
 */
void PI_API s3d_query_obj_obb_update(void *queryObj, float center_x, float center_y, float center_z, float axis_0_x, float axis_0_y, float axis_0_z, float axis_1_x, float axis_1_y, float axis_1_z, float axis_2_x, float axis_2_y, float axis_2_z, float extent_0, float extent_1, float extent_2);


/**
 * 销毁指定的查询体
 * @param queryObj 查询体指针
 */
void PI_API s3d_query_obj_destroy(void *queryObj);

PI_END_DECLS

#endif
