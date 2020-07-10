#ifndef __INCLUDE_A_STAR_H__
#define __INCLUDE_A_STAR_H__

#include <pi_lib.h>
#include "gird_obstacle.h"

PI_BEGIN_DECLS
#define MAX_STEP 300000

typedef struct AStarEnv AStarEnv;

typedef uint32 (*GenReachableNodesFunc) (AStarEnv *env, void *env_impl, void *node, PiBool only_static);
typedef PiBool (*NodeVisibleTestFunc)(void *env_impl, void *src_node, void *dst_node, PiBool onlyStatic);
typedef void (*FixNodeToReachableFunc)(void *env_impl, void *node, void *ref_node);
typedef uint32 (*CalFFnc) (void *env_impl, void *node, void *ref_node, void *start_node, void *end_node);
typedef uint32 (*CalHFunc) (void *env_impl, void *node, void* end_node);
typedef uint32 (*CalDistanceFunc) (void *env_impl, void *node1, void *node2);
typedef void (*NodeImplInit) (void *node);
typedef void (*NodeImplShutdown) (void *node);

typedef enum AStarType
{
	ASTAR_GRID = 0,
	ASTAR_GRAPH
} AStarType;

struct AStarEnv
{
	void *impl;

	PiQueue *results_queue;
	PiHeap *open_heap;
	PiHash *open_hash;
	PiHash *close_hash;

	PiVector *node_cache;
	PiVector *reachable_node_cache;
	uint32 cache_idx;

	GenReachableNodesFunc gen_reachable_nodes;
	NodeVisibleTestFunc node_visible_test;
	FixNodeToReachableFunc fix_node_to_reachable;
	CalFFnc cal_f;
	CalDistanceFunc cal_distance;
	CalHFunc cal_h;

	NodeImplInit node_impl_init;
	NodeImplShutdown node_impl_shutdown;
	PiEqualFunc node_equal;
	AStarType findType;
};

typedef struct AStarNode
{
	void	*impl;
	uint32 idx;
	uint32 f;//评价值
	uint32 g;
	uint32 h;
	struct AStarNode *parent;
}AStarNode;


/**
  * a*环境初始化
 */
void astar_env_init(AStarEnv *env, GenReachableNodesFunc get_reachable_nodes_func, NodeVisibleTestFunc node_visible_test_func, FixNodeToReachableFunc fix_node_to_reachable_func,
				PiHashFunc node_hash_func, PiEqualFunc node_equal_func,
				CalFFnc cal_f_func, CalDistanceFunc cal_distance_func, CalHFunc cal_h_func,
				NodeImplInit node_impl_init_func, NodeImplShutdown node_impl_shutdown_func, AStarType findType);

/*
 * a*环境销毁
*/
void astar_env_shutdown(AStarEnv *env);

/*
 * a*寻路调用
*/
void astar_find_path(AStarEnv *env, AStarNode *start, AStarNode *end, PiBool only_static);

//从node缓存里拿一个node
AStarNode* astar_env_get_node_from_cache(AStarEnv *env);

// 创建寻路环境
AStarEnv* PI_API s3d_astar_env_create();

// 销毁寻路环境
void PI_API s3d_astar_env_destroy(AStarEnv *env);

//获得寻路结果
PiQueue* PI_API s3d_astar_get_result(AStarEnv *env);

PI_END_DECLS

#endif

