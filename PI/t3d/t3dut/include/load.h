#ifndef INCLUDE_LOAD_H
#define INCLUDE_LOAD_H

#include <pi_lib.h>
#include <pi_skeleton.h>
#include <rendersystem.h>

PI_BEGIN_DECLS

void *pi_load_file(const char *file_name, uint *size);

void pi_load_shader(char *key, char *path);

PiTexture *pi_load_texture(const char *file_name, uint num_mipmap);

PiImage *pi_load_image(char *file_name);

PiTexture *pi_load_cube_texture(char *px_name, char *nx_name, char *py_name, char *ny_name, char *pz_name, char *nz_name, uint mipmap);

PiEntity **pi_load_entity(const char *file_name, uint *p_num);

PiRenderMesh **pi_load_meshes(char *file_name, uint *num);

PiSkeleton *pi_load_skeleton(const char *file_name);

/*mesh”≈ªØ*/
PiMesh** PI_API pi_mesh_optimize(PiMesh** meshes, uint32 num);

PI_END_DECLS

#endif /* INCLUDE_LOAD_H */