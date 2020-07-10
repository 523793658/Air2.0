#ifndef __INCLUDE_A_STAR_GRAPH_H__
#define __INCLUDE_A_STAR_GRAPH_H__

#include <pi_lib.h>
#include "a_star.h"
#include "gird_obstacle.h"
#include "graph.h"

PI_BEGIN_DECLS

AStarEnv* PI_API s3d_astar_graph_env_create();

void PI_API s3d_astar_graph_env_destroy(AStarEnv* env);

//连通图a*环境初始化
void PI_API s3d_astar_graph_env_init(AStarEnv *env, PiGraph *graph);

//连通图a*环境销毁
void PI_API s3d_astar_graph_env_shutdown(AStarEnv *env);

//连通图a*寻路调用
void PI_API s3d_astar_graph_find_path(AStarEnv *env, VertexIndexType start_idx, VertexIndexType end_idx, PiBool only_static);
PI_END_DECLS

#endif
