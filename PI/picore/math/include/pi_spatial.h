#ifndef __PI_SPATIAL_H__
#define __PI_SPATIAL_H__

#include <pi_vector3.h>
#include <pi_aabb.h>
#include <pi_obb.h>
#include <pi_quaternion.h>
#include <pi_matrix4.h>

/**
 * ���ļ������˿ռ伸���Լ��ռ�ڵ�
 * �ռ������ṩ�˿ռ�任�Ĺ��ܣ�ͬʱ���ڶ������������Ĳ�ͬ����Ϊ2�ࣺ
 * 1.�����壨Geometry��������ӵ�м��ΰ�Χ������
 * 2.�ڵ㣨Node���������޷�ӵ�м��ΰ�Χ����Ϣ��������ͨ��
 * ������Ӱ����˼�����Ϣ��Child�ķ�ʽ���������Child��ͬ��ϳ��Ŀռ���Ϣ
 */

typedef enum 
{
	EST_GEOMETRY,
	EST_NODE
}PiSpatialType;

/**
 * �������ͣ�parent�ĸ�������
 */
typedef enum
{
	ESAT_NONE,	/* �޸������� */
	ESAT_NODE,	/* ͨ��node���� */
	ESAT_SYNC,	/* ͨ��ͬ������ */
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

	PiVector children;					/* Ԫ��������PiSpatial* */
	struct PiSpatial *parent;			/* ���� */
	PiSpatialAttachType attach_type;	/* ���ӵ����� */

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
	char dummy;	/* Ԥ����Ա�����õ���Ԫ */
}_Node;

PI_BEGIN_DECLS

/**
 * ����һ���ռ伸����
 * @return ������ָ��
 */
PiSpatial* PI_API pi_spatial_geometry_create();

/**
 * ����һ���ռ����
 * @return �ռ����ָ��
 */
PiSpatial* PI_API pi_spatial_node_create();

/**
 * ����һ���ռ�ڵ�
 * @return �ռ�ڵ�ָ��
 */
void PI_API pi_spatial_destroy(PiSpatial* spatial);

/**
 * ���ظÿռ����������
 * @param spatial ָ���Ŀռ�����ָ��
 * @return �ÿռ����������
 */
PiSpatialType PI_API pi_spatial_get_type(PiSpatial* spatial);

/**
 * ����ͬ�����ͣ���src����dst�󣬵�����dstʱ��src�ͻ´��dst������������
 * ע��һ��srcͬ����dst����src��ƽ�ƣ���ת�����ţ�����AABB��ȫ��dstһ����
 * ע��һ��srcͬ����dst����src��OBB��������!
 */
void PI_API pi_spatial_attach_sync(PiSpatial *dst, PiSpatial *src);

/**
* ���src��dst��ͬ��
*/
void PI_API pi_spatial_detach_sync(PiSpatial *dst, PiSpatial *src);

/*Ϊnode�ڵ����ó�ʼ��aabb*/
void PI_API pi_spatial_join_basic_aabb(PiSpatial* spatial, PiAABBBox* aabb);

/**
* �Ƴ�ָ���ռ����е��ӿռ����
* @param spatial ָ���Ŀռ�ڵ�ָ��
*/
void PI_API pi_spatial_detach_children(PiSpatial* spatial);

/**
 * ����ָ���ռ�����ľֲ��ռ�ƽ��
 * @param spatial ָ���Ŀռ�����ָ��
 * @param x ƽ�Ƶ�X����
 * @param y ƽ�Ƶ�Y����
 * @param z ƽ�Ƶ�Z����
 */
void PI_API pi_spatial_set_local_translation(PiSpatial* spatial, float x, float y, float z);

/**
 * ����ָ���ռ�����ľֲ��ռ���ת
 * ע�⣺�������Ԫ��������Ҫ��֤�Ѿ���һ��
 * @param spatial ָ���Ŀռ�����ָ��
 * @param w ��ת��Ԫ����W����
 * @param x ��ת��Ԫ����X����
 * @param y ��ת��Ԫ����Y����
 * @param z ��ת��Ԫ����Z����
 */
void PI_API pi_spatial_set_local_rotation(PiSpatial* spatial, float w, float x, float y, float z);

/**
 * ����ָ���ռ�����ľֲ��ռ�����
 * @param spatial ָ���Ŀռ�����ָ��
 * @param x ���ŵ�X����
 * @param y ���ŵ�Y����
 * @param z ���ŵ�Z����
 */
void PI_API pi_spatial_set_local_scaling(PiSpatial* spatial, float x, float y, float z);

/**
 * ��ȡָ���ռ�����ľֲ��ռ�ƽ��
 * @param spatial ָ���Ŀռ�����ָ��
 * @return �ֲ��ռ�ƽ��
 */
PiVector3* PI_API pi_spatial_get_local_translation(PiSpatial* spatial);

/**
 * ��ȡָ���ռ�����ľֲ��ռ���ת
 * @param spatial ָ���Ŀռ�����ָ��
 * @return �ֲ��ռ���ת
 */
PiQuaternion* PI_API pi_spatial_get_local_rotation(PiSpatial* spatial);

/**
 * ��ȡָ���ռ�����ľֲ��ռ�����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return �ֲ��ռ�����
 */
PiVector3* PI_API pi_spatial_get_local_scaling(PiSpatial* spatial);

/**
 * ��ȡָ���ռ�����ľֲ��ռ�任����
 * ע�⣺���������Ŀռ�任�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return �ֲ��ռ�任����
 */
PiMatrix4* PI_API pi_spatial_get_local_transform(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����λ�ھֲ��ռ��AABB��Χ��
 * @param spatial ָ���Ŀռ�����ָ��
 * @return �ֲ��ռ�AABB��Χ��
 */
PiAABBBox* PI_API pi_spatial_get_local_aabb(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����������ռ�ƽ��
 * ע�⣺���������Ŀռ�任�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return ����ռ�ƽ��
 */
PiVector3* PI_API pi_spatial_get_world_translation(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����������ռ���ת
 * ע�⣺���������Ŀռ�任�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return ����ռ���ת
 */
PiQuaternion* PI_API pi_spatial_get_world_rotation(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����������ռ�����
 * ע�⣺���������Ŀռ�任�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return ����ռ�����
 */
PiVector3* PI_API pi_spatial_get_world_scaling(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����������ռ�任����
 * ע�⣺���������Ŀռ�任�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return ����ռ�任����
 */
PiMatrix4* PI_API pi_spatial_get_world_transform(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����λ������ռ��AABB��Χ��
 * ע�⣺���������Ŀռ�任���Χ����Ϣ�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ�����ָ��
 * @return ����ռ�AABB��Χ��
 */
PiAABBBox* PI_API pi_spatial_get_world_aabb(PiSpatial* spatial);

/**
 * ��ȡָ���ռ�����ĸ��ڵ�
 * @param spatial ָ���Ŀռ�����ָ��
 * @return ���ڵ�
 */
PiSpatial* PI_API pi_spatial_get_parent(PiSpatial* spatial);

/**
 * ��ָ���Ŀռ�������丸�׽ڵ����Ƴ�
 * @param spatial ָ���Ŀռ�����ָ��
 */
void PI_API pi_spatial_detach_from_parent(PiSpatial* spatial);

/**
 * ����ָ���Ŀռ�������������¼�����任�Լ���Χ����Ϣ
 * ע�⣺����get����ִ��ǰ��Ҫ��֤�����Ѿ�����ȷִ�к󷽿ɵ���
 * @param spatial ָ���Ŀռ�����ָ��
 */
void PI_API pi_spatial_update(PiSpatial* spatial);

/**
 * ���ô˿ռ������Update��ִ�к�ĸ��Ӳ���
 * @param spatial ָ���Ŀռ�����ָ��
 * @param operation_func ִ�еĲ����ĺ���ָ��
 * @param user_data �û�����
 * @param is_check �Ƿ���mask
 */
void PI_API pi_spatial_set_update_operation(PiSpatial* spatial, OnUpdateFunc operation_func, void* user_data, PiBool is_check);

/**
* ɾ���˿ռ������Update��ִ�к�ĸ��Ӳ���
* @param spatial ָ���Ŀռ�����ָ��
* @param user_data �û����ݣ�ͨ���û�����ָ���ж�����һ����
*/
void PI_API pi_spatial_remove_update_operation(PiSpatial* spatial, void* user_data);

/**
 * ����ָ���Ŀռ伸����ľֲ��ռ�AABB��Χ�У��������ΪNULL������OBBʱ��ʹ��OBB����������AABB����
 * ע�⣺Geometry���Ϳռ����ר�÷���
 * @param spatial ָ���Ŀռ伸����ָ��
 * @param aabb ָ����aabb��Χ��
 */
void PI_API pi_geometry_set_local_aabb(PiSpatial* spatial, PiAABBBox* aabb);

/**
 * ����ָ���Ŀռ伸����ľֲ��ռ�OBB��Χ��,����ΪNULL��ʾ����OBB���ã�����ͨ������AABBΪNULL��OBB�������AABB
 * ע�⣺Geometry���Ϳռ����ר�÷���
 * @param spatial ָ���Ŀռ伸����ָ��
 * @param obb ָ����OBB��Χ��
 */
void PI_API pi_geometry_set_local_obb(PiSpatial* spatial, PiOBBBox* obb);

/**
 * ��ȡָ���ռ����λ�ھֲ��ռ��OBB��Χ��
 * ע�⣺Geometry���Ϳռ����ר�÷���
 * @param spatial ָ���Ŀռ伸����ָ��
 * @return �ֲ��ռ�OBB��Χ��
 */
PiOBBBox* PI_API pi_geometry_get_local_obb(PiSpatial* spatial);

/**
 * ��ȡָ���ռ����λ������ռ��OBB��Χ��
 * ע�⣺Geometry���Ϳռ����ר�÷���
 * ע�⣺���������Ŀռ�任���Χ����Ϣ�Ѿ������ı䣬�ڴ˷�������ǰӦ����pi_spatial_update����
 * @param spatial ָ���Ŀռ伸����ָ��
 * @return ����ռ�OBB��Χ��
 */
PiOBBBox* PI_API pi_geometry_get_world_obb(PiSpatial* spatial);

/**
 * ��ָ���ռ�ڵ������һ���ӿռ����
 * ע�⣺Node���Ϳռ�ڵ�ר�÷���
 * @param spatial ָ���Ŀռ�ڵ�ָ��
 * @param spatial ָ�����ӿռ����ָ��
 */
void PI_API pi_node_attach_child(PiSpatial* spatial, PiSpatial* child);

/**
 * ��ָ���ռ�ڵ����Ƴ�ָ�����ӿռ����
 * ע�⣺Node���Ϳռ�ڵ�ר�÷���
 * @param spatial ָ���Ŀռ�ڵ�ָ��
 * @param spatial ָ�����ӿռ����ָ��
 */
void PI_API pi_node_detach_child(PiSpatial* spatial, PiSpatial* child);

/**
 * ��ȡָ���ռ�������ӿռ�����б�
 * ע�⣺Node���Ϳռ����ר�÷���
 * @param spatial ָ���Ŀռ�ڵ�ָ��
 * @return �ӿռ�����б�
 */
PiVector* PI_API pi_node_get_children(PiSpatial* spatial);

/**
* �򵥵�spatial����
* ע�⣺�÷���Ŀǰ�������ڰ�����ĸ��£�
*       �����Ĳ�û�н��Ͷ��٣������кõķ������Կ��Ƿ�������д
* @param spatial ָ���Ŀռ�ڵ�ָ��
*/
void PI_API pi_spatial_update_simple(PiSpatial* spatial);

PI_END_DECLS

#endif /* __PI_SPATIAL_H__ */

