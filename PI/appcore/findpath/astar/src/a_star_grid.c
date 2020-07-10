#include "a_star_grid.h"
#include "tiered_a_star.h"

static uint32 _gen_reachable_nodes(AStarEnv *env, GirdObstacle *obs, AStarNode *node, PiBool only_static)
{
	uint32 num_count = 0;
	AStarNode *new_node;
	int32 x = node->idx % obs->width;
	int32 y = node->idx / obs->width;
	int32 i, n = obs->curr_size;
	PiBool right, left, down, up;
	uint32 idx;


	//右

	pi_vector_clear(env->reachable_node_cache, FALSE);
	right = FALSE;
	for(i= -n+1; i < n; ++i)
	{
		if (is_obstacle_size(obs, x + n, y + i, 1, only_static))
		{
			right = TRUE;
			break;
		}
	}
    idx = x + 1 + y * obs->width;
	if(!right && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 10;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}


	//左
	left = FALSE;
	for(i= -n+1; i < n; ++i)
	{
		if (is_obstacle_size(obs, x - n, y + i, 1, only_static))
		{
			left = TRUE;
			break;
		}
	}
    idx = x - 1 + y * obs->width;
	if(!left && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 10;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}

	//下
	down = FALSE;
	for(i= -n+1; i < n; ++i)
	{
		if (is_obstacle_size(obs, x + i, y + n, 1, only_static))
		{
			down = TRUE;
			break;
		}
	}
    idx = x + ( y + 1 ) * obs->width;
	if(!down && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 10;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}

	//上
	up = FALSE;
	for(i= -n+1; i < n; ++i)
	{
		if (is_obstacle_size(obs, x + i, y - n, 1, only_static))
		{
			up = TRUE;
			break;
		}
	}
    idx = x + ( y - 1 ) * obs->width;
	if(!up && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 10;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}

	//四个角

	//右上
    idx = x + 1 + ( y - 1 ) * obs->width;
	if (!right && !up && !is_obstacle_size(obs, x + n, y - n, 1, only_static) && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 14;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}
	//右下
    idx =  x + 1 + ( y + 1 ) * obs->width;
	if (!right && !down && !is_obstacle_size(obs, x + n, y + n, 1, only_static) && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 14;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}
	//左上
    idx =  x - 1 + ( y - 1 ) * obs->width;
	if (!left && !up && !is_obstacle_size(obs, x - n, y - n, 1, only_static) && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 14;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}
	//左下
    idx =  x - 1 + ( y + 1 ) * obs->width;
	if (!left && !down && !is_obstacle_size(obs, x - n, y + n, 1, only_static) && !pi_hash_lookup(env->close_hash, &idx, NULL))
	{
		num_count += 1;
		if(!pi_hash_lookup(env->open_hash, &idx, (void**)&new_node))
		{
			new_node = (AStarNode*)astar_env_get_node_from_cache(env);
			new_node->idx = idx;
			new_node->g = node->g + 14;
		}
		pi_vector_push(env->reachable_node_cache, new_node);
	}

	return num_count;
}

static PiBool _node_visibile_test(GirdObstacle* obs, AStarNode* src_node, AStarNode* dst_node, PiBool onlyStatic)
{
	return node_visible_test_func(obs, src_node->idx, dst_node->idx, onlyStatic);
}

static void _fix_node_to_reachable(GirdObstacle* obs, AStarNode* node, AStarNode *ref_node)
{
	fix_node_to_reachable(obs, &node->idx, &ref_node->idx);
}

static uint	PI_API _node_hash(const AStarNode *node)
{
	return node->idx;
}

static uint PI_API _node_idx_hash(const uint32 *idx)
{
	return (uint)*idx;
}

static PiBool PI_API _node_equal(const AStarNode* node1, const AStarNode* node2)
{
	/*GirdNode *node1 = (GirdNode*)kv1->impl;
	GirdNode *node2 = (GirdNode*)kv2->impl;*/
	return node1->idx == node2->idx;
}

static PiBool PI_API _node_idx_equal(const uint32* idx1, const uint32* idx2)
{
	return *idx1 == *idx2;
}

static uint32 _cal_f( GirdObstacle *obs, AStarNode *node, AStarNode *ref_node, AStarNode *start_node, AStarNode *end_node)
{
	int32 end_x, end_y, x, y;
	int32 x2,y2;

	PI_USE_PARAM(ref_node);

	x = (int32)node->idx % obs->width;
	y = (int32)node->idx / obs->width;

	end_x = (int32)end_node->idx % obs->width;
	end_y = (int32)end_node->idx / obs->width;

	x2 = ABS(x - end_x);
	y2 = ABS(y - end_y);

	/*x2 *= x2;
	y2 *= y2;*/

	//node->h = x2 + y2;
	node->h = (x2 + y2) * 10 * (1 + 1 / MAX_STEP);
	//node->h = (uint32)pi_math_sqrt((float)(x2 + y2) );

	return node->h + node->g;
}

static uint32 _cal_h( GirdObstacle *obs, AStarNode *node,AStarNode *end_node)
{
	int32 end_x, end_y, x, y;
	int32 x2,y2;

	x = (int32)node->idx % obs->width;
	y = (int32)node->idx / obs->width;

	end_x = (int32)end_node->idx % obs->width;
	end_y = (int32)end_node->idx / obs->width;

	x2 = ABS(x - end_x)*10;
	y2 = ABS(y - end_y)*10;

	x2 *= x2;
	y2 *= y2;

	//node->h = x2 + y2;
	node->h = (uint32)pi_math_sqrt((float)(x2 + y2) );

	return node->h;
}

static uint32 _cal_distance( GirdObstacle *obs, AStarNode* node1, AStarNode* node2)
{
	PosType x1, x2, y1, y2;
	x1 = node1->idx % obs->width;
	y1 = node1->idx / obs->width;
	x2 = node2->idx % obs->width;
	y2 = node2->idx / obs->width;
	return (EdgeType)pi_math_sqrt((float)((x1 - x2) * (x1 - x2)*100 + (y1 - y2) * (y1 - y2)*100));
}
static void _gird_node_impl_init(AStarNode *node)
{
	if( node->impl == NULL )
		node->impl = pi_new0(GirdNode, 1);

	pi_memset(node->impl, 0, sizeof(GirdNode));
}

static void _gird_node_impl_shutdown(AStarNode *node)
{
	if( node->impl != NULL)
	{
		pi_free(node->impl);
	}
	node->impl = NULL;
}

AStarEnv* PI_API s3d_astar_grid_env_create()
{
	AStarEnv *env = s3d_astar_env_create();
	astar_env_init(env,(GenReachableNodesFunc)_gen_reachable_nodes, (NodeVisibleTestFunc)_node_visibile_test, (FixNodeToReachableFunc)_fix_node_to_reachable, (PiHashFunc)_node_idx_hash, (PiEqualFunc)_node_equal, (CalFFnc)_cal_f,(CalDistanceFunc)_cal_distance, (CalHFunc)_cal_h, (NodeImplInit)_gird_node_impl_init, (NodeImplShutdown)_gird_node_impl_shutdown, ASTAR_GRID);
	return env;
}

void PI_API s3d_astar_grid_env_destroy(AStarEnv *env)
{
    s3d_astar_env_destroy(env);
}

void PI_API s3d_astar_gird_env_init(AStarEnv *env, GirdObstacle *obs)
{
	PI_ASSERT(obs, "grid env init error!");
	env->impl = obs;
}

void PI_API s3d_astar_gird_env_shutdown(AStarEnv *env)
{
	env->impl = NULL;
	astar_env_shutdown(env);
}

void PI_API s3d_astar_gird_find_path(AStarEnv *env, uint32 start_x, uint32 start_y, uint32 end_x, uint32 end_y, int32 size, PiBool only_static)
{
	AStarNode *start, *end;
	GirdObstacle *obs = (GirdObstacle*)env->impl;
	obs->curr_size = size;

	env->cache_idx = 0;

	start = astar_env_get_node_from_cache(env);
	start->idx = start_x + start_y * obs->width;

	end = astar_env_get_node_from_cache(env);
	end->idx = end_x + end_y * obs->width;

	astar_find_path(env, start, end, only_static);
}


uint32 PI_API s3d_astar_gird_get_node_info(AStarNode *node)
{
	return node->idx;
}

