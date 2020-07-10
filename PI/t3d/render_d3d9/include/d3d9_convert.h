#ifndef INCLUDE_D3D9_CONVERT_H
#define INCLUDE_D3D9_CONVERT_H

/**
 * 将t3d的枚举转成D3D9对应的值
 */

#include "renderstate.h"
#include "pi_renderdata.h"
#include <d3d9.h>

PI_BEGIN_DECLS

D3DCUBEMAP_FACES d3d9_cube_map_face_get(TextureCubeFace face);

D3DFILLMODE d3d9_polygon_get(PolygonMode mode);

D3DCULL d3d9_cull_mode_get(CullMode mode);

D3DSHADEMODE d3d9_shade_mode_get(ShadeMode mode);

D3DCMPFUNC d3d9_compare_func_get(CompareFunction func);

D3DSTENCILOP d3d9_stencil_op_get(StencilOperation op);

D3DBLENDOP d3d9_blend_op_get(BlendOperation bo);

D3DBLEND d3d9_blend_factor_get(BlendFactor factor);

uint d3d9_color_write_mask_get(uint8 color_mask);

D3DTEXTUREADDRESS d3d9_tex_addr_get(TexAddressMode mode);

void d3d9_tex_filter_get(TexFilterOp filter, D3DTEXTUREFILTERTYPE *mag_filter, D3DTEXTUREFILTERTYPE *min_filter, D3DTEXTUREFILTERTYPE *mip_filter);

D3DPRIMITIVETYPE d3d9_primitive_type_get(EGeometryType type);

uint d3d9_indexed_primitive_count_get(EGeometryType type, uint num_indexes);

uint d3d9_primitive_count_get(EGeometryType type, uint num_vertices);

D3DFORMAT d3d9_tex_format_get(RenderFormat fmt);

void d3d9_tex_usage_get(TextureUsage usage, uint *d3d9_usage, D3DPOOL *d3d9_pool);

void d3d9_vertex_semantic_get(VertexSemantic semantic, uint8 *decl_usage, uint8 *usage_index);

void d3d9_vertex_type_get(EVertexType type, uint32 num_components, uint8 *decl_type, uint *stride);

void d3d9_buffer_usage_get(EBufferUsage usage, uint *d3d9_usage, D3DPOOL *d3d9_pool, uint *d3d9_lock_flags);

D3DMULTISAMPLE_TYPE d3d9_multi_sample_get(MULTISAMPLE_TYPE type);

PI_END_DECLS

#endif /* INCLUDE_D3D9_CONVERT_H */
