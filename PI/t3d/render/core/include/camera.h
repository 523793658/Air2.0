#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_matrix4.h>

/**
 * �����ģ��
 */

typedef struct  
{
	PiVector3 location;
	PiVector3 lookat;
	PiMatrix4 rotation;
	float frustum_left;
	float frustum_right;
	float frustum_bottom;
	float frustum_top;
	float frustum_near;
	float frustum_far;
	PiBool is_ortho;
	PiMatrix4 view_matrix;
	PiMatrix4 projection_matrix;
	PiMatrix4 view_projection_matrix;
}PiCamera;

PI_BEGIN_DECLS

/**
 * ���������
 * @returns �����ָ��
 */
PiCamera* PI_API pi_camera_new();

/**
 * �ͷ������
 * @param cam �����ָ��
 */
void PI_API pi_camera_free(PiCamera* cam);

/**
 * ���������׵�壨͸��ͶӰ������ͶӰͨ�ã�
 * @param cam �����ָ��
 * @param left ���ü���������
 * @param right ���ü���������
 * @param bottom ���ü���������
 * @param top ���ü���������
 * @param near ���ü������
 * @param far Զ�ü������
 * @param is_ortho �Ƿ�������ͶӰ��ʽ������׵��
 */
void PI_API pi_camera_set_frustum(PiCamera* cam, float left, float right, float bottom, float top, float near, float far, PiBool is_ortho);

/**
 * ��͸�ӷ�ʽ������׵��
 * @param cam �����ָ��
 * @param fovy yzƽ���ϵ���Ұ�Ƕȣ�degree����������0��~180��֮��
 * @param aspect �ݺ�ȣ���ȳ��Ը߶�
 * @param near ���ü������
 * @param far Զ�ü������
 */
void PI_API pi_camera_set_perspective(PiCamera* cam, float fovy, float aspect, float near, float far);

/**
 * ���ֽ��ü���߶Ȳ��䣬����������ݺ��
 * @param cam �����ָ��
 * @param aspect �ݺ�ȣ���ȳ��Ը߶�
 */
void PI_API pi_camera_resize(PiCamera* cam, float aspect);


/**
* ��ȡ������ݺ��
* @param cam �����ָ��
*/
float PI_API pi_camera_get_aspect(PiCamera* cam);

/**
 * �������λ��
 * @param cam �����ָ��
 * @param x ���λ������X����
 * @param y ���λ������Y����
 * @param z ���λ������Z����
 */
void PI_API pi_camera_set_location(PiCamera* cam, float x, float y, float z);

/**
 * �����������ת
 * @param cam �����ָ��
 * @param w �����ת��Ԫ��W����
 * @param x �����ת��Ԫ��X����
 * @param y �����ת��Ԫ��Y����
 * @param z �����ת��Ԫ��Z����
 */
void PI_API pi_camera_set_rotation(PiCamera* cam, float w, float x, float y, float z);

/**
 * ��������ĳ���
 * @param cam �����ָ��
 * @param x ��������X����
 * @param y ��������Y����
 * @param z ��������Z����
 */
void PI_API pi_camera_set_direction(PiCamera* cam, float x, float y, float z);

/**
 * ��ת���ʹ�䳯��ָ��λ��
 * @param cam �����ָ��
 * @param x Ŀ��λ�õ�X����
 * @param y Ŀ��λ�õ�Y����
 * @param z Ŀ��λ�õ�Z����
 */
void PI_API pi_camera_set_look_at(PiCamera* cam, float x, float y, float z);

/**
 * ����������Ϸ�������
 * @param cam �����ָ��
 * @param x �Ϸ���������X����
 * @param y �Ϸ���������Y����
 * @param z �Ϸ���������Z����
 */
void PI_API pi_camera_set_up(PiCamera* cam, float x, float y, float z);

/**
 * �����Զ���ͶӰ����
 * ע�⣺�˺������ܻᵼ�������׵��������Ϣ��ͶӰ����ƥ�䣬�ҵ�ͶӰ��غ���������ʱ�����¸��ǣ�����ʹ��
 * @param cam �����ָ��
 * @param proj_mat ͶӰ����
 */
void PI_API pi_camera_set_projection_matrix(PiCamera* cam, PiMatrix4* proj_mat);

/**
 * ��ȡ���ü���������
 * @param cam �����ָ��
 * @returns ���ü���������
 */
float PI_API pi_camera_get_frustum_left(PiCamera* cam);

/**
 * ��ȡ���ü���������
 * @param cam �����ָ��
 * @returns ���ü���������
 */
float PI_API pi_camera_get_frustum_right(PiCamera* cam);

/**
 * ��ȡ���ü���ײ�����
 * @param cam �����ָ��
 * @returns ���ü���ײ�����
 */
float PI_API pi_camera_get_frustum_bottom(PiCamera* cam);

/**
 * ��ȡ���ü��涥������
 * @param cam �����ָ��
 * @returns ���ü��涥������
 */
float PI_API pi_camera_get_frustum_top(PiCamera* cam);

/**
 * ��ȡ���ü������
 * @param cam �����ָ��
 * @returns ���ü������
 */
float PI_API pi_camera_get_frustum_near(PiCamera* cam);

/**
 * ��ȡԶ�ü������
 * @param cam �����ָ��
 * @returns Զ�ü������
 */
float PI_API pi_camera_get_frustum_far(PiCamera* cam);

/**
 * ��ȡ����ĵ�ǰλ��
 * @param cam �����ָ��
 * @returns ���λ��
 */
PiVector3* PI_API pi_camera_get_location(PiCamera* cam);

/**
 * ��ȡ�������ת
 * @param cam �����ָ��
 * @param result �������
 */
void PI_API pi_camera_get_rotation(PiCamera* cam, PiQuaternion* result);

/**
 * ��ȡ�������
 * @param cam �����ָ��
 * @param result �������
 */
void PI_API pi_camera_get_direction(PiCamera* cam, PiVector3* result);

/**
 * ��ȡ�������������
 * @param cam �����ָ��
 * @param result �������
 */
void PI_API pi_camera_get_up(PiCamera* cam, PiVector3* result);

/**
 * ��ȡ��ͼ����
 * @param cam �����ָ��
 * @returns ��ȡ��ͼ����
 */
PiMatrix4* PI_API pi_camera_get_view_matrix(PiCamera* cam);

/**
 * ��ȡͶӰ����
 * @param cam �����ָ��
 * @returns ��ȡͶӰ����
 */
PiMatrix4* PI_API pi_camera_get_projection_matrix(PiCamera* cam);

/**
 * ��ȡ�������ͼͶӰ����
 * @param cam �����ָ��
 * @returns ��ͼͶӰ����
 */
PiMatrix4* PI_API pi_camera_get_view_projection_matrix(PiCamera* cam);

/**
 * ��ȡ��ǰ����Ƿ�Ϊ��ͶӰ
 * @param cam �����ָ��
 * @returns �Ƿ�Ϊ��ͶӰ
 */
PiBool PI_API pi_camera_is_ortho(PiCamera* cam);

/**
 * ������ռ�����ת����ͶӰ�ռ�
 * @param cam �����ָ��
 * @param x ����ռ�����X����
 * @param y ����ռ�����Y����
 * @param z ����ռ�����Z����
 * @param result �������
 */
void PI_API pi_camera_world2projection(PiCamera* cam, float x, float y, float z, PiVector3* result);

/**
 * ��ͶӰ�ռ�����ת��������ռ�
 * @param cam �����ָ��
 * @param x ͶӰ�ռ�����X����
 * @param y ͶӰ�ռ�����Y����
 * @param z ͶӰ�ռ�����Z����
 * @param result �������
 */
void PI_API pi_camera_projection2world(PiCamera* cam, float x, float y, float z, PiVector3* result);

/**
 * ����Ļ�ռ�����ת����ͶӰ�ռ�
 * @param screen_width ��Ļ���
 * @param screen_height ��Ļ�߶�
 * @param x ��Ļ�ռ�����X����
 * @param y ��Ļ�ռ�����Y����
 * @param result ������棬ע��Z����������
 */
void PI_API pi_camera_screen2projection(uint screen_width, uint screen_height, float x, float y, PiVector3* result);

/**
 * ��ͶӰ�ռ�����ת������Ļ�ռ�
 * @param screen_width ��Ļ���
 * @param screen_height ��Ļ�߶�
 * @param x ͶӰ�ռ�����X����
 * @param y ͶӰ�ռ�����Y����
 * @param result ������棬ע��Z����������
 */
void PI_API pi_camera_projection2screen(uint screen_width, uint screen_height, float x, float y, PiVector3* result);

/**
 * ������ռ�����ת������Ļ�ռ�
 * @param cam �����ָ��
 * @param screen_width ��Ļ���
 * @param screen_height ��Ļ�߶�
 * @param x ����ռ�����X����
 * @param y ����ռ�����Y����
 * @param z ����ռ�����Z����
 * @param result ������棬ע��Z����������
 */
void PI_API pi_camera_world2screen(PiCamera* cam, uint screen_width, uint screen_height, float x, float y, float z, PiVector3* result);

/**
 * ����Ļ�ռ�����ת��������ռ�
 * @param cam �����ָ��
 * @param screen_width ��Ļ���
 * @param screen_height ��Ļ�߶�
 * @param x ��Ļ�ռ�����X����
 * @param y ��Ļ�ռ�����Y����
 * @param z ͶӰ�ռ����ֵ��ͨ����-1��ʾ���ü��棬1��ʾԶ�ü���
 * @param result �������
 */
void PI_API pi_camera_screen2world(PiCamera* cam, uint screen_width, uint screen_height, float x, float y, float z, PiVector3* result);


/**
* ��¡���
*/
PiCamera* PI_API pi_camera_clone(PiCamera* src);
PI_END_DECLS


#endif /* INCLUDE_CAMERA_H */