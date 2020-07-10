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
 * ���ļ������˻���Opcode����ײ����
 */

//��ײģ��
typedef struct  
{
	 Opcode::Model* opc_model;
	 IceMaths::Matrix4x4 world_matrix;
}PiCollisionModel;

PI_BEGIN_DECLS

/**
 * ��������world_matrix����ײģ��
 * @param mesh ���ڴ�����ײģ�͵�����
 * @return ��ײģ��ָ��
 */
Opcode::Model* PI_API pi_collision_obj_new(PiMesh* mesh);

/**
* �ͷ�Opcode::Modelָ��
*/
void PI_API pi_collision_obj_free(Opcode::Model* opc_model);


/**
* ͨ��Opcode::Modelָ���齨��һ��PiCollisionModeld����
*/
PiCollisionModel* PI_API pi_collision_model_build(Opcode::Model* opc_model);

/*
*���PiCollisionModeld����
*/
void PI_API pi_collision_model_destroy(PiCollisionModel* model);

/**
 * ������ײģ��
 * @param mesh ���ڴ�����ײģ�͵�����
 * @return ��ײģ��ָ��
 */
PiCollisionModel* PI_API pi_collision_model_new(PiMesh* mesh);

/**
 * ʹ��ָ����������´���ײģ��
 * @param mesh ���ڸ�����ײ�������
 */
void PI_API pi_collision_model_update_mesh(PiCollisionModel* model, PiMesh* mesh);

/**
 * ʹ��ָ���ľ�����´���ײ�������ռ�任
 * @param model ��Ҫ���µ���ײģ��
 * @param world_matrix ����ռ�任����
 */
void PI_API pi_collision_model_update_transform(PiCollisionModel* model, PiMatrix4* world_matrix);

/**
 * �ͷ���ײģ��
 * @param model ��Ҫ�ͷŵ���ײģ��
 * @return ��ײģ��ָ��
 */
void PI_API pi_collision_model_free(PiCollisionModel* model);

/**
 * ����������ײ���
 * @param model Ҫ������ײ����ģ��
 * @param start_x �������X����
 * @param start_y �������Y����
 * @param start_z �������Z����
 * @param end_x �����յ�X����
 * @param end_y �����յ�Y����
 * @param end_z �����յ�Z����
 * @param first_contact �Ƿ���ȷ�ϵ��κ���ײʱ���ٷ��أ��������������ֻ��Ҫȷ����ײ�������������Ч��
 * @param closest_hit �Ƿ�ֻ����������������ײ�㣬���������stopOnFirstΪfalseʱ��������
 * @param hit_pront_buffer ��ײ������������
 * @param hit_normal_buffer ��ײ��������ķ��߷��������棬���ΪNULL�򲻽��з��߲�ѯ�����Խ�ʡЧ��
 */
void PI_API pi_collision_ray_collide(PiCollisionModel* model, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, PiBool first_contact, PiBool closest_hit, PiBytes* hit_pront_buffer, PiBytes* hit_normal_buffer);

/**
 * ����ģ����ײ���
 * @param model0 ��ײģ��0
 * @param model1 ��ײģ��1
 * @return �Ƿ�����ײ
 */
PiBool PI_API pi_collision_model_collide(PiCollisionModel* model0, PiCollisionModel* model1);

PI_END_DECLS

#endif /* __PI_COLLISION_H__ */