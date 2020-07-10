#ifndef INCLUDE_D3D9_RENDERLAYOUT_H
#define INCLUDE_D3D9_RENDERLAYOUT_H

#include "pi_rendermesh.h"
#include <d3d9.h>

typedef struct
{
	uint stride;
	uint buffer_size;
	uint d3d9_usage;
	D3DPOOL d3d9_pool;
	uint d3d9_lock_flags;
	IDirect3DVertexBuffer9 *vertex_buffer;
	D3DVERTEXELEMENT9 decl;
} D3D9VertexElement;

typedef struct  
{
	uint buffer_size;
	uint d3d9_index_usage;
	uint d3d9_index_lock_flags;
	uint index_num;
	D3DFORMAT d3d9_index_format;
	D3DPOOL d3d9_index_pool;
	IDirect3DIndexBuffer9 *d3d9_ib;

}D3D9IndexElement;

typedef struct
{
	D3DPRIMITIVETYPE primitive_type;
	uint primitive_count;

	uint num_vertices;

	uint buffer_size;
	uint d3d9_index_format;
	uint d3d9_index_usage;
	D3DPOOL d3d9_index_pool;
	uint d3d9_index_lock_flags;
	IDirect3DIndexBuffer9 *d3d9_ib;

	IDirect3DVertexDeclaration9 *vertex_declaration;

	D3D9VertexElement elements[EVS_NUM];
} D3D9RenderLayout;

PI_BEGIN_DECLS

void d3d9_create_ib(D3D9RenderLayout *layout);

void d3d9_create_vb(D3D9VertexElement *element);

void d3d9_release_ib(D3D9RenderLayout *layout);

void d3d9_release_vb(D3D9VertexElement *element);

/* ------------------ 供t3d_d3d9模块调用 */
PiBool d3d9_renderlayout_draw(PiRenderMesh *mesh, PiRenderMesh* skinedMesh, uint num);

PI_END_DECLS

#endif /* INCLUDE_RENDERLAYOUT_H */
