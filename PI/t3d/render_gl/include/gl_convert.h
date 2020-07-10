#ifndef INCLUDE_GL_CONVERT_H
#define INCLUDE_GL_CONVERT_H

#include <renderstate.h>
#include <pi_renderdata.h>

PI_BEGIN_DECLS

uint gl_polygon_get(PolygonMode mode);

uint gl_compare_func_get(CompareFunction func);

uint gl_stencil_op_get(StencilOperation op);

uint gl_blend_op_get(BlendOperation bo);

uint gl_blend_factor_get(BlendFactor factor);

uint gl_tex_addr_get(TexAddressMode mode);

void gl_tex_filter_get(TexFilterOp filter, uint *min_filter, uint *mag_filter);

uint gl_primitive_get(EGeometryType type);

uint gl_tex_target_get(TextureType type, uint array_size);

void gl_tex_format_get(RenderFormat fmt, uint *internal_fmt, uint *gl_fmt, uint *gl_type);

void gl_vertex_format_get(RenderFormat rf, uint *gl_type, PiBool *is_normalized);

uint gl_tex_face_get(TextureCubeFace face);

uint gl_vertex_type_get(EVertexType type);

uint gl_buffer_usage_get(EBufferUsage usage);

PI_END_DECLS

#endif /* INCLUDE_GL_CONVERT_H */