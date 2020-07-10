#ifndef INCLUDE_T3D_RENDER_H
#define INCLUDE_T3D_RENDER_H

#include <pi_lib.h>

#include <entity.h>
#include <texture.h>
#include <renderview.h>
#include <renderstate.h>
#include <renderlayout.h>
#include <rendersystem.h>

/**
 * 渲染底层的封装
 * 所有渲染的动态库模块都要实现这些接口
 */

PI_BEGIN_DECLS

/** ------------ graphics buffer ------------ */


/** ------------ render system ------------ */

PiBool PI_API render_system_init(PiRenderSystem *rs, RenderContextLoadType type, uint width, uint height, void *data);

PiBool PI_API render_system_clear(PiRenderSystem *rs);

RenderCheckType PI_API render_system_check(PiRenderSystem *rs);

PiBool PI_API render_system_resize_check(PiRenderSystem *rs, PiBool is_window, uint width, uint height);

PiBool PI_API render_system_reset(PiRenderSystem *rs, PiBool is_window, uint width, uint height);

PiBool PI_API render_system_set_program(PiRenderSystem *rs, GpuProgram *program);

PiBool PI_API render_system_set_target(PiRenderSystem *rs, PiRenderTarget* rt);

PiBool PI_API render_system_draw(PiRenderSystem *rs, PiEntity *entity, uint num);

void PI_API render_system_swapbuffer(PiRenderSystem *rs);

void PI_API render_system_set_default_uniform(uint register_index, void* data, uint register_count, PiBool is_vs, PiBool is_ps);

PiBool PI_API render_system_clearview(PiRenderSystem *rs, uint32 flags, PiColor *clr, float depth, uint stencil);

PiBool PI_API render_system_begin(PiRenderSystem *rs);

PiBool PI_API render_system_end(PiRenderSystem *rs);

uint PI_API rendersystem_get_device_vendor(PiRenderSystem *rs);

/** ------------ render state ------------ */

PiBool PI_API render_force_def_state(void);

PiBool PI_API render_state_set(RenderStateType key, uint32 value);

PiBool PI_API render_state_set_list(StateList *lst);

uint32 PI_API render_state_get(RenderStateType key);

/** ------------ shader ------------ */

PiBool PI_API render_shader_init(Shader *shader);

PiBool PI_API render_shader_clear(Shader *shader);

PiBool PI_API render_create_shader_from_buffer(Shader *shader, void* buffer);

PiBool PI_API render_shader_compile(Shader *shader, const char *src_data, uint src_data_size, uint num_defines, const char *const *defines, void* output_buffer, uint32* size, PiBool save_buffer);

PiBool PI_API render_program_init(GpuProgram *program);

PiBool PI_API render_program_clear(GpuProgram *program);

PiBool PI_API render_program_attach(GpuProgram *program, void *shader);

PiBool PI_API render_program_dettach(GpuProgram *program, ShaderType type);

PiBool PI_API render_program_link(GpuProgram *program);

uint PI_API render_shader_compile_offline(ShaderType type, const char *src_data, uint src_data_size, uint num_defines, const char *const *defines, void** buffer);

/* 该函数的参数uniform就是从get_uniforms取出来的结构，内存值已经更新到最新，直接设置底层api即可，无需比较 */
void PI_API render_program_set_uniform(GpuProgram *program, Uniform *uniform, Uniform* material_uniform);

/* 返回该program所包含的所有uniform组成的数组，已经按名字从小到大排序 */
PiDvector *PI_API render_program_get_uniforms(GpuProgram *program, PiBool is_global);

/* 清空所有的uniform值 */
PiBool PI_API render_program_clear_uniforms(GpuProgram *program);

/** ------------ texture ------------ */

PiBool PI_API render_texture_init(PiTexture *texture);

PiBool PI_API render_texture_clear(PiTexture *texture);

PiBool PI_API render_texture_build_mipmap(PiTexture *texture);

PiBool PI_API render_texture_2d_update(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h, uint data_size, byte *data);

PiBool PI_API render_texture_3d_update(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, uint data_size, byte *data);

PiBool PI_API render_texture_cube_update(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h, uint data_size, byte *data);

PiImage* PI_API render_texture_2d_get(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h);

PiImage* PI_API render_texture_cube_get(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h);

PiBool PI_API render_texture_2d_copy(PiTexture *dst, PiTexture *src,
	uint dst_array_index, uint dst_level, uint dst_x, uint dst_y,
	uint src_array_index, uint src_level, uint src_x, uint src_y, uint w, uint h);

PiBool PI_API render_texture_3d_copy(PiTexture *dst, PiTexture *src,
	uint dst_level, uint dst_x, uint dst_y, uint dst_z, 
	uint src_level, uint src_x, uint src_y, uint src_z, uint w, uint h, uint d);

PiBool PI_API render_texture_cube_copy(PiTexture *dst, PiTexture *src,
	TextureCubeFace dst_face, uint dst_level, uint dst_x, uint dst_y, 
	TextureCubeFace src_face, uint src_level, uint src_x, uint src_y, uint w, uint h);

void* PI_API render_texture_get_curr_sampler(PiTexture *tex);

/** ------------ render view ------------ */

PiBool PI_API render_view_init(PiRenderView *view);

PiBool PI_API render_view_clear(PiRenderView *view);

PiBool PI_API render_view_on_attach(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment);

PiBool PI_API render_view_on_detach(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment);

PiBool PI_API render_view_on_bind(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment);

PiBool PI_API render_view_on_unbind(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment);

/** ------------ render target ------------ */

PiBool PI_API render_target_init(PiRenderTarget *target);

PiBool PI_API render_target_clear(PiRenderTarget* target);

PiBool PI_API render_target_copy(PiRenderTarget *dst, PiRenderTarget *src, TargetBufferMask mask, TargetFilterType filter);

PiBool PI_API render_target_set_viewport(PiRenderTarget* target, uint32 left, uint32 bottom, uint32 width, uint32 height);

PiBool PI_API render_target_bind(PiRenderTarget *target);

PiBool PI_API render_target_unbind(PiRenderTarget *target);

/** ------------ render layout ------------ */

PiBool PI_API render_layout_init(PiRenderLayout *layout);

PiBool PI_API render_layout_clear(PiRenderLayout *layout);

PiBool PI_API render_layout_update(PiRenderMesh *mesh);

PiBool PI_API render_layout_update_by_buffers(PiRenderMesh* mesh, EGeometryType type, const IndexData* index_data, uint num_verts, uint num_vert_buffers, VertexElement** vertex_buffers);

PiBool PI_API vertex_element_init(VertexElement* element);

PiBool PI_API vertex_element_free(VertexElement* vertex_element);

void PI_API vertex_element_update(VertexElement* element, uint first_vertex, uint num_vertex, const void* data, const uint32 srcStride);

PiBool PI_API index_data_init(IndexData* index_data);

PiBool PI_API index_data_free(IndexData* index_data);

void PI_API index_data_update(IndexData* index_data, uint offset, uint num, const void * data);

PiMesh* PI_API mesh_atlas_create(PiMesh* mesh);

PI_END_DECLS

#endif /* INCLUDE_T3D_RENDER_H */