/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __PI_COLLISION_H__
#define __PI_COLLISION_H__

#include <Opcode.h>

#include <pi_lib.h>
#include <pi_mesh.h>
#include <pi_vector3.h>

/**
 * 此文件定义了基于Opcode的碰撞功能
 */

//碰撞模型
typedef struct  
{
	 Opcode::Model* opc_model;
	 IceMaths::Matrix4x4 world_matrix;
}PiCollisionModel;

PI_BEGIN_DECLS

/**
 * 创建不带world_matrix的碰撞模型
 * @param mesh 用于创建碰撞模型的网格
 * @return 碰撞模型指针
 */
Opcode::Model* PI_API pi_collision_obj_new(PiMesh* mesh);

/**
* 释放Opcode::Model指针
*/
void PI_API pi_collision_obj_free(Opcode::Model* opc_model);


/**
* 通过Opcode::Model指针组建出一个PiCollisionModeld对象
*/
PiCollisionModel* PI_API pi_collision_model_build(Opcode::Model* opc_model);

/*
*拆分PiCollisionModeld对象
*/
void PI_API pi_collision_model_destroy(PiCollisionModel* model);

/**
 * 创建碰撞模型
 * @param mesh 用于创建碰撞模型的网格
 * @return 碰撞模型指针
 */
PiCollisionModel* PI_API pi_collision_model_new(PiMesh* mesh);

/**
 * 使用指定的网格更新此碰撞模型
 * @param mesh 用于更新碰撞体的网格
 */
void PI_API pi_collision_model_update_mesh(PiCollisionModel* model, PiMesh* mesh);

/**
 * 使用指定的矩阵更新此碰撞体的世界空间变换
 * @param model 需要更新的碰撞模型
 * @param world_matrix 世界空间变换矩阵
 */
void PI_API pi_collision_model_update_transform(PiCollisionModel* model, PiMatrix4* world_matrix);

/**
 * 释放碰撞模型
 * @param model 需要释放的碰撞模型
 * @return 碰撞模型指针
 */
void PI_API pi_collision_model_free(PiCollisionModel* model);

/**
 * 进行射线碰撞检测
 * @param model 要进行碰撞检测的模型
 * @param start_x 射线起点X分量
 * @param start_y 射线起点Y分量
 * @param start_z 射线起点Z分量
 * @param end_x 射线终点X分量
 * @param end_y 射线终点Y分量
 * @param end_z 射线终点Z分量
 * @param first_contact 是否在确认到任何碰撞时快速返回，开启此项可以在只需要确认碰撞与否的情况下提升效率
 * @param closest_hit 是否只返回离起点最近的碰撞点，此项必须在stopOnFirst为false时才能启用
 * @param hit_pront_buffer 碰撞点坐标结果缓存
 * @param hit_normal_buffer 碰撞点所处面的法线方向结果缓存，如果为NULL则不进行法线查询，可以节省效率
 */
void PI_API pi_collision_ray_collide(PiCollisionModel* model, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, PiBool first_contact, PiBool closest_hit, PiBytes* hit_pront_buffer, PiBytes* hit_normal_buffer);

/**
 * 进行模型碰撞检测
 * @param model0 碰撞模型0
 * @param model1 碰撞模型1
 * @return 是否发生碰撞
 */
PiBool PI_API pi_collision_model_collide(PiCollisionModel* model0, PiCollisionModel* model1);

PI_END_DECLS

#endif /* __PI_COLLISION_H__ */