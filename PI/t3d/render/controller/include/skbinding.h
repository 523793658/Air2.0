#ifndef INCLUDE_SK_BINDING_H
#define INCLUDE_SK_BINDING_H

#include <controller.h>
#include <pi_skeleton.h>
#include <pi_aabb.h>
#include <pi_spatial.h>

//��������
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
	PiSpatial *spatial; /* �󶨵�spatial */
	BindType type;  /* �󶨵����� */
	PiBool offset_enable; /* ƫ�ƿ��� */
	PiVector3 offset_translation; /* �󶨵��ƶ� */
	PiQuaternion offset_rotation; /* �󶨵���ת */
	PiVector3 offset_scaling; /* �󶨵����� */
	PiMatrix4 offset_matrix; /* �󶨵�ƫ�ƾ��� */
} BindingNode;

/**
 * ��ͷ�󶨿�����
 */

PI_BEGIN_DECLS

/**
 * ����
 */
PiController* PI_API pi_skbinding_new();

/**
 * �ͷ�
 */
void PI_API pi_skbinding_free(PiController *c);

/**
 * ���ù�ͷ
 */
PiBool PI_API pi_skbinding_set_bone(PiController *c, PiController *skanim, const char *bone_name, PiSpatial *spatial, PiMatrix4 *local_matrix);

/**
* �½�һ���󶨽ڵ�
*/
BindingNode* PI_API pi_binding_node_new(PiSpatial* spatial, BindType type);

/**
* �ͷ�һ���󶨽ڵ�
*/
void PI_API pi_binding_free(BindingNode* bindNode);

/**
* ���ð󶨽ڵ�İ�ƫ��
*/
void PI_API pi_set_binding_offset_translation(BindingNode* node, float x, float y, float z);

/**
* ���ð󶨽ڵ������ƫ��
*/
void PI_API pi_set_binding_offset_scale(BindingNode* node, float x, float y, float z);

/**
* ���ð󶨽ڵ����ת
*/
void PI_API pi_set_binding_offset_rotation(BindingNode* node, float x, float y, float z, float w);

/**
* Ӧ�ð󶨽ڵ��ƫ�ƾ���
*/
void PI_API pi_apply_binding_offset(BindingNode* node);

PI_END_DECLS

#endif /* INCLUDE_SK_BINDING_H */