#include <pi_rendermesh.h>
#include <renderwrap.h>

static PiBool _is_valid(PiRenderMesh *mesh)
{
	PiBool r = (mesh != NULL);
	if (!r)
	{
		pi_log_print(LOG_WARNING, "rendermesh is NULL");
	}

	r = r && (mesh->mesh != NULL);
	if (!r)
	{
		pi_log_print(LOG_WARNING, "rendermesh's mesh is NULL");
	}
	 
	r = r && (mesh->mesh->data.vertex_num > 0);
	if (!r)
	{
		pi_log_print(LOG_WARNING, "rendermesh's mesh invalid, vertex's num is 0");
	}

	r = r && (mesh->mesh->data.type > EGOT_UNKOWN) && (mesh->mesh->data.type < EGOT_NUM);
	if (!r)
	{
		pi_log_print(LOG_WARNING, "rendermesh's mesh type invalid = %d", mesh->mesh->data.type);
	}
	return r;
}

PiBool PI_API pi_rendermesh_init(PiRenderMesh *mesh)
{
	if (mesh->gpu_data != NULL)
	{
		pi_renderlayout_free(mesh->gpu_data);
		mesh->gpu_data = NULL;
	}

	if (_is_valid(mesh))
	{
		pi_rendermesh_update(mesh);
	}
	return TRUE;
}

PiRenderMesh* PI_API pi_rendermesh_new(PiMesh *mesh_data, PiBool is_create_handle)
{
	PiRenderMesh *r = pi_new0(PiRenderMesh, 1);
	if (mesh_data)
	{
		r->mesh = mesh_data;
		r->vert_num = pi_mesh_get_vertex_num(mesh_data);
		r->face_num = pi_mesh_get_face_num(mesh_data);
	}
	if (is_create_handle) 
	{
		pi_rendermesh_init(r);
	}
		
	return r;
}

void PI_API pi_rendermesh_free(PiRenderMesh *mesh)
{
	if(mesh != NULL)
	{
		if(mesh->gpu_data != NULL)
		{
			pi_renderlayout_free(mesh->gpu_data);
		}		
		pi_free(mesh);
	}	
}

void PI_API pi_rendermesh_update(PiRenderMesh *mesh)
{
	render_layout_update(mesh);
}

void PI_API pi_rendermesh_update_by_buffers(PiRenderMesh* mesh, EGeometryType type, const IndexData* index_data, uint num_verts, uint num_verts_buffers, VertexElement** elements)
{
	uint num = 0;
	switch (type)
	{
	case EGOT_POINT_LIST:
		num = num_verts;
		break;
	case EGOT_LINE_LIST:
		if (index_data->num > 0)
		{
			num = index_data->num / 2;
		}
		else
		{
			num = num_verts / 2;
		}
		break;
	case EGOT_LINE_STRIP:
		if (index_data->num  > 0)
		{
			num = index_data->num - 1;
		}
		else
		{
			num = num_verts - 1;
		}
		break;
	case EGOT_TRIANGLE_LIST:
		if (index_data->num > 0)
		{
			num = index_data->num / 3;
		}
		else
		{
			num = num_verts / 3;
		}
		break;
	case EGOT_TRIANGLE_STRIP:
		if (index_data->num  > 0)
		{
			num = index_data->num / 2;
		}
		else
		{
			num = (num_verts / 2);
		}
		break;
	case EGOT_TRIANGLE_FAN:
		if (index_data->num  > 0)
		{
			num = (index_data->num - 2);
		}
		else
		{
			num = (num_verts - 2);
		}
		break;
	case EGOT_QUAD_LIST:
		if (index_data->num  > 0)
		{
			num = index_data->num / 4;
		}
		else
		{
			num = num_verts / 4;
		}
		break;
	default:
		break;
	}
	mesh->vert_num = num_verts;
	mesh->face_num = num;
	render_layout_update_by_buffers(mesh, type, index_data, num_verts, num_verts_buffers, elements);

}