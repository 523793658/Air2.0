#include <a_star.h>

static PiBool PI_API _idx_equal(const uint32* idx1, const uint32* idx2)
{
	return *idx1 == *idx2;
}

static PiCompR PI_API _node_compare(void *impl, const AStarNode *p1, const AStarNode *p2)
{
	return p1->f > p2->f ? PI_COMP_GREAT : PI_COMP_LESS;
}

void astar_env_init(AStarEnv *env, GenReachableNodesFunc gen_reachable_nodes_func, NodeVisibleTestFunc node_visible_test_func, 
					FixNodeToReachableFunc fix_node_to_reachable_func,
					PiHashFunc node_hash_func, PiEqualFunc node_equal_func, 
					CalFFnc cal_f_func, CalDistanceFunc cal_distance_func, CalHFunc cal_h_func,
					NodeImplInit node_impl_init_func, NodeImplShutdown node_impl_shutdown_func, AStarType findType)
{
	env->gen_reachable_nodes = gen_reachable_nodes_func;
	env->node_visible_test = node_visible_test_func;
	env->fix_node_to_reachable = fix_node_to_reachable_func;
	env->cal_f = cal_f_func;
	env->cal_distance = cal_distance_func;
	env->cal_h = cal_h_func;
	env->node_impl_init = node_impl_init_func;
	env->node_impl_shutdown = node_impl_shutdown_func;
	env->node_equal = node_equal_func;
	
	env->open_hash = pi_hash_new(0.75f, node_hash_func, (PiEqualFunc)_idx_equal);
	env->close_hash = pi_hash_new(0.75f, node_hash_func, (PiEqualFunc)_idx_equal);
	env->findType = findType;
}

void astar_env_shutdown(AStarEnv *env)
{
	uint32 i;
	AStarNode *node;

	pi_queue_clear(env->results_queue, TRUE);
	pi_hash_clear(env->open_hash, TRUE);
	pi_hash_free(env->open_hash);
	pi_hash_clear(env->close_hash, TRUE);
	pi_hash_free(env->close_hash);
	pi_heap_clear(env->open_heap, TRUE);

	for(i=0; i < pi_vector_size(env->node_cache); ++i)
	{
		node = (AStarNode *)pi_vector_get(env->node_cache, i);
		env->node_impl_shutdown(node);
		pi_free(node);
	}
	pi_vector_clear(env->node_cache, TRUE);
	env->cache_idx = 0;
}

void astar_find_path(AStarEnv *env, AStarNode *start, AStarNode *end, PiBool only_static)
{
	AStarNode *curr_node = NULL;
	PiHeap *open_heap = env->open_heap;
	PiHash *close_hash = env->close_hash;
	PiHash *open_hash = env->open_hash;
	uint i, num;
	PiBool find = FALSE;
	uint32 num_step = 0;
	uint heap_size;

	//重置一些数据
	pi_queue_clear(env->results_queue, FALSE);
	pi_hash_clear(env->open_hash, TRUE);
	pi_hash_clear(env->close_hash, TRUE);
	pi_heap_clear(env->open_heap, FALSE);

	//修正起终点
	//env->fix_node_to_reachable(env->impl, start->impl, end->impl);
	env->fix_node_to_reachable(env->impl, end, start);


	//start 加入openlist
	pi_heap_push(open_heap, start);
	pi_hash_insert(open_hash, &(start->idx), start);


	//start 做为curr_node
	curr_node = start;

	//如果openlist非空
	heap_size = pi_heap_size(open_heap);
	while(heap_size && !find && num_step <= MAX_STEP)
	{

		//取出代价最小的node, 加入到closelist, 同时设置其parent为curr_node
		AStarNode *node = (AStarNode*)pi_heap_pop(open_heap);
		if(node == NULL){
#if defined (_DEBUG)
			pi_log_print(LOG_INFO, "no result..");
#endif
			find = FALSE;
			break;
		}

		//若此node为终点,设置此node的parent为curr_node,寻路结束.
		if(env->node_equal(node, end))
		{
			/*node->parent = curr_node;
			end->parent = curr_node;*/
			end->parent = node->parent;
			find = TRUE;
			break;
		}
		pi_hash_delete(open_hash, &(node->idx), NULL);

		pi_hash_insert(close_hash, &(node->idx), node);

		num_step += 1;

		//将此node设置为curr_node
		curr_node = node;

		//pi_log_print(LOG_INFO, "curr node is %d, %d\n", ((GirdNode*)curr_node->impl)->idx % 512 ,  ((GirdNode*)curr_node->impl)->idx / 512 );

		//若与终点不能直连
		if (env->findType == ASTAR_GRAPH || !env->node_visible_test(env->impl, curr_node, end, only_static))
		{
			//遍历curr_node可达的node
			num = env->gen_reachable_nodes(env, env->impl, curr_node, only_static);
			for(i=0; i < num; ++i)
			{
				//node = (AStarNode*)pi_vector_get( env->node_cache, env->cache_idx-i );
				node = (AStarNode *)pi_vector_get(env->reachable_node_cache, i);
				//忽略已经在closelist的node
				if (!pi_hash_lookup(close_hash, &(node->idx), NULL))
				{
					//若此node为终点,设置此node的parent为curr_node,寻路结束.
						//计算其代价,并加入openlist, 设置parent为curr_node, 若openlist已有此node,且此次计算出的代价更大,则不重设置parent
						//f = env->cal_f(env->impl, node, curr_node, start, end);
						//h = env->cal_h(env->impl, node, end);
						if(!pi_hash_lookup(open_hash, &(node->idx), NULL))
						{
							node->parent = curr_node;
							env->cal_h(env->impl, node, end);
							//node->g = curr_node->g + env->cal_distance(env->impl, node, curr_node);
							node->f = node->g+node->h;
							pi_hash_insert(open_hash, &(node->idx), node);
							pi_heap_push(open_heap, node);
#if defined(_DEBUG)
							if(env->findType == ASTAR_GRAPH){
								pi_log_print(LOG_INFO, "%d--g=%d h=%d f=%d,paren=%d", node->idx, node->g,node->h,node->f,node->parent->idx);
							}
#endif
						}
						else if(curr_node->g + env->cal_distance(env->impl, node, curr_node) < node->g){
							node->g = curr_node->g + env->cal_distance(env->impl, node, curr_node);
							node->f = node->g + node->h;
							node->parent = curr_node;
						}
				}
			}
		}
		else
		{
			end->parent = curr_node;
			find = TRUE;
		}
	}

	//组织寻路结果
	if(find)
	{
		AStarNode *node = end;
		AStarNode *last_node = node;
        AStarNode *child_node = node;
		pi_queue_push_head(env->results_queue, node);

		while(node != NULL && node != start)
		{
			if (env->node_visible_test(env->impl, start, last_node, only_static))
				break;

            //如果当前节点和上一个计入节点不能直连，把当前节点的前一个节点计入
            //由于当前节点理论上和之前节点肯定直连，所以可以直接计算当前节点的下一个节点
			if (!env->node_visible_test(env->impl, node, last_node, only_static))
			//if (node != last_node)
            {
				pi_queue_push_head(env->results_queue, child_node);
				last_node = child_node;
            }
            child_node = node;
			node = node->parent;
		}

		if (node == start && !env->node_visible_test(env->impl, node, last_node, only_static))
        {
            pi_queue_push_head(env->results_queue, child_node);
        }
#if defined (_DEBUG)
		pi_log_print(LOG_INFO,"num_step = %d", num_step);
#endif
	}
}

AStarNode* astar_env_get_node_from_cache(AStarEnv *env)
{
	AStarNode *node;
	PiVector *cache = env->node_cache;
	//容量不足,新建
	if(env->cache_idx < pi_vector_size(cache))
	{
		node = (AStarNode*)pi_vector_get(cache, env->cache_idx);
		node->f = 0;
		node->parent = NULL;
	}
	else
	{
		node = pi_new0(AStarNode, 1);
		pi_vector_push(env->node_cache, node);
	}

	env->node_impl_init(node);
	env->cache_idx += 1;
	
	return node;
}



AStarEnv* PI_API s3d_astar_env_create()
{
    AStarEnv *env = pi_new0(AStarEnv, 1);
    //astar_env_init(env, _gen_reachable_nodes, node_visible_test_func, fix_node_to_reachable, _node_hash, _node_equal, _cal_f, _gird_node_impl_init, _gird_node_impl_shutdown);
	env->results_queue = pi_queue_new();
	pi_queue_init(env->results_queue);

	env->open_heap = pi_heap_new((PiCompareFunc)_node_compare, env->impl, FALSE);
	
	env->node_cache = pi_vector_new();
	env->reachable_node_cache = pi_vector_new();
	pi_vector_init(env->node_cache);
	env->cache_idx = 0;

	return env;
}

void PI_API s3d_astar_env_destroy(AStarEnv *env)
{
	pi_queue_free(env->results_queue);
	pi_vector_free(env->node_cache);
	pi_vector_free(env->reachable_node_cache);
	pi_heap_free(env->open_heap);
	pi_free(env);
}

PiQueue* PI_API s3d_astar_get_result( AStarEnv *env )
{
	return env->results_queue;
}
