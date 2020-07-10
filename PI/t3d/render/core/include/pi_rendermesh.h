#ifndef INCLUDE_RENDERMESH_H
#define INCLUDE_RENDERMESH_H

#include <pi_lib.h>
#include <pi_mesh.h>

/**
 * ��Ⱦ����
 */

typedef struct 
{
	PiMesh *mesh;				/* ���� */
	uint face_num;				/*����*/
	uint vert_num;				/*������*/
	void *gpu_data;				/* GPU���� */
}PiRenderMesh;

PI_BEGIN_DECLS

/* ���� */
PiRenderMesh* PI_API pi_rendermesh_new(PiMesh *mesh_data, PiBool is_create_handle);

PiBool PI_API pi_rendermesh_init(PiRenderMesh *mesh);

/* ���� */
void PI_API pi_rendermesh_free(PiRenderMesh *mesh);

/* ����render mesh��layout */
void PI_API pi_rendermesh_update(PiRenderMesh *mesh);

void PI_API pi_rendermesh_update_by_buffers(PiRenderMesh* mesh, EGeometryType type, const IndexData* index_data, uint num_verts, uint num_verts_buffers, VertexElement** elements);


PI_END_DECLS

#endif /* INCLUDE_RENDERMESH_H */