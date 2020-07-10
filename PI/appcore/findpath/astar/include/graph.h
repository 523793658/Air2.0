#ifndef __INCLUDE_GRAPH_H__
#define __INCLUDE_GRAPH_H__

#include <pi_lib.h>
#include <stdio.h>
PI_BEGIN_DECLS

#if (defined(__linux) || defined(__linux__))
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==NULL
#endif

#define GRAPH_NODE_VERTEX_MAX 6000
#define GRAPH_MAX_DISTANCE 100000000
typedef int32 EdgeType;
typedef int32 PosType;
//typedef int32 VertexKeyType;
typedef uint32 VertexIndexType;
typedef struct
{
	PosType x;
	PosType y;
	//VertexKeyType key;
	/*VertexIndexType idx;
	EdgeType g;
	EdgeType h;*/
}graph_vertex;

typedef struct graph_node
{
	/*VertexIndexType idx;
	EdgeType g;
	EdgeType h;*/
	uint32 idx1;
}graph_node;

typedef struct 
{
	VertexIndexType vertex_num;
	graph_vertex* graph_vertexs[GRAPH_NODE_VERTEX_MAX];
	EdgeType adjacency_matrix[GRAPH_NODE_VERTEX_MAX][GRAPH_NODE_VERTEX_MAX];
}PiGraph;

//************************************
// Method:    pi_graph_new0
// FullName:  pi_graph_new0
// Access:    public 
// Returns:   PiGraph* PI_API
// Qualifier:
// 创建没有顶点的图
//************************************
PiGraph* PI_API pi_graph_new0();

PiGraph* PI_API pi_graph_new(VertexIndexType vertex_num, PosType* vertexData, EdgeType* data);

void PI_API pi_graph_init(PiGraph* graph, VertexIndexType vertex_num,PosType* vertexData, EdgeType* data);

void PI_API pi_graph_init_from_bin(PiGraph* graph, byte *data);

void PI_API pi_graph_save(PiGraph* graph, char* path);

void PI_API pi_graph_clear(PiGraph* graph);

void PI_API pi_graph_free(PiGraph* graph);

PiBool PI_API pi_graph_is_vertex_connected(PiGraph* graph, VertexIndexType index1, VertexIndexType index2);

//************************************
// Method:    pi_graph_add_vertex
// FullName:  pi_graph_add_vertex
// Access:    public 
// Returns:   VertexIndexType PI_API
// Qualifier:
// Parameter: PiGraph * graph
// Parameter: PosType x
// Parameter: PosType y
// 在图最后添加顶点，坐标为(x, y)
//************************************
VertexIndexType PI_API pi_graph_add_vertex(PiGraph* graph, /*VertexKeyType key,*/ PosType x, PosType y);

//************************************
// Method:    pi_graph_remove_vertex
// FullName:  pi_graph_remove_vertex
// Access:    public 
// Returns:   void PI_API
// Qualifier:
// Parameter: PiGraph * graph
// 删除最后一个顶点
//************************************
void PI_API pi_graph_remove_vertex(PiGraph* graph/*, VertexKeyType key*/);

//************************************
// Method:    pi_graph_add_edge
// FullName:  pi_graph_add_edge
// Access:    public 
// Returns:   void PI_API
// Qualifier:
// Parameter: PiGraph * graph
// Parameter: EdgeType value
// Parameter: VertexIndexType index_start
// Parameter: VertexIndexType index_end
// 连接顶点index_star和index_end，长度为value
//************************************
void PI_API pi_graph_add_edge(PiGraph* graph, EdgeType value, VertexIndexType index_start, VertexIndexType index_end/*, VertexKeyType key_start, VertexKeyType key_end*/);

//************************************
// Method:    pi_graph_remove_edge
// FullName:  pi_graph_remove_edge
// Access:    public 
// Returns:   void PI_API
// Qualifier:
// Parameter: PiGraph * graph
// Parameter: VertexIndexType index_start
// Parameter: VertexIndexType index_end
// 删除index_start和index_end之间的边
//************************************
void PI_API pi_graph_remove_edge(PiGraph* graph, VertexIndexType index_start, VertexIndexType index_end /*VertexKeyType key_start, VertexKeyType key_end*/);

//************************************
// Method:    pi_graph_print
// FullName:  pi_graph_print
// Access:    public 
// Returns:   void PI_API
// Qualifier:
// Parameter: PiGraph * graph
// 打印图的邻接矩阵
//************************************

uint PI_API pi_graph_get_vertex_num(PiGraph* graph);

PosType* PI_API pi_graph_get_vertexs(PiGraph* graph);

EdgeType* PI_API pi_graph_get_adjacency(PiGraph* graph);

uint PI_API pi_graph_get_max_distance();

void PI_API pi_graph_print(PiGraph* graph);

PI_END_DECLS

#endif

