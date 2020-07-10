#ifndef __INCLUDE_SCENE_H__
#define __INCLUDE_SCENE_H__

#include <pi_obb.h>
#include <pi_spatial.h>

#include "octree.h"

/**
 * ���ļ�������һ�����ڰ˲����ĳ������������ṩ�����������Ϣ�����Լ����ٲ���
 * ����������԰���3��ռ���Ϣ��
 * 1.�㣨point����2.������Χ�У�AABB����3.�����Χ�У�OBB��
 * ����ǰһ����Ϊ��һ��������Ӽ����û�����ѡ���Ե��������е�һ�����δ������������������Ϣ�Զ��ó�
 * ��֧�ֶ�Ӧ���ȵĲ�ѯ��ȷ
 * ���磺һ�����������AABB��δ����OBB��point����ͬ��������OBB��Point���ȱ���ȷ�Ĳ�ѯ
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
	S3dOctree *octree; // �˲���

	//���������
	PiHandleAllocator *obj_alloc;
} S3dScene;

PI_BEGIN_DECLS

/**
 * ʹ��ָ���ĳ�����С����������������
 * @param min_x ����X������Сֵ
 * @param min_y ����Y������Сֵ
 * @param min_z ����Z������Сֵ
 * @param max_x ����X�������ֵ
 * @param max_y ����Y�������ֵ
 * @param max_z ����Z�������ֵ
 * @return ����������ָ��
 */
S3dScene *PI_API s3d_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * ����ָ���Ĵ�������������
 * @param mgr ָ���ĳ���������
 */
void PI_API s3d_destroy(S3dScene *mgr);

/**
 * ����ָ���Ĵ�������������
 * @param dst Ŀ�곡��������
 * @param src ԭʼ����������
 */
void PI_API s3d_copy(S3dScene *dst, S3dScene *src);

/**
 * ����ָ���Ĵ��������������������ڲ����ݽṹ���Ƽ��ڴ��ģ�������������ɺ󣬴��ģ��ѯ��ʼǰִ��
 * @param mgr Ŀ�곡��������
 */
void PI_API s3d_update(S3dScene *mgr);

/**
 * ��ָ���ĳ����������д���һ���������
 * ע�⣺�������������δ�������볡���������й������������������ͽṹ��ʼ��
 * ʵ�ʼ�������������s3d_obj_insert
 * @param mgr Ŀ�곡��������
 * @return ����������
 */
uint PI_API s3d_obj_create(S3dScene *mgr, PiSpatial* spatial);

/**
 * ����ָ���ĳ������
 * @param mgr Ŀ�곡��������
 * @param id ����������
 */
void PI_API s3d_obj_destroy(S3dScene *mgr, uint id);

/**
 * �򳡾��в���ָ���ĳ������,����ǰ��Ҫ��֤����Ŀռ������ѱ�����
 * @param mgr Ŀ�곡��������
 * @param id ����������
 */
void PI_API s3d_obj_insert(S3dScene *mgr, uint id);

/**
 * �ӳ������Ƴ�ָ�����
 * ע�⣺�˷��������������
 * @param mgr Ŀ�곡��������
 * @param id ����������
 */
void PI_API s3d_obj_remove(S3dScene *mgr, uint id);

/**
 * ��ȡ�����spatial
 * @param mgr Ŀ�곡��������
 * @param id ����������
 */
PiSpatial *PI_API s3d_obj_get_spatial(S3dScene *mgr, uint id);

/**
 * ��ȡ��������ĵ㣨Point�����Ϳռ���Ϣ
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param pos ���������ռ���Ϣ
 */
void PI_API s3d_obj_get_point(S3dScene *mgr, uint id, float *pos);

/**
 * ��ȡ���������������Χ�У�AABB�����Ϳռ���Ϣ
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param aabb �������������Χ�пռ���Ϣ
 */
void PI_API s3d_obj_get_aabb(S3dScene *mgr, uint id, PiAABBBox *args);

/**
 * ��ȡ��������Ķ����Χ�У�OBB�����Ϳռ���Ϣ
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param obb ������������Χ�пռ���Ϣ
 */
void PI_API s3d_obj_get_obb(S3dScene *mgr, uint id, PiOBBBox *obb);

/**
 * ���ó�������ĵ㣨Point�����Ϳռ���Ϣ��NULLΪȡ�������ɹ������Զ�����������Ϣ����
 * ע�⣺����ѳɹ�ִ�й�s3d_obj_insert���������Point/AABB/OBB������������ΪNULL�����´������ִ��s3d_obj_remove
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param position ��ռ���Ϣ
 */
void PI_API s3d_obj_set_point(S3dScene *mgr, uint id, PiVector3 *position);

/**
 * ���ó��������������Χ�У�AABB�����Ϳռ���Ϣ��NULLΪȡ�������ɹ������Զ�����������Ϣ����
 * ע�⣺����ѳɹ�ִ�й�s3d_obj_insert���������Point/AABB/OBB������������ΪNULL�����´������ִ��s3d_obj_remove
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param position ������Χ����Ϣ
 */
void PI_API s3d_obj_set_aabb(S3dScene *mgr, uint id, PiAABBBox *aabb);

/**
 * ���ó�������Ķ����Χ�У�OBB�����Ϳռ���Ϣ��NULLΪȡ�������ɹ������Զ�����������Ϣ����
 * ע�⣺����ѳɹ�ִ�й�s3d_obj_insert���������Point/AABB/OBB������������ΪNULL�����´������ִ��s3d_obj_remove
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param position �����Χ����Ϣ
 */
void PI_API s3d_obj_set_obb(S3dScene *mgr, uint id, PiOBBBox *obb);

/**
 * ��ȡ��������Ĳ�ѯ����
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @return ��ѯ����
 */
uint PI_API s3d_obj_get_mask(S3dScene *mgr, uint id);

/**
 * ���ó�������Ĳ�ѯ����
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param mask ��ѯ����
 */
void PI_API s3d_obj_set_mask(S3dScene *mgr, uint id, uint mask);

/**
 * ��ȡ���������������Ϣָ��
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @return ������Ϣָ��
 */
void *PI_API s3d_obj_get_link(S3dScene *mgr, uint id);

/**
 * ���ó��������������Ϣָ��
 * @param mgr Ŀ�곡��������
 * @param id ����������
 * @param link ������Ϣָ��
 */
void PI_API s3d_obj_set_link(S3dScene *mgr, uint id, void *link);

/**
 * �ڳ�����ִ�������ѯ
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 */
void PI_API s3d_query(S3dScene *mgr, uint mask, PiVector *list);

/**
 * �ڳ�����ִ���߶β�ѯ
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 * @param line �߶β�ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_query_line(S3dScene *mgr, uint mask, PiVector *list, PiLineSegment *line, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ִ����׵���ѯ
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 * @param frustum ��׵��ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_query_frustum(S3dScene *mgr, uint mask, PiVector *list, PiFrustum *frustum, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ִ����׵���ѯ����link���뵽list��
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ����������ŵ���linkָ�� ע�⣺��������������ո�����
 * @param frustum ��׵��ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_query_link_frustum(S3dScene *mgr, uint mask, PiVector *list, PiFrustum *frustum, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ִ�����ѯ
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 * @param sphere ���ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_query_sphere(S3dScene *mgr, uint mask, PiVector *list, PiSphere *sphere, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ִ��AABB�������ѯ
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 * @param aabb AABB������ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_query_aabb(S3dScene *mgr, uint mask, PiVector *list,  PiAABBBox *aabb, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ִ��OBB�������ѯ
 * @param mgr Ŀ�곡��������
 * @param mask ��ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 * @param obb OBB������ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_query_obb(S3dScene *mgr, uint mask, PiVector *list,  PiOBBBox *obb, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ʹ��ָ�������������ײ���
 * @param mgr Ŀ�곡��������
 * @param mask ������ײ���Ĳ�ѯ����
 * @param list ��ѯ������� ע�⣺��������������ո�����
 * @param id ���м��Ĳ�ѯ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_collision(S3dScene *mgr, uint mask, PiVector *list,  uint id, S3dQueryAccuracy accuracy);

/**
 * �ڳ�����ʹ��ָ��������б����Ⱥ��ײ���
 * @param mgr Ŀ�곡��������
 * @param mask ������ײ���Ĳ�ѯ����
 * @param list ��ѯ������� ,��ʼ��һ��Ԫ��Ϊ��ײ���壨id_list�е����֮һ����������Ϊ����ײ�������
 * ÿ�������ײ�������ڶ���������һ��NULL��֮��Ϊ��һ����ײ�������ײ��������磺{(a, e, f, c, null), (b, c, t, null)...}
 * ע�⣺��������������ո�����
 * @param obb OBB������ѯ��ָ��
 * @param accuracy ��ѯ����
 */
void PI_API s3d_collision_group(S3dScene *mgr, uint mask, PiVector *list,  PiVector *id_list, S3dQueryAccuracy accuracy);

/**
 * ����һ���߶β�ѯ��
 * @return ��ѯ��ָ��
 */
PiLineSegment *PI_API s3d_query_obj_line_create();

/**
 * ����һ����׵��ѯ��
 * @return ��ѯ��ָ��
 */
PiFrustum *PI_API s3d_query_obj_frustum_create();

/**
 * ����һ�����ѯ��
 * @return ��ѯ��ָ��
 */
PiSphere *PI_API s3d_query_obj_sphere_create();

/**
 * ����һ��AABB������ѯ��
 * @return ��ѯ��ָ��
 */
PiAABBBox *PI_API s3d_query_obj_aabb_create();

/**
 * ����һ��OBB������ѯ��
 * @return ��ѯ��ָ��
 */
PiOBBBox *PI_API s3d_query_obj_obb_create();


/**
 * ����ָ�����߶β�ѯ��
 * @param queryObj ��ѯ��ָ��
 * @param start_x �߶��������X����
 * @param start_y �߶��������Y����
 * @param start_z �߶��������Z����
 * @param end_x �߶��յ�����X����
 * @param end_y �߶��յ�����Y����
 * @param end_z �߶��յ�����Z����
 */
void PI_API s3d_query_obj_line_update(void *queryObj, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z);

/**
 * ����ָ������׵��ѯ��
 * @param queryObj ��ѯ��ָ��
 * @param viewMat ��׵����ͼ����
 * @param projMat ��׵��ͶӰ����
 */
void PI_API s3d_query_obj_frustum_update_matrix(void *queryObj, PiMatrix4 *viewMat, PiMatrix4 *projMat);

/**
 * ����ָ������׵��ѯ��
 * @param queryObj ��ѯ��ָ��
 * @param pos �۲��
 * @param dir �۲췽��
 * @param up ��������
 * @param left ���ü���left
 * @param right ���ü���right
 * @param bottom ���ü���bottom
 * @param top ���ü���top
 * @param near ���ü������
 * @param far Զ�ü������
 * @param is_ortho �Ƿ�������ͶӰ����
 */
void PI_API s3d_query_obj_frustum_update(void *queryObj, PiVector3 *pos, PiVector3 *dir, PiVector3 *up, float left, float right, float bottom, float top, float near, float far, PiBool is_ortho);

/**
 * ����ָ�������ѯ��
 * @param queryObj ��ѯ��ָ��
 * @param pos_x ����λ������X����
 * @param pos_y ����λ������Y����
 * @param pos_Z ����λ������Z����
 * @param radius ��뾶
 */
void PI_API s3d_query_obj_sphere_update(void *queryObj, float pos_x, float pos_y, float pos_z, float radius);

/**
 * ����ָ����AABB������ѯ��
 * @param queryObj ��ѯ��ָ��
 * @param min_x �����帺�������X����
 * @param min_y �����帺�������Y����
 * @param min_z �����帺�������Z����
 * @param max_x ���������������X����
 * @param max_y ���������������Y����
 * @param max_z ���������������Z����
 */
void PI_API s3d_query_obj_aabb_update(void *queryObj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

/**
 * ����ָ����OBB������ѯ��
 * TODO:��������Ϊ������·���
 * @param queryObj ��ѯ��ָ��
 * @param center_x OBB���ĵ�����X����
 * @param center_y OBB���ĵ�����Y����
 * @param center_z OBB���ĵ�����Z����
 * @param axis_0_x OBB��0����X����
 * @param axis_0_y OBB��0����Y����
 * @param axis_0_z OBB��0����Z����
 * @param axis_1_x OBB��1����X����
 * @param axis_1_y OBB��1����Y����
 * @param axis_1_z OBB��1����Z����
 * @param axis_2_x OBB��2����X����
 * @param axis_2_y OBB��2����Y����
 * @param axis_2_z OBB��2����Z����
 * @param extent_0 OBB��0�Ű��᳤
 * @param extent_1 OBB��1�Ű��᳤
 * @param extent_2 OBB��2�Ű��᳤
 */
void PI_API s3d_query_obj_obb_update(void *queryObj, float center_x, float center_y, float center_z, float axis_0_x, float axis_0_y, float axis_0_z, float axis_1_x, float axis_1_y, float axis_1_z, float axis_2_x, float axis_2_y, float axis_2_z, float extent_0, float extent_1, float extent_2);


/**
 * ����ָ���Ĳ�ѯ��
 * @param queryObj ��ѯ��ָ��
 */
void PI_API s3d_query_obj_destroy(void *queryObj);

PI_END_DECLS

#endif
