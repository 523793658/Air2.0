#ifndef __INCLUDE_GIRD_OBSTACLE_H__
#define __INCLUDE_GIRD_OBSTACLE_H__

#include <pi_lib.h>


PI_BEGIN_DECLS


typedef struct  
{
	uint8 *data;
	uint32 width;
	uint32 height;
	int32 curr_size;//当前寻路的size
} GirdObstacle;

typedef struct  
{
	uint32 idx1;
	//uint32 idx;//在栅格数据中的索引
	//uint32 g;//距离起点的距离估算
	//uint32 h;//距离终点的距离估算
} GirdNode;

//是否直连判断
PiBool node_visible_test_func(GirdObstacle *obs, uint32 srcIdx, uint32 dstIdx, PiBool onlyStatic);

PiBool PI_API node_visible_test(GirdObstacle *obs, int32 x1, int32 y1, int32 x2, int32 y2, int32 *outX, int32* outY, PiBool onlyStatic, int32 *obsX, int32 *obsY);

//修正点
void fix_node_to_reachable(GirdObstacle *obs, uint32 *idx, uint32 *ref_idx);

//计算估值
uint32 cal_f( GirdObstacle *obs, GirdNode *node, GirdNode *ref_node, GirdNode *start_node, GirdNode *end_node);

//是否是障碍点
PiBool PI_API s3d_is_obstacle( GirdObstacle *obs, uint32 x, uint32 y, int32 size );

//是否是静态障碍点
PiBool PI_API s3d_is_static_obstacle( GirdObstacle *obs, uint32 x, uint32 y, int32 size );

PiBool PI_API is_obstacle_size(GirdObstacle *obs, uint32 x, uint32 y, int32 size, PiBool only_static);

/**
 * 判断两点之间有无障碍
 * @param obs 障碍数据指针
 * @param src 起点坐标
 * @param des 终点坐标
 * @param result 存放遇到障碍后的障碍坐标
*/
PiBool PI_API s3d_obstacle_visible_test(GirdObstacle *obs, uint32 src[2], uint32 des[2], int32 result[4]);

/**
 * 批量修改障碍数据
 * @param env 寻路环境指针
 * @param modify_data 修改数据的向量,每个修改元素由四个uint32组成,代表x,y,size,value,type
 */
void PI_API s3d_obstacle_modify_data_bat(GirdObstacle* obs, PiDvector *modify_data);

/**
 * 修改障碍数据
 * @param obs 障碍数据指针
 * @param x x坐标
 * @param y y坐标
 * @param size 大小
 * @param value 设置值
 * @param type 设置类型(加/减）
*/
void PI_API s3d_obstacle_modify_data(GirdObstacle* obs, uint32 x, uint32 y, uint32 size, uint8 value, uint8 type);

/**
 * 创建障碍数据
 * @param data 障碍二进制数据
 * @param width 地图宽度
 * @param height 地图高度
*/
GirdObstacle* PI_API s3d_obstacle_create(uint8 *data, uint32 width, uint32 height);

/**
 * 获得一个位置的障碍数据
 * @param obs 障碍指针
 * @param x 坐标x
 * @param y 坐标y
 * @param size 尺寸
*/
uint32 PI_API s3d_get_obstacle_value(GirdObstacle* obs, uint32 x, uint32 y, int32 size);

/**
 *释放障碍数据
*/
void PI_API s3d_obstacle_free(GirdObstacle* obs);

PiBool PI_API s3d_obstacle_modify_data_with_rotate(GirdObstacle *obs, float x, float y, float scaleX, float scaleY, float dirX, float dirY, uint8 value, uint8 type,  PiBool modify, uint testPointX, uint testPointY);


PI_END_DECLS




#endif

