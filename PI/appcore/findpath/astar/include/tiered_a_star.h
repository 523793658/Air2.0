#ifndef __INCLUDE_TIERED_A_STAR_H__
#define __INCLUDE_TIERED_A_STAR_H__

#include <pi_lib.h>
#include "a_star.h"
#include "a_star_graph.h"
#include "a_star_grid.h"
#include "graph.h"
#include "gird_obstacle.h"

PI_BEGIN_DECLS

typedef struct TieredAStarEnv
{
	AStarEnv* astar_graph_env;
	AStarEnv* astar_grid_env;
	GirdObstacle* obstacle;
	PiGraph* graph;
	PiQueue *result;
	uint result_idx;
	AStarNode start_node;
	AStarNode end_node;
}TieredAStarEnv;

TieredAStarEnv* PI_API s3d_tiered_astar_env_create();

void PI_API s3d_tiered_astar_env_free(TieredAStarEnv* env);

//************************************
// Method:    s3d_tiered_astar_env_init
// FullName:  s3d_tiered_astar_env_init
// Access:    public 
// Returns:   void PI_API
// Qualifier:
// Parameter: TieredAStarEnv * env
// Parameter: GirdObstacle * obstacle 障碍
// Parameter: PiGraph * graph 连通图
//************************************
void PI_API s3d_tiered_astar_env_init(TieredAStarEnv* env, GirdObstacle* obstacle, PiGraph* graph);

void PI_API s3d_tiered_astar_env_shutdown(TieredAStarEnv* env);

void PI_API s3d_tiered_astar_findpath(TieredAStarEnv* env, PosType start_x, PosType start_y, PosType end_x, PosType end_y, int32 size, PiBool only_static);

//************************************
// Method:    s3d_tiered_astar_link_vertex
// FullName:  s3d_tiered_astar_link_vertex
// Access:    public 
// Returns:   void PI_API
// Qualifier:
// Parameter: TieredAStarEnv * env
// 连通图中连接所有可以直连的点
//************************************
void PI_API s3d_tiered_astar_link_vertex(TieredAStarEnv* env);

PiQueue* PI_API s3d_tiered_astar_get_result(TieredAStarEnv* env);

//************************************
// Method:    s3d_tiered_get_index
// FullName:  s3d_tiered_get_index
// Access:    public 
// Returns:   uint PI_API
// Qualifier:
// Parameter: uint * idx
// 获取指针idx的值
//************************************
uint PI_API s3d_tiered_get_index(uint *idx);

VertexIndexType PI_API s3d_tiered_astar_generate_guides(TieredAStarEnv* env);
PI_END_DECLS

#endif
