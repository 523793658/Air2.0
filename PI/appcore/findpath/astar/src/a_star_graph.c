#include "tiered_a_star.h"
/******static methods*****/

static VertexIndexType get_reachable_nodes( AStarEnv *env, PiGraph* graph, AStarNode* node )
{
	VertexIndexType num_count = 0;
	AStarNode *new_node;
	PosType x, y, new_x, new_y;
	EdgeType new_g;
	VertexIndexType i, n = graph->vertex_num, idx = node->idx;
	x = graph->graph_vertexs[idx]->x;
	y = graph->graph_vertexs[idx]->y;

	pi_vector_clear(env->reachable_node_cache, FALSE);

	for(i = 0; i < n; ++i)
	{
		if(pi_graph_is_vertex_connected(graph, idx, i) && !pi_hash_lookup(env->close_hash, &i, NULL))
		{
			++num_count;
			if(!pi_hash_lookup(env->open_hash, &i, (void**)&new_node))
			{
				new_node = (AStarNode *)astar_env_get_node_from_cache(env);
				new_node->idx = i;
				new_x = graph->graph_vertexs[i]->x;
				new_y = graph->graph_vertexs[i]->y;
				new_g = node->g + (EdgeType)pi_math_sqrt((float)((new_y - y)*(new_y - y)*100 + (new_x - x)*(new_x - x)*100));
				new_node->g = new_g;

			}
			pi_vector_push(env->reachable_node_cache, new_node);			
		}
	}


	return num_count;
}

static uint	PI_API _node_hash(const AStarNode *node)
{
	return node->idx;
}

static uint PI_API _node_idx_hash(const uint32 *idx)
{
	return (uint)*idx;
}

static PiBool PI_API _node_equal(const AStarNode*  node1, const AStarNode* node2)
{
	return node1->idx == node2->idx;
}

static PiBool PI_API _node_idx_equal(const uint32* idx1, const uint32* idx2)
{
	return *idx1 == *idx2;
}

static uint32 _cal_f( PiGraph *graph, AStarNode *node, AStarNode *ref_node, AStarNode *start_node, AStarNode *end_node)
{
	PosType end_x,end_y,x,y;
	EdgeType dx,dy;
	
	PI_USE_PARAM(ref_node);

	x = (PosType)graph->graph_vertexs[node->idx]->x;
	y = (PosType)graph->graph_vertexs[node->idx]->y;
	
	end_x = (PosType)graph->graph_vertexs[end_node->idx]->x;
	end_y = (PosType)graph->graph_vertexs[end_node->idx]->y;

	dx = ABS(x - end_x);
	dy = ABS(y - end_y);

	dx *= dx;
	dy *= dy;

	node->h = (EdgeType)pi_math_sqrt((float)(dx + dy));
#if defined(_DEBUG)
	pi_log_print(LOG_INFO, "%d -- (%d,%d) to (%d,%d) g = %d, h=%d, f=%d", node->idx, x, y, end_x, end_y, node->g, node->h, node->g + node->h);
#endif
	return node->g + node->h;
}

static uint32 _cal_h( PiGraph *graph, AStarNode *node, AStarNode *end_node)
{
	PosType end_x,end_y,x,y;
	EdgeType dx,dy;

	x = (PosType)graph->graph_vertexs[node->idx]->x;
	y = (PosType)graph->graph_vertexs[node->idx]->y;

	end_x = (PosType)graph->graph_vertexs[end_node->idx]->x;
	end_y = (PosType)graph->graph_vertexs[end_node->idx]->y;

	dx = ABS(x - end_x)*10;
	dy = ABS(y - end_y)*10;

	dx *= dx;
	dy *= dy;
	//node->h = (dx + dy) * 10 * (1 + 1 / MAX_STEP);
	node->h = (EdgeType)pi_math_sqrt((float)(dx + dy));
	return node->h;
}

static uint32 _cal_distance( PiGraph *graph, AStarNode* node1, AStarNode* node2)
{
	PosType x1, x2, y1, y2;
	x1 = graph->graph_vertexs[node1->idx]->x;
	y1 = graph->graph_vertexs[node1->idx]->y;
	x2 = graph->graph_vertexs[node2->idx]->x;
	y2 = graph->graph_vertexs[node2->idx]->y;
	return (EdgeType)pi_math_sqrt((float)((x1 - x2) * (x1 - x2)*100 + (y1 - y2) * (y1 - y2)*100));
}

static void _node_impl_init(AStarNode *node)
{
	if( node->impl == NULL )
		node->impl = pi_new0(graph_node, 1);

	pi_memset(node->impl, 0, sizeof(graph_node));
}

static void _node_impl_shutdown(AStarNode *node)
{
	if( node->impl != NULL)
	{
		pi_free(node->impl);
	}
	node->impl = NULL;
}

static void _fix_node_to_reachable(PiGraph* graph, graph_node* node, graph_node* ref_node)
{
	return;
}

static PiBool _node_visible_test(PiGraph* graph, AStarNode* start_node, AStarNode* end_node, PiBool onlyStatic)
{
	
	return graph->adjacency_matrix[start_node->idx][end_node->idx] != GRAPH_MAX_DISTANCE;
}


/******public methods*****/
AStarEnv* PI_API s3d_astar_graph_env_create()
{
	AStarEnv* env = s3d_astar_env_create();
	astar_env_init(env, (GenReachableNodesFunc)get_reachable_nodes, (NodeVisibleTestFunc)_node_visible_test, (FixNodeToReachableFunc)_fix_node_to_reachable,(PiHashFunc)_node_idx_hash, (PiEqualFunc)_node_equal, (CalFFnc)_cal_f, (CalDistanceFunc)_cal_distance, (CalHFunc)_cal_h, (NodeImplInit)_node_impl_init, (NodeImplShutdown)_node_impl_shutdown, ASTAR_GRAPH);
	return env;
}

void PI_API s3d_astar_graph_env_destroy(AStarEnv* env)
{
    s3d_astar_env_destroy(env);
}

void PI_API s3d_astar_graph_env_init( AStarEnv *env, PiGraph *graph )
{
	PI_ASSERT(graph, "graph env init error!");
	env->impl = graph;
}

void PI_API s3d_astar_graph_env_shutdown( AStarEnv *env )
{
	env->impl = NULL;
	astar_env_shutdown(env);
}

void PI_API s3d_astar_graph_find_path(AStarEnv *env, VertexIndexType start_idx, VertexIndexType end_idx, PiBool only_static)
{
	AStarNode *start, *end;
	env->cache_idx = 0;
	start = astar_env_get_node_from_cache(env);
	end = astar_env_get_node_from_cache(env);
	start->idx = start_idx;
	end->idx = end_idx;
	astar_find_path(env, start, end, only_static);
}
