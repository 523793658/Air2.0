#include "tiered_a_star.h"


TieredAStarEnv* PI_API s3d_tiered_astar_env_create()
{
	TieredAStarEnv* env = pi_new0(TieredAStarEnv, 1);
    env->astar_grid_env = s3d_astar_grid_env_create();
    env->astar_graph_env = s3d_astar_graph_env_create();
	return env;
}


void PI_API s3d_tiered_astar_env_free( TieredAStarEnv* env )
{
    s3d_astar_grid_env_destroy(env->astar_grid_env);
    s3d_astar_graph_env_destroy(env->astar_graph_env);
    pi_free(env);
}


void PI_API s3d_tiered_astar_env_init( TieredAStarEnv* env, GirdObstacle* obstacle, PiGraph* graph )
{
	//env->astar_grid_env = s3d_astar_grid_env_create();
	s3d_astar_gird_env_init(env->astar_grid_env, obstacle);
	env->obstacle = obstacle;
	//env->astar_graph_env = s3d_astar_graph_env_create();
	s3d_astar_graph_env_init(env->astar_graph_env, graph);
	env->graph = graph;
	env->result = pi_queue_new();
	pi_queue_init(env->result);
	pi_queue_push_head(env->result, &(env->result_idx));//第一位存缓冲索引，方便直接从结果中获取实际长度，每次寻路开始时设为1，实际定点数需-1
#if defined(_DEBUG)
	pi_log_print(LOG_INFO, "tiered init finished...........................");
#endif
}

void PI_API s3d_tiered_astar_env_shutdown(TieredAStarEnv* env)
{
    uint i;
    uint *node;
    env->obstacle = NULL;
    env->graph = NULL;
    for(i = 1; i < pi_queue_size(env->result); ++i)
    {
        node = (uint*)pi_queue_get(env->result, i);
        pi_free(node);
    }
    pi_queue_clear(env->result, TRUE);
    s3d_astar_gird_env_shutdown(env->astar_grid_env);
    s3d_astar_graph_env_shutdown(env->astar_graph_env);
}

uint* _get_node_from_cache(TieredAStarEnv *env)
{
	uint *node;
	PiQueue *cache = env->result;
	//容量不足,新建
	if(env->result_idx < pi_queue_size(cache))
	{
		node = (uint*)pi_queue_get(cache, env->result_idx);
	}
	else
	{
		node = pi_new0(uint, 1);
		pi_queue_push_tail(env->result, node);
	}

	env->result_idx += 1;
	
	return node;
}

static PiBool _is_obs(uint8 value)
{
	return value % 2 != 0;
}

void PI_API s3d_tiered_astar_findpath( TieredAStarEnv* env, PosType start_x, PosType start_y, PosType end_x, PosType end_y, int32 size, PiBool only_static )
{
	/*graph_vertex* start_vertex = pi_new0(graph_vertex, 1);
	graph_vertex* end_vertex = pi_new0(graph_vertex, 1);*/
	PiQueue *vertex_queue;// = pi_queue_new();
	PiQueue *grid_queue;
	AStarNode *node;
	EdgeType edgeValue;
	uint i, j, grid_queue_length;
	PiBool is_first_point = TRUE;
#if defined (_DEBUG)
	uint idx;
#endif
	VertexIndexType start_idx, end_idx;
	PosType x, y;
	AStarNode* grid_node;
	uint *idx_node;
	env->result_idx = 1;
	//修正起止点
	env->start_node.idx = start_y * env->obstacle->width + start_x;
	env->end_node.idx = end_y * env->obstacle->width + end_x;
	fix_node_to_reachable(env->obstacle, &env->end_node.idx, &env->start_node.idx);
	end_x = env->end_node.idx % env->obstacle->width;
	end_y = env->end_node.idx / env->obstacle->width;

	//判断起止点是否直连
	if(node_visible_test(env->obstacle, start_x, start_y, end_x, end_y, NULL, NULL, TRUE, NULL, NULL))
	{
		s3d_astar_gird_find_path(env->astar_grid_env, start_x, start_y, end_x, end_y, size, only_static);
		grid_queue = s3d_astar_get_result(env->astar_grid_env);
		grid_queue_length = pi_queue_size(grid_queue);
		if(grid_queue_length > 0)
		{
			for(i=0; i < pi_queue_size(grid_queue); ++i)
			{
				grid_node = (AStarNode*)pi_queue_get(grid_queue, i);
				idx_node = _get_node_from_cache(env);
				*idx_node = grid_node->idx;
			}
		}
		else
		{
			node_visible_test(env->obstacle, start_x, start_y, end_x, end_y, &end_x, &end_y, FALSE, NULL, NULL);
			idx_node = _get_node_from_cache(env);
			*idx_node = end_y * env->obstacle->width + end_x;
		}
	}
	else
	{
		//将起止点加入连通图
		start_idx = pi_graph_add_vertex(env->graph, start_x, start_y);
		end_idx = pi_graph_add_vertex(env->graph, end_x, end_y);
		env->graph->graph_vertexs[start_idx]->x = start_x;
		env->graph->graph_vertexs[start_idx]->y = start_y;
		env->graph->graph_vertexs[end_idx]->x = end_x;
		env->graph->graph_vertexs[end_idx]->y = end_y;
		for(i = 0; i < env->graph->vertex_num; ++i)
		{
			x = env->graph->graph_vertexs[i]->x;
			y = env->graph->graph_vertexs[i]->y;
			if(i != start_idx &&node_visible_test(env->obstacle, x, y, start_x, start_y, NULL, NULL, TRUE, NULL, NULL))
			{
				edgeValue = (EdgeType)(EdgeType)pi_math_sqrt((float)((start_x-x)*(start_x-x)+(start_y-y)*(start_y-y)));
				pi_graph_add_edge(env->graph, edgeValue, start_idx, i);
			}

			if(i != end_idx && node_visible_test(env->obstacle, x, y, end_x, end_y, NULL,NULL, TRUE, NULL, NULL))
			{
				edgeValue = (EdgeType)(EdgeType)pi_math_sqrt((float)((end_x-x)*(end_x-x)+(end_y-y)*(end_y-y)));
				pi_graph_add_edge(env->graph, edgeValue, end_idx, i);
			}
		}
		//在连通图内进行寻路
		s3d_astar_graph_find_path(env->astar_graph_env, start_idx, end_idx, only_static);
		vertex_queue = s3d_astar_get_result(env->astar_graph_env);

		//各顶点间寻路
		for(i = 0; i < pi_queue_size(vertex_queue); ++i)
		{
			node = (AStarNode*)pi_queue_get(vertex_queue, i);
			x = env->graph->graph_vertexs[node->idx]->x;
			y = env->graph->graph_vertexs[node->idx]->y;
			if(env->obstacle->data[y * env->obstacle->width + x]==0){
				
				if(is_first_point){
					s3d_astar_gird_find_path(env->astar_grid_env, start_x, start_y, x, y, size, only_static);
					grid_queue = s3d_astar_get_result(env->astar_grid_env);
					grid_queue_length = pi_queue_size(grid_queue);
					if(grid_queue_length > 0){
						for(j=0; j < grid_queue_length; ++j)
						{
							grid_node = (AStarNode*)pi_queue_get(grid_queue, j);
							idx_node = _get_node_from_cache(env);
							*idx_node = grid_node->idx;
						}
					}else
					{
						node_visible_test(env->obstacle, start_x, start_y, x, y, &end_x, &end_y, FALSE, NULL, NULL);
						idx_node = _get_node_from_cache(env);
						*idx_node = end_y * env->obstacle->width + end_x;
						break;
					}

					start_x = x;
					start_y = y;
					is_first_point = FALSE;
#if defined(_DEBUG)
					pi_log_print(LOG_INFO, "%d: pos = (%d,%d) f=%d", i, x, y, node->f);
#endif
				}else{
					idx_node = _get_node_from_cache(env);
					*idx_node = y * env->obstacle->width + x;
				}
			}
			if(node->f == 0){
				break;
			}
		}
		//从连通图中删除起止顶点
		pi_graph_remove_vertex(env->graph);
		pi_graph_remove_vertex(env->graph);
	}
	

#if defined (_DEBUG)
	pi_log_print(LOG_INFO, "**********Final Result*******");
	for(i = 1; i < pi_queue_size(env->result); ++i)
	{
		idx = *(uint*)pi_queue_get(env->result, i);
		x = idx % env->obstacle->width;
		y = idx / env->obstacle->width;
		pi_log_print(LOG_INFO,"%d. pos = (%d,%d)",i, x, y);
	}
	pi_log_print(LOG_INFO, "******Final Result End*******");
#endif
}

void PI_API s3d_tiered_astar_link_vertex( TieredAStarEnv* env )
{
	PiGraph *graph = env->graph;
	GirdObstacle* obstacle =env->obstacle;
	VertexIndexType i, j, n;
	PosType x1, y1, x2, y2;
	EdgeType edgeValue;
	n = env->graph->vertex_num;
	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < n; ++j)
		{
			if( i!=j )
			{
				x1 = graph->graph_vertexs[i]->x;
				y1 = graph->graph_vertexs[i]->y;
				x2 = graph->graph_vertexs[j]->x;
				y2 = graph->graph_vertexs[j]->y;
				if(node_visible_test(obstacle, x1, y1, x2, y2, NULL, NULL, TRUE, NULL, NULL))
				{
					edgeValue = (EdgeType)pi_math_sqrt((float)( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) ));
					pi_graph_add_edge(graph, edgeValue, i, j);
				}
			}
		}
	}
}

PiQueue* PI_API s3d_tiered_astar_get_result( TieredAStarEnv* env )
{
	return env->result;
}

uint PI_API s3d_tiered_get_index( uint *idx )
{
	return *idx;
}


static void _add_node(PiGraph* graph, PiBool* guides, uint width, uint height, uint x, uint y)
{
	pi_graph_add_vertex(graph, x, y);
	guides[y * width + x] = TRUE;
}

VertexIndexType PI_API s3d_tiered_astar_generate_guides( TieredAStarEnv* env )
{
	uint n = 0, x, y, height, width;
	uint8* obs;
	PiBool *guides;
	if(env->graph == NULL || env->obstacle == NULL)
	{
		return 0;
	}
	obs = env->obstacle->data;
	height = env->obstacle->height;
	width = env->obstacle->width;

	pi_graph_clear(env->graph);
	guides = pi_new0(PiBool, height*width);
	//n = env->obstacle->width * env->obstacle->height;
	for(x = 0; x < env->obstacle->height; ++x)
	{
		for(y = 0; y <env->obstacle->width; ++y)
		{
			if(!_is_obs( obs[y * width + x]))//中0
			{
				if(y > 0 && !_is_obs(obs[(y - 1) * width + x]))//上0
				{
					if(x > 0 && !_is_obs(obs[y * width + x - 1]) && _is_obs(obs[(y - 1) * width + x -1])) //左0 左上1
					{
						_add_node(env->graph, guides, width, height, x, y);
						++n;
						continue;
					}
					if(x + 1 < width && !_is_obs(obs[y * width + x + 1]) && _is_obs(obs[(y -1) * width + x + 1])) //右0 右上1
					{
						_add_node(env->graph, guides, width, height, x, y);
						++n;
						continue;
					}
				}
				if(y + 1 < height && !_is_obs(obs[(y + 1) * width + x]))//下0
				{
					if(x > 0 && !_is_obs(obs[y * width + x - 1]) && _is_obs(obs[(y + 1) * width + x - 1])) //左0 左下1
					{
						_add_node(env->graph, guides, width, height, x, y);
						++n;
						continue;
					}
					if(x +1 < width && !_is_obs(obs[y * width + x + 1]) && _is_obs(obs[y + 1 * width + x + 1])) //右0 右下1
					{
						_add_node(env->graph, guides, width, height, x, y);
						++n;
						continue;
					}
				}
			}
		}
	}
	return n;
}
