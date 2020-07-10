
#ifndef INCLUDE_GL_RENDERLAYOUT_H
#define INCLUDE_GL_RENDERLAYOUT_H

#include <renderlayout.h>
#include <rendersystem.h>

typedef struct  
{
	uint num;
	uint gl_type;
	uint gl_vbo_id;
	uint buffer_size;
}VertexLayout;

typedef struct
{
	uint gl_primary_type;

	uint index_num;
	uint index_size;
	uint gl_index_type;
	uint gl_index_id;

	uint gl_vao_id;
	
	uint vertex_num;
	VertexLayout vertexs[EVS_NUM];
}GLRenderLayout;

PI_BEGIN_DECLS

/* ------------------ 供OpenGL模块调用 */
PiBool PI_API gl_renderlayout_draw(PiRenderMesh *mesh, uint num);

PI_END_DECLS

#endif /* INCLUDE_RENDERLAYOUT_H */