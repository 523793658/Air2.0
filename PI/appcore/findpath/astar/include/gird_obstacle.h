#ifndef __INCLUDE_GIRD_OBSTACLE_H__
#define __INCLUDE_GIRD_OBSTACLE_H__

#include <pi_lib.h>


PI_BEGIN_DECLS


typedef struct  
{
	uint8 *data;
	uint32 width;
	uint32 height;
	int32 curr_size;//��ǰѰ·��size
} GirdObstacle;

typedef struct  
{
	uint32 idx1;
	//uint32 idx;//��դ�������е�����
	//uint32 g;//�������ľ������
	//uint32 h;//�����յ�ľ������
} GirdNode;

//�Ƿ�ֱ���ж�
PiBool node_visible_test_func(GirdObstacle *obs, uint32 srcIdx, uint32 dstIdx, PiBool onlyStatic);

PiBool PI_API node_visible_test(GirdObstacle *obs, int32 x1, int32 y1, int32 x2, int32 y2, int32 *outX, int32* outY, PiBool onlyStatic, int32 *obsX, int32 *obsY);

//������
void fix_node_to_reachable(GirdObstacle *obs, uint32 *idx, uint32 *ref_idx);

//�����ֵ
uint32 cal_f( GirdObstacle *obs, GirdNode *node, GirdNode *ref_node, GirdNode *start_node, GirdNode *end_node);

//�Ƿ����ϰ���
PiBool PI_API s3d_is_obstacle( GirdObstacle *obs, uint32 x, uint32 y, int32 size );

//�Ƿ��Ǿ�̬�ϰ���
PiBool PI_API s3d_is_static_obstacle( GirdObstacle *obs, uint32 x, uint32 y, int32 size );

PiBool PI_API is_obstacle_size(GirdObstacle *obs, uint32 x, uint32 y, int32 size, PiBool only_static);

/**
 * �ж�����֮�������ϰ�
 * @param obs �ϰ�����ָ��
 * @param src �������
 * @param des �յ�����
 * @param result ��������ϰ�����ϰ�����
*/
PiBool PI_API s3d_obstacle_visible_test(GirdObstacle *obs, uint32 src[2], uint32 des[2], int32 result[4]);

/**
 * �����޸��ϰ�����
 * @param env Ѱ·����ָ��
 * @param modify_data �޸����ݵ�����,ÿ���޸�Ԫ�����ĸ�uint32���,����x,y,size,value,type
 */
void PI_API s3d_obstacle_modify_data_bat(GirdObstacle* obs, PiDvector *modify_data);

/**
 * �޸��ϰ�����
 * @param obs �ϰ�����ָ��
 * @param x x����
 * @param y y����
 * @param size ��С
 * @param value ����ֵ
 * @param type ��������(��/����
*/
void PI_API s3d_obstacle_modify_data(GirdObstacle* obs, uint32 x, uint32 y, uint32 size, uint8 value, uint8 type);

/**
 * �����ϰ�����
 * @param data �ϰ�����������
 * @param width ��ͼ���
 * @param height ��ͼ�߶�
*/
GirdObstacle* PI_API s3d_obstacle_create(uint8 *data, uint32 width, uint32 height);

/**
 * ���һ��λ�õ��ϰ�����
 * @param obs �ϰ�ָ��
 * @param x ����x
 * @param y ����y
 * @param size �ߴ�
*/
uint32 PI_API s3d_get_obstacle_value(GirdObstacle* obs, uint32 x, uint32 y, int32 size);

/**
 *�ͷ��ϰ�����
*/
void PI_API s3d_obstacle_free(GirdObstacle* obs);

PiBool PI_API s3d_obstacle_modify_data_with_rotate(GirdObstacle *obs, float x, float y, float scaleX, float scaleY, float dirX, float dirY, uint8 value, uint8 type,  PiBool modify, uint testPointX, uint testPointY);


PI_END_DECLS




#endif

