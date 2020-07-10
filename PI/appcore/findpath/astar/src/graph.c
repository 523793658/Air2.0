#include "graph.h"

void PI_API pi_graph_print( PiGraph* graph )
{
	VertexIndexType i, j;
	pi_log_print(LOG_INFO, "===========GRAPH adjMatrix %d*%d============", graph->vertex_num, graph->vertex_num);
	for(i = 0; i < graph->vertex_num; ++i){
		printf("%d--(%d,%d)  ", i, graph->graph_vertexs[i]->x, graph->graph_vertexs[i]->y);//graph->graph_vertexs[i]);
		for(j = 0; j < graph->vertex_num; ++j){
			printf("%d | ", graph->adjacency_matrix[i][j]);
		}
		printf("\n");
	}
	pi_log_print(LOG_INFO, "------------GRAPH END----------");
}

PiGraph* PI_API pi_graph_new0()
{
	PiGraph* graph = pi_new0(PiGraph, 1);
	graph->vertex_num = 0;
	return graph;
}

PiGraph* PI_API pi_graph_new(VertexIndexType vertex_num,PosType* vertexData, EdgeType* data)
{
	PiGraph* graph = pi_new0(PiGraph, 1);
	pi_graph_init(graph, vertex_num, vertexData, data);
	return graph;
}

void PI_API pi_graph_free( PiGraph* graph )
{
	pi_graph_clear(graph);
	pi_free(graph);
}

void PI_API pi_graph_init(PiGraph* graph, VertexIndexType vertex_num,PosType* vertexData, EdgeType* data)
{
	VertexIndexType i, j;
	pi_graph_clear(graph);
	graph->vertex_num = vertex_num;
	/*for(i = 0; i < GRAPH_NODE_VERTEX_MAX; ++i)
	{
	graph->graph_vertexs[i] = NULL;
	for(j = 0; j < GRAPH_NODE_VERTEX_MAX; ++j)
	{
	graph->adjacency_matrix[i][j] = GRAPH_MAX_DISTANCE;
	}
	}*/
	for(i = 0; i < vertex_num; ++i)
	{
		graph->graph_vertexs[i] = pi_new0(graph_vertex, 1);
		graph->graph_vertexs[i]->x = vertexData[i * 2];
		graph->graph_vertexs[i]->y = vertexData[i * 2 + 1];
		for(j = 0 ; j < vertex_num; ++j)
		{
			graph->adjacency_matrix[i][j] = data[i * vertex_num + j];
		}
	}
}


void PI_API pi_graph_save( PiGraph* graph, char* path )
{
	FILE *pGraph;
	//byte *data;
	uint32 i, j;//, data_length;
	fopen_s(&pGraph, path, "wb");
	//data_length = sizeof(uint32) + sizeof(PosType)*graph->vertex_num + sizeof(EdgeType)*graph->vertex_num*graph->vertex_num;
	fwrite(&graph->vertex_num, sizeof(uint32), 1, pGraph);
	for(i=0;i<graph->vertex_num;++i)
	{
		fwrite(&graph->graph_vertexs[i]->x, sizeof(PosType), 1, pGraph);
		fwrite(&graph->graph_vertexs[i]->y, sizeof(PosType), 1, pGraph);
	}
	for(i = 0; i < graph->vertex_num; ++i)
	{
		for(j = 0; j < graph->vertex_num; ++j)
		{
			fwrite(&graph->adjacency_matrix[i][j], sizeof(EdgeType), 1, pGraph);
		}
	}
	fclose(pGraph);
	//data = pi_new0(byte, data_length);
}


void PI_API pi_graph_init_from_bin( PiGraph* graph, byte* data)
{
	PiBytes vertex_num_buffer, vertex_buffer, adj_matrix_buffer;
	uint32 i, j;
	pi_memset_inline(graph, 0, sizeof(graph));

	pi_bytes_load(&vertex_num_buffer, data, sizeof(uint32), FALSE);
	pi_bytes_read_uint32(&vertex_num_buffer, &graph->vertex_num);
	data += sizeof(uint32);
	pi_bytes_clear(&vertex_num_buffer, FALSE);
	
	for(i = 0; i < graph->vertex_num; ++i)
	{
		graph->graph_vertexs[i] = pi_new0(graph_vertex, 1);
		pi_bytes_load(&vertex_buffer, data, sizeof(PosType), FALSE);
		pi_bytes_read_int32(&vertex_buffer, &graph->graph_vertexs[i]->x);
		data += sizeof(PosType);
		pi_bytes_load(&vertex_buffer, data, sizeof(PosType), FALSE);
		pi_bytes_read_int32(&vertex_buffer, &graph->graph_vertexs[i]->y);
		data += sizeof(PosType);
	}
	pi_bytes_clear(&vertex_buffer, FALSE);
	//todo 读取还能优化
	for(i = 0; i < graph->vertex_num; ++i)
	{
		for(j = 0; j < graph->vertex_num; ++j)
		{
			pi_bytes_load(&adj_matrix_buffer, data, sizeof(EdgeType), FALSE);
			pi_bytes_read_int32(&adj_matrix_buffer, &graph->adjacency_matrix[i][j]);
			data += sizeof(EdgeType);
		}
	}
	pi_bytes_clear(&adj_matrix_buffer, FALSE);
}


void PI_API pi_graph_clear( PiGraph* graph )
{
	VertexIndexType i, j;
	for(i = 0; i < graph->vertex_num; ++i)
	{
		if(graph->graph_vertexs[i] != NULL)
		{
			pi_free(graph->graph_vertexs[i]);
			graph->graph_vertexs[i] = NULL;
		}
		for(j = 0; j < graph->vertex_num; ++j)
		{
			graph->adjacency_matrix[i][j] = GRAPH_MAX_DISTANCE;
		}
	}
	graph->vertex_num = 0;
}

PiBool PI_API pi_graph_is_vertex_connected( PiGraph* graph, VertexIndexType index1, VertexIndexType index2 )
{
	return graph->adjacency_matrix[index1][index2] != GRAPH_MAX_DISTANCE;
}


VertexIndexType PI_API pi_graph_add_vertex( PiGraph* graph, PosType x, PosType y)
{
	graph_vertex* vertex_new = pi_new0(graph_vertex, 1);
	VertexIndexType idx;
	VertexIndexType i;
	vertex_new->x = x;
	vertex_new->y = y;
	idx = graph->vertex_num;
	graph->graph_vertexs[idx] = vertex_new;
	++graph->vertex_num;
	for(i = 0 ; i < graph->vertex_num; ++i)
	{
		graph->adjacency_matrix[i][idx] = GRAPH_MAX_DISTANCE;
		graph->adjacency_matrix[idx][i] = GRAPH_MAX_DISTANCE;
	}
#if defined (_DEBUG)
	//pi_graph_print(graph);
#endif
	return idx;
}

void PI_API pi_graph_remove_vertex( PiGraph* graph )
{
	VertexIndexType i;
	if(graph->vertex_num <= 0)
	{
		pi_log_print(LOG_WARNING, "vertex removing error, no vertex left..");
		return;
	}
	--graph->vertex_num;
	for(i = 0 ; i < graph->vertex_num; ++i)
	{
		graph->adjacency_matrix[i][graph->vertex_num] = GRAPH_MAX_DISTANCE;
		graph->adjacency_matrix[graph->vertex_num][i] = GRAPH_MAX_DISTANCE;
	}

	pi_free(graph->graph_vertexs[graph->vertex_num]);
}

void PI_API pi_graph_add_edge(PiGraph* graph, EdgeType value, VertexIndexType index_start, VertexIndexType index_end)
{
	if(index_start < graph->vertex_num && index_end < graph->vertex_num && index_start != index_end)
	{
		graph->adjacency_matrix[index_start][index_end] = value;
		graph->adjacency_matrix[index_end][index_start] = value;
	}
#if defined (_DEBUG)
	//pi_graph_print(graph);
#endif
}

void PI_API pi_graph_remove_edge(PiGraph* graph, VertexIndexType index_start, VertexIndexType index_end)
{
	if(index_start < graph->vertex_num && index_end < graph->vertex_num && index_start != index_end)
	{
		graph->adjacency_matrix[index_start][index_end] = GRAPH_MAX_DISTANCE;
		graph->adjacency_matrix[index_end][index_start] = GRAPH_MAX_DISTANCE;
	}
}

uint PI_API pi_graph_get_vertex_num( PiGraph* graph )
{
	return graph->vertex_num;
}

PosType* PI_API pi_graph_get_vertexs( PiGraph* graph )
{
	VertexIndexType i;
	PosType *result = pi_new0(PosType, graph->vertex_num * 2);
	for(i = 0; i < graph->vertex_num; ++i)
	{
		result[i * 2] = graph->graph_vertexs[i]->x;
		result[i * 2 + 1] = graph->graph_vertexs[i]->y;
	}
	return result;
}

uint PI_API pi_graph_get_max_distance()
{
	return GRAPH_MAX_DISTANCE;
}


