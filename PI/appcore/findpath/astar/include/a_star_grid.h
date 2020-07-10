#ifndef __INCLUDE_A_STAR_GRID_H__
#define __INCLUDE_A_STAR_GRID_H__

#include <pi_lib.h>
#include "a_star.h"
#include "gird_obstacle.h"

PI_BEGIN_DECLS

AStarEnv* PI_API s3d_astar_grid_env_create();

void PI_API s3d_astar_grid_env_destroy(AStarEnv *env);

//栅格a*环境初始化
void PI_API s3d_astar_gird_env_init(AStarEnv *env, GirdObstacle *obs);

//栅格a*环境销毁
void PI_API s3d_astar_gird_env_shutdown(AStarEnv *env);

//栅格a*寻路调用
void PI_API s3d_astar_gird_find_path(AStarEnv *env, uint32 start_x, uint32 start_y, uint32 end_x, uint32 end_y, int32 size, PiBool only_static);
//
////获得寻路结果
//PiQueue* PI_API s3d_astar_gird_get_result(AStarEnv *env);

//获取节点信息
uint32 PI_API s3d_astar_gird_get_node_info(AStarNode *node);

PI_END_DECLS

#endif
