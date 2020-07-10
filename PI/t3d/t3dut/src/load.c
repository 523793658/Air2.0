#include "load.h"
#include <renderwrap.h>
#include "lightmap.h"
void pi_load_shader(char *a_key, char *a_path)
{
	int64 size;
	byte *data;
	wchar *key = pi_utf8_to_wstr(a_key);
	wchar *path = pi_utf8_to_wstr(a_path);

	void *f = pi_file_open(path, FILE_OPEN_READ);
	pi_file_size(f, &size);
	data = (byte *)pi_malloc((uint)size);
	pi_file_read(f, 0, FALSE, (char *)data, (uint)size);
	pi_file_close(f);

	pi_rendersystem_add_shader_source(key, data, (uint)size);
	pi_free(data);
	pi_free(key);
	pi_free(path);
}

void *pi_load_file(const char *file_name, uint *size)
{
	void *data = NULL;
	wchar *name = pi_utf8_to_wstr(file_name);
	void *file = pi_file_open(name, FILE_OPEN_READ);
	*size = 0;

	if (file != NULL)
	{
		int64 size64 = 0;
		pi_file_size(file, &size64);
		*size = (uint)size64;
		data = pi_malloc0(*size);
		pi_file_read(file, 0, FALSE, (char *)data, *size);
		pi_file_close(file);
	}
	return data;
}

PiTexture *pi_load_texture(const char *file_name, uint num_mipmap)
{
	PiTexture *tex;
	PiImage *image;

	uint size = 0;
	byte *data = (byte *)pi_load_file(file_name, &size);

	image = pi_render_image_load(data, size, FALSE);
	switch (image->type)
	{
	case TT_2D:
		tex = pi_texture_2d_create(image->format, TU_NORMAL, 1, num_mipmap, image->width, image->height, TRUE);
		break;
	case TT_3D:
		tex = pi_texture_3d_create(image->format, TU_NORMAL, num_mipmap, image->width, image->height, image->depth, TRUE);
		break;
	case TT_CUBE:
		tex = pi_texture_cube_create(image->format, TU_NORMAL, num_mipmap, image->width, TRUE);
		break;
	default:
		return NULL;
		break;
	}

	pi_texture_update_image(tex, image, 0);
	pi_render_image_free(image);
	pi_free(data);

	return tex;
}

PiImage *pi_load_image(char *file_name)
{
	PiImage *image;

	uint size = 0;

	byte *data = (byte *)pi_load_file(file_name, &size);
	image = pi_render_image_load(data, size, FALSE);
	pi_free(data);

	return image;
}

PiTexture *pi_load_cube_texture(char *px_name, char *nx_name, char *py_name, char *ny_name, char *pz_name, char *nz_name, uint mipmap)
{
	uint i, j;
	PiTexture *tex = NULL;
	char *names[6];
	names[0] = px_name;
	names[1] = nx_name;
	names[2] = py_name;
	names[3] = ny_name;
	names[4] = pz_name;
	names[5] = nz_name;

	for (i = 0; i < 6; ++i)
	{
		byte *p;
		uint size_byte;
		uint width;
		PiImage *image = pi_load_image(names[i]);
		if (i == 0)
		{
			tex = pi_texture_cube_create(image->format, TU_NORMAL, mipmap, image->width, TRUE);
		}
		else if (tex->format != image->format)
		{
			pi_texture_free(tex);
			pi_render_image_free(image);
			return NULL;
		}

		width = image->width;
		for (j = 0; j < image->num_mipmap; j++) {
			p = pi_render_image_get_pointer(image, 0, j, &size_byte);
			pi_texture_cube_update(tex, j, CF_POSITIVE_X + i, 0, 0, width, width, size_byte, p);
			width /= 2;
		}
		pi_render_image_free(image);
	}

	return tex;
}

PiMesh** PI_API pi_mesh_optimize(PiMesh** meshes, uint32 num)
{
	uint i;
	PiMesh** optimizeMesh = pi_new(PiMesh*, num);
	for (i = 0; i < num; i++)
	{
		optimizeMesh[i] = pi_new0(PiMesh, 1);
		pi_mesh_copy(optimizeMesh[i], meshes[i]);
		//d3d_mesh_optimize(optimizeMesh[i]);
	}
	return optimizeMesh;
}

PiEntity **pi_load_entity(const char *file_name, uint *p_num)
{
	PiEntity **es = NULL;
	PiMesh **meshes = NULL;
	uint e_num = 0, size = 0;

	byte *data = (byte *)pi_load_file(file_name, &size);
	if (size > 0)
	{
		e_num = pi_mesh_num(data, size);
		if (e_num > 0)
		{
			uint i;
			es = pi_new0(PiEntity *, e_num);
			meshes = pi_new0(PiMesh *, e_num);
			for (i = 0; i < e_num; ++i)
			{
				meshes[i] = pi_new0(PiMesh, 1);
			}
			pi_mesh_load(meshes, e_num, data, size);
			for (i = 0; i < e_num; ++i)
			{
				es[i] = pi_entity_new();
				pi_entity_set_mesh(es[i], pi_rendermesh_new(meshes[i], TRUE));
			}
		}
	}

	pi_free(data);
	pi_free(meshes);
	if (p_num != NULL)
	{
		*p_num = e_num;
	}
	return es;
}

PiRenderMesh **pi_load_meshes(char *file_name, uint *num)
{
	uint size = 0;
	byte *data = (byte *)pi_load_file(file_name, &size);
	if (size > 0)
	{
		uint mesh_num = pi_mesh_num(data, size);
		if (mesh_num > 0)
		{
			uint i;
			PiMesh **meshes = pi_new0(PiMesh *, mesh_num);
			PiRenderMesh **rmeshes = pi_new0(PiRenderMesh *, mesh_num);

			for (i = 0; i < mesh_num; ++i)
			{
				meshes[i] = pi_new0(PiMesh, 1);
			}

			pi_mesh_load(meshes, mesh_num, data, size);

			for (i = 0; i < mesh_num; ++i)
			{
				rmeshes[i] = pi_rendermesh_new(meshes[i], TRUE);
			}

			pi_free(meshes);

			if (num != NULL)
			{
				*num = mesh_num;
			}

			pi_free(data);
			return rmeshes;
		}

		pi_free(data);
	}

	return NULL;
}

PiSkeleton *pi_load_skeleton(const char *file_name)
{
	uint size = 0;
	PiSkeleton *sk = NULL;
	byte *data = (byte *)pi_load_file(file_name, &size);
	if (size > 0)
	{
		sk = pi_skeleton_new(data, size);
	}

	return sk;
}