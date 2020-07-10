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
	_S3dOTNodeHandle root; // �˲������ڵ�
	PiHandleAllocator *objAlloc; // ������������
	PiHandleAllocator *nodeAlloc; // �˲����ڵ������
} S3dOctree;

typedef EIntersectState (*NodeFilter)(PiAABBBox *aabb, void *query_obj, void *user_data);
typedef PiBool (*ObjFilter)(S3dOTObjHandle obj, void *query_obj, void *user_data);
typedef void *(*ResultOperation)(S3dOTObjHandle obj, void *user_data);

PI_BEGIN_DECLS

/**
 * ʹ��ָ���Ĵ�С�������˲���
 * @param min_x ����X������Сֵ
 * @param min_y ����Y������Сֵ
 * @param min_z ����Z������Сֵ
 * @param max_x ����X�������ֵ
 * @param max_y ����Y�������ֵ
 * @param max_z ����Z�������ֵ
 * @return �˲���ָ��
 */
S3dOctree *ot_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);
void ot_destroy(S3dOctree *ot);
void ot_update(S3dOctree *ot);
void ot_copy(S3dOctree *dst, S3dOctree *src);

/**
 * ����һ��ָ����С�İ˲��������ע�⣺������ִ��Insert������Żᱻ����˲�����
 * @param ot �˲���ָ��
 * @param user_data ����ĸ����û���Ϣ
 * @param min_x ���AABB��X������Сֵ
 * @param min_y ���AABB��Y������Сֵ
 * @param min_z ���AABB��Z������Сֵ
 * @param max_x ���AABB��X�������ֵ
 * @param max_y ���AABB��Y�������ֵ
 * @param max_z ���AABB��Z�������ֵ
 * @return ������
 */
S3dOTObjHandle ot_obj_create(S3dOctree *ot, void *user_data, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * ����ָ���İ˲��������������λ�ڰ˲����л���ִ���Ƴ�����
 * @param ot �˲���ָ��
 * @param id ������
 */
void ot_obj_destroy(S3dOctree *ot, S3dOTObjHandle id);

/**
 * ��ָ��������������˲�����
 * @param ot �˲���ָ��
 * @param id ������
 */
void ot_obj_insert(S3dOctree *ot, S3dOTObjHandle id);

/**
 * ��ָ��������ɰ˲������Ƴ�
 * @param ot �˲���ָ��
 * @param id ������
 */
void ot_obj_remove(S3dOctree *ot, S3dOTObjHandle id);

/**
 * ����ָ��������Ŀռ���Ϣ
 * @param ot �˲���ָ��
 * @param id ������
 * @param min_x ���AABB��X������Сֵ
 * @param min_y ���AABB��Y������Сֵ
 * @param min_z ���AABB��Z������Сֵ
 * @param max_x ���AABB��X�������ֵ
 * @param max_y ���AABB��Y�������ֵ
 * @param max_z ���AABB��Z�������ֵ
 */
void ot_obj_update(S3dOctree *ot, S3dOTObjHandle id, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * ��ȡָ�������AABB�ռ���Ϣ
 * @param ot �˲���ָ��
 * @param id ������
 * @return ���AABB��Χ��ָ��
 */
PiAABBBox *ot_obj_get_aabb(S3dOctree *ot, S3dOTObjHandle id);

/**
 * ��ȡָ������ĸ����û���Ϣ
 * @param ot �˲���ָ��
 * @param id ������
 * @return ����ĸ����û���Ϣָ��
 */
void *ot_obj_get_user_data(S3dOctree *ot, S3dOTObjHandle id);

/**
 * ��ȡָ������Ĳ�ѯ����
 * @param ot �˲���ָ��
 * @param id ������
 * @return ����Ĳ�ѯ����
 */
uint ot_obj_get_mask(S3dOctree *ot, S3dOTObjHandle id);

/**
 * ����ָ�����������Ĳ�ѯ����
 * @param ot �˲���ָ��
 * @param id ������
 * @param mask ����Ĳ�ѯ����
 */
void ot_obj_set_mask(S3dOctree *ot, S3dOTObjHandle id, uint mask);

/**
 * ʹ��ָ���Ĳ�ѯ���ѯ�˲���
 * @param ot �˲���ָ��
 * @param node_filter �ڵ��ཻ���Ժ���ָ��
 * @param obj_filter ����ཻ���Ժ���ָ��
 * @param mask ����Ĳ�ѯ����
 * @param list ��ѯ�������
 * @param query_obj ��ѯ��ָ��
 * @param operation �Բ�ѯ��������Ĵ���������˲����ķ��ؽ�������ѯ�����
 * @param user_data ������
 * @return ��ѯ��ɺ������Ĵ�С
 */
uint ot_query(S3dOctree *ot, NodeFilter node_filter, ObjFilter obj_filter, uint mask, PiVector *list, void *query_obj, ResultOperation operation, void *user_data);

PI_END_DECLS

#endif
