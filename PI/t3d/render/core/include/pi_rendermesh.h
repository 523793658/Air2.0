#ifndef INCLUDE_RENDERMESH_H
#define INCLUDE_RENDERMESH_H

#include <pi_lib.h>
#include <pi_mesh.h>

/**
 * 渲染网格
 */

typedef struct 
{
	PiMesh *mesh;				/* 网格 */
	uint face_num;				/*面数*/
	uint vert_num;				/*顶点数*/
	void *gpu_data;				/* GPU数据 */
}PiRenderMesh;

PI_BEGIN_DECLS

/* 创建 */
PiRenderMesh* PI_API pi_rendermesh_new(PiMesh *mesh_data, PiBool is_create_handle);

PiBool PI_API pi_rendermesh_init(PiRenderMesh *mesh);

/* 销毁 */
void PI_API pi_rendermesh_free(PiRenderMesh *mesh);

/* 更新render mesh的layout */
void PI_API pi_rendermesh_update(PiRenderMesh *mesh);

void PI_API pi_rendermesh_update_by_buffers(PiRenderMesh* mesh, EGeometryType type, const IndexData* index_data, uint num_verts, uint num_verts_buffers, VertexElement** elements);


PI_END_DECLS

#endif /* INCLUDE_RENDERMESH_H */