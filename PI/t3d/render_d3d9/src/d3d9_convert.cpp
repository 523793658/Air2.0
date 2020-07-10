#include "d3d9_convert.h"

#include <rendersystem.h>
#include "d3d9_context.h"
extern "C" extern PiRenderSystem *g_rsystem;

D3DCUBEMAP_FACES d3d9_cube_map_face_get(TextureCubeFace face)
{
	D3DCUBEMAP_FACES r = D3DCUBEMAP_FACE_POSITIVE_X;
	switch (face)
	{
	case CF_POSITIVE_X:
		r = D3DCUBEMAP_FACE_POSITIVE_X;
		break;
	case CF_NEGATIVE_X:
		r = D3DCUBEMAP_FACE_NEGATIVE_X;
		break;
	case CF_POSITIVE_Y:
		r = D3DCUBEMAP_FACE_POSITIVE_Y;
		break;
	case CF_NEGATIVE_Y:
		r = D3DCUBEMAP_FACE_NEGATIVE_Y;
		break;
	case CF_POSITIVE_Z:
		r = D3DCUBEMAP_FACE_POSITIVE_Z;
		break;
	case CF_NEGATIVE_Z:
		r = D3DCUBEMAP_FACE_NEGATIVE_Z;
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", face);
		break;
	}
	return r;
}

D3DFILLMODE d3d9_polygon_get(PolygonMode mode)
{
	D3DFILLMODE r = D3DFILL_SOLID;
	switch (mode)
	{
	case PM_POINT:
		r = D3DFILL_POINT;
		break;
	case PM_LINE:
		r = D3DFILL_WIREFRAME;
		break;
	case PM_FILL:
		r = D3DFILL_SOLID;
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", mode);
		break;
	}
	return r;
}

D3DSHADEMODE d3d9_shade_mode_get(ShadeMode mode)
{
	D3DSHADEMODE m = D3DSHADE_GOURAUD;
	switch (mode)
	{
	case SM_FLAT:
		m = D3DSHADE_FLAT;
		break;
	case SM_GOURAUD:
		m = D3DSHADE_GOURAUD;
		break;
	case SM_PHONG:
		m = D3DSHADE_PHONG;
		break;
	case SM_FORCE_DWORD:
		m = D3DSHADE_FORCE_DWORD;
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", mode);
		break;
	}
	return m;
}

D3DCULL d3d9_cull_mode_get(CullMode mode)
{
	D3DCULL r = D3DCULL_NONE;
	RasterizerState *state = &g_rsystem->rs;
	switch (mode)
	{
	case CM_NO:
		r = D3DCULL_NONE;
		break;
	case CM_FRONT:
		r = state->is_front_face_ccw ? D3DCULL_CCW : D3DCULL_CW;
		break;
	case CM_BACK:
		r = state->is_front_face_ccw ? D3DCULL_CW : D3DCULL_CCW;
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", mode);
		break;
	}
	return r;
}

D3DCMPFUNC d3d9_compare_func_get(CompareFunction func)
{
	D3DCMPFUNC r = D3DCMP_LESS;
	switch (func)
	{
	case CF_ALWAYSFAIL:
		r = D3DCMP_NEVER;
		break;
	case CF_ALWAYSPASS:
		r = D3DCMP_ALWAYS;
		break;
	case CF_LESS:
		r = D3DCMP_LESS;
		break;
	case CF_LESSEQUAL:
		r = D3DCMP_LESSEQUAL;
		break;
	case CF_EQUAL:
		r = D3DCMP_EQUAL;
		break;
	case CF_NOTEQUAL:
		r = D3DCMP_NOTEQUAL;
		break;
	case CF_GREATEREQUAL:
		r = D3DCMP_GREATEREQUAL;
		break;
	case CF_GREATER:
		r = D3DCMP_GREATER;
		break;
	default:
		PI_ASSERT(FALSE, "invalid func = %d", func);
		break;
	}
	return r;
}

D3DSTENCILOP d3d9_stencil_op_get(StencilOperation op)
{
	D3DSTENCILOP r = D3DSTENCILOP_KEEP;
	switch (op)
	{
	case SOP_KEEP:
		r = D3DSTENCILOP_KEEP;
		break;
	case SOP_ZERO:
		r = D3DSTENCILOP_ZERO;
		break;
	case SOP_REPLACE:
		r = D3DSTENCILOP_REPLACE;
		break;
	case SOP_INCR:
		r = D3DSTENCILOP_INCRSAT;
		break;
	case SOP_DECR:
		r = D3DSTENCILOP_DECRSAT;
		break;
	case SOP_INVERT:
		r = D3DSTENCILOP_INVERT;
		break;
	case SOP_INCR_WRAP:
		r = D3DSTENCILOP_INCR;
		break;
	case SOP_DECR_WRAP:
		r = D3DSTENCILOP_DECR;
		break;
	default:
		PI_ASSERT(FALSE, "invalid op = %d", op);
		break;
	}
	return r;
}

D3DBLENDOP d3d9_blend_op_get(BlendOperation bo)
{
	D3DBLENDOP r = D3DBLENDOP_ADD;
	switch (bo)
	{
	case BOP_ADD:
		r = D3DBLENDOP_ADD;
		break;
	case BOP_SUB:
		r = D3DBLENDOP_SUBTRACT;
		break;
	case BOP_REV_SUB:
		r = D3DBLENDOP_REVSUBTRACT;
		break;
	case BOP_MIN:
		r = D3DBLENDOP_MIN;
		break;
	case BOP_MAX:
		r = D3DBLENDOP_MAX;
		break;
	default:
		PI_ASSERT(FALSE, "invalid bo = %d", bo);
		break;
	}
	return r;
}

D3DBLEND d3d9_blend_factor_get(BlendFactor factor)
{
	D3DBLEND r = D3DBLEND_ONE;
	switch (factor)
	{
	case BF_ZERO:
		r = D3DBLEND_ZERO;
		break;
	case BF_ONE:
		r = D3DBLEND_ONE;
		break;
	case BF_SRC_ALPHA:
		r = D3DBLEND_SRCALPHA;
		break;
	case BF_DST_ALPHA:
		r = D3DBLEND_DESTALPHA;
		break;
	case BF_INV_SRC_ALPHA:
		r = D3DBLEND_INVSRCALPHA;
		break;
	case BF_INV_DST_ALPHA:
		r = D3DBLEND_INVDESTALPHA;
		break;
	case BF_SRC_COLOR:
		r = D3DBLEND_SRCCOLOR;
		break;
	case BF_DST_COLOR:
		r = D3DBLEND_DESTCOLOR;
		break;
	case BF_INV_SRC_COLOR:
		r = D3DBLEND_INVSRCCOLOR;
		break;
	case BF_INV_DST_COLOR:
		r = D3DBLEND_INVDESTCOLOR;
		break;
	case BF_SRC_ALPHA_SAT:
		r = D3DBLEND_SRCALPHASAT;
		break;
	default:
		PI_ASSERT(FALSE, "invalid factor = %d", factor);
		break;
	}
	return r;
}

uint d3d9_color_write_mask_get(uint8 color_mask)
{
	uint r = 0;
	if (color_mask & CMASK_RED)
	{
		r |= D3DCOLORWRITEENABLE_RED;
	}
	if (color_mask & CMASK_GREEN)
	{
		r |= D3DCOLORWRITEENABLE_GREEN;
	}
	if (color_mask & CMASK_BLUE)
	{
		r |= D3DCOLORWRITEENABLE_BLUE;
	}
	if (color_mask & CMASK_ALPHA)
	{
		r |= D3DCOLORWRITEENABLE_ALPHA;
	}
	return r;
}

D3DTEXTUREADDRESS d3d9_tex_addr_get(TexAddressMode mode)
{
	D3DTEXTUREADDRESS r = D3DTADDRESS_WRAP;
	switch (mode)
	{
	case TAM_WRAP:
		r = D3DTADDRESS_WRAP;
		break;
	case TAM_MIRROR:
		r = D3DTADDRESS_MIRROR;
		break;
	case TAM_CLAMP:
		r = D3DTADDRESS_CLAMP;
		break;
	case TAM_BORDER:
		r = D3DTADDRESS_BORDER;
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", mode);
		break;
	}
	return r;
}

void d3d9_tex_filter_get(TexFilterOp filter, D3DTEXTUREFILTERTYPE *mag_filter, D3DTEXTUREFILTERTYPE *min_filter, D3DTEXTUREFILTERTYPE *mip_filter)
{
	if (filter & TFOE_ANISOTROPIC)
	{
		*mag_filter = D3DTEXF_ANISOTROPIC;
		*min_filter = D3DTEXF_ANISOTROPIC;
		*mip_filter = D3DTEXF_LINEAR;
		return;
	}

	if (filter & TFOE_MAG_LINEAR)
	{
		*mag_filter = D3DTEXF_LINEAR;
	}
	else
	{
		*mag_filter = D3DTEXF_POINT;
	}

	if (filter & TFOE_MIN_LINEAR)
	{
		*min_filter = D3DTEXF_LINEAR;
	}
	else
	{
		*min_filter = D3DTEXF_POINT;
	}

	if (filter & TFOE_MIP_LINEAR)
	{
		*mip_filter = D3DTEXF_LINEAR;
	}
	else if (filter & TFOE_MIP_POINT)
	{
		*mip_filter = D3DTEXF_POINT;
	}
	else
	{
		*mip_filter = D3DTEXF_NONE;
	}
}

D3DPRIMITIVETYPE d3d9_primitive_type_get(EGeometryType type)
{
	D3DPRIMITIVETYPE d3d9_primitive_type = D3DPT_TRIANGLELIST;

	switch (type)
	{
	case EGOT_POINT_LIST:
		d3d9_primitive_type = D3DPT_POINTLIST;
		break;
	case EGOT_LINE_LIST:
		d3d9_primitive_type = D3DPT_LINELIST;
		break;
	case EGOT_LINE_STRIP:
		d3d9_primitive_type = D3DPT_LINESTRIP;
		break;
	case EGOT_TRIANGLE_STRIP:
		d3d9_primitive_type = D3DPT_TRIANGLESTRIP;
		break;
	case EGOT_TRIANGLE_FAN:
		d3d9_primitive_type = D3DPT_TRIANGLEFAN;
		break;
	case EGOT_TRIANGLE_LIST:
		d3d9_primitive_type = D3DPT_TRIANGLELIST;
		break;
	default:
		PI_ASSERT(FALSE, "invalid primitive type = %d", type);
		break;
	}
	return d3d9_primitive_type;
}

uint d3d9_indexed_primitive_count_get(EGeometryType type, uint num_indexes)
{
	uint primitive_count = 0;
	switch (type)
	{
	case EGOT_POINT_LIST:
		primitive_count = num_indexes;
		break;
	case EGOT_LINE_LIST:
		primitive_count = num_indexes / 2;
		break;
	case EGOT_LINE_STRIP:
		primitive_count = num_indexes - 1;
		break;
	case EGOT_TRIANGLE_STRIP:
		primitive_count = num_indexes - 2;
		break;
	case EGOT_TRIANGLE_FAN:
		primitive_count = num_indexes - 2;
		break;
	case EGOT_TRIANGLE_LIST:
		primitive_count = num_indexes / 3;
		break;
	default:
		PI_ASSERT(FALSE, "invalid primitive type = %d", type);
		break;
	}
	return primitive_count;
}

uint d3d9_primitive_count_get(EGeometryType type, uint num_vertices)
{
	uint primitive_count = 0;
	switch (type)
	{
	case EGOT_POINT_LIST:
		primitive_count = num_vertices;
		break;
	case EGOT_LINE_LIST:
		primitive_count = num_vertices / 2;
		break;
	case EGOT_LINE_STRIP:
		primitive_count = num_vertices - 1;
		break;
	case EGOT_TRIANGLE_STRIP:
		primitive_count = num_vertices - 2;
		break;
	case EGOT_TRIANGLE_FAN:
		primitive_count = num_vertices - 2;
		break;
	case EGOT_TRIANGLE_LIST:
		primitive_count = num_vertices / 3;
		break;
	default:
		PI_ASSERT(FALSE, "invalid primitive type = %d", type);
		break;
	}
	return primitive_count;
}

D3DFORMAT d3d9_tex_format_get(RenderFormat fmt)
{
	D3DFORMAT d3d9_format = D3DFMT_UNKNOWN;
	uint64 impl = pi_renderformat_get_impl(fmt);
	switch (impl)
	{
	case RFIMPL_A8:
		d3d9_format = D3DFMT_A8;
		break;
	case RFIMPL_ABGR8:
		d3d9_format = D3DFMT_A8R8G8B8;
		break;
	case RFIMPL_GR16:
		d3d9_format = D3DFMT_G16R16;
		break;
	case RFIMPL_ABGR16:
		d3d9_format = D3DFMT_A16B16G16R16;
		break;
	case RFIMPL_R16F:
		d3d9_format = D3DFMT_R16F;
		break;
	case RFIMPL_GR16F:
		d3d9_format = D3DFMT_G16R16F;
		break;
	case RFIMPL_ABGR16F:
		d3d9_format = D3DFMT_A16B16G16R16F;
		break;
	case RFIMPL_R32F:
		d3d9_format = D3DFMT_R32F;
		break;
	case RFIMPL_GR32F:
		d3d9_format = D3DFMT_G32R32F;
		break;
	case RFIMPL_ABGR32F:
		d3d9_format = D3DFMT_A32B32G32R32F;
		break;
	case RFIMPL_BC1:
		d3d9_format = D3DFMT_DXT1;
		break;
	case RFIMPL_BC2:
		d3d9_format = D3DFMT_DXT3;
		break;
	case RFIMPL_BC3:
		d3d9_format = D3DFMT_DXT5;
		break;
	case RFIMPL_D16:
		d3d9_format = D3DFMT_D16;
		break;
	case RFIMPL_D32F:
		d3d9_format = D3DFMT_D32;
	case RFIMPL_D24S8:
		d3d9_format = D3DFMT_D24S8;
		break;
	case RFIMPL_INTZ:
		d3d9_format = FOURCC_INTZ;
		break;
	case RFIMPL_NULL:
		d3d9_format = FOURCC_NULL;
		break;
	default:
		PI_ASSERT(FALSE, "unsupported texture format = %d", impl);
	}

	return d3d9_format;
}

void d3d9_tex_usage_get(TextureUsage usage, uint *d3d9_usage, D3DPOOL *d3d9_pool)
{
	switch (usage)
	{
	case TU_DEPTH_STENCIL:
		*d3d9_usage = D3DUSAGE_DEPTHSTENCIL;
		*d3d9_pool = D3DPOOL_DEFAULT;
		break;
	case TU_COLOR:
		*d3d9_usage = D3DUSAGE_RENDERTARGET;
		*d3d9_pool = D3DPOOL_DEFAULT;
		break;
	case TU_NORMAL:
		*d3d9_usage = 0;
		*d3d9_pool = D3DPOOL_MANAGED;
		break;
	default: 
		break;
	}
}

void d3d9_vertex_semantic_get(VertexSemantic semantic, uint8 *decl_usage, uint8 *usage_index)
{
	switch (semantic)
	{
	case EVS_POSITION:
		*decl_usage = D3DDECLUSAGE_POSITION;
		*usage_index = 0;
		break;
	case EVS_NORMAL:
		*decl_usage = D3DDECLUSAGE_NORMAL;
		*usage_index = 0;
		break;
	case EVS_DIFFUSE:
		*decl_usage = D3DDECLUSAGE_COLOR;
		*usage_index = 0;
		break;
	case EVS_SPECULAR:
		*decl_usage = D3DDECLUSAGE_COLOR;
		*usage_index = 1;
		break;
	case EVS_BINORMAL:
		*decl_usage = D3DDECLUSAGE_BINORMAL;
		*usage_index = 0;
		break;
	case EVS_TANGENT:
		*decl_usage = D3DDECLUSAGE_TANGENT;
		*usage_index = 0;
		break;
	case EVS_BLEND_WEIGHTS:
		*decl_usage = D3DDECLUSAGE_BLENDWEIGHT;
		*usage_index = 0;
		break;
	case EVS_BLEND_INDICES:
		*decl_usage = D3DDECLUSAGE_BLENDINDICES;
		*usage_index = 0;
		break;
	case EVS_TEXCOORD_0:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 0;
		break;
	case EVS_TEXCOORD_1:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 1;
		break;
	case EVS_TEXCOORD_2:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 2;
		break;
	case EVS_TEXCOORD_3:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 3;
		break;
	case EVS_TEXCOORD_4:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 4;
		break;
	case EVS_TEXCOORD_5:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 5;
		break;
	case EVS_TEXCOORD_6:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 6;
		break;
	case EVS_INSTANCE:
		*decl_usage = D3DDECLUSAGE_TEXCOORD;
		*usage_index = 7;
		break;
	default:
		break;
	}
}

void d3d9_vertex_type_get(EVertexType type, uint32 num_components, uint8 *decl_type, uint *stride)
{
	switch (type)
	{
	case EVT_UNSIGNED_BYTE:
		switch (num_components)
		{
		case 4:
			*decl_type = D3DDECLTYPE_UBYTE4;
			*stride = 4;
			break;
		default:
			PI_ASSERT(FALSE, "invalid vertex type = %d", type);
			break;
		}
		break;
	case EVT_SHORT:
		switch (num_components)
		{
		case 2:
			*decl_type = D3DDECLTYPE_SHORT2;
			*stride = 4;
			break;
		case 4:
			*decl_type = D3DDECLTYPE_SHORT4;
			*stride = 8;
			break;
		default:
			PI_ASSERT(FALSE, "invalid vertex type = %d", type);
			break;
		}
		break;
	case EVT_UNSIGNED_SHORT:
		switch (num_components)
		{
		case 2:
			*decl_type = D3DDECLTYPE_SHORT2;
			*stride = 4;
			break;
		case 4:
			*decl_type = D3DDECLTYPE_SHORT4;
			*stride = 8;
			break;
		default:
			PI_ASSERT(FALSE, "invalid vertex type = %d", type);
			break;
		}
		break;
	case EVT_FLOAT:
		switch (num_components)
		{
		case 1:
			*decl_type = D3DDECLTYPE_FLOAT1;
			*stride = 4;
			break;
		case 2:
			*decl_type = D3DDECLTYPE_FLOAT2;
			*stride = 8;
			break;
		case 3:
			*decl_type = D3DDECLTYPE_FLOAT3;
			*stride = 12;
			break;
		case 4:
			*decl_type = D3DDECLTYPE_FLOAT4;
			*stride = 16;
			break;
		default:
			PI_ASSERT(FALSE, "invalid vertex type = %d", type);
			break;
		}
		break;
	default:
		PI_ASSERT(FALSE, "invalid vertex type = %d", type);
		break;
	}
}

void d3d9_buffer_usage_get(EBufferUsage usage, uint *d3d9_usage, D3DPOOL *d3d9_pool, uint *d3d9_lock_flags)
{
	switch (usage)
	{
	case EVU_STATIC_DRAW:
		*d3d9_usage = D3DUSAGE_WRITEONLY;
		*d3d9_pool = D3DPOOL_MANAGED;
		*d3d9_lock_flags = 0;
		break;
	case EVU_DYNAMIC_DRAW:
		*d3d9_usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
		*d3d9_pool = D3DPOOL_DEFAULT;
		*d3d9_lock_flags = D3DLOCK_DISCARD;
		break;
	default:
		break;
	}
}

D3DMULTISAMPLE_TYPE d3d9_multi_sample_get(MULTISAMPLE_TYPE type)
{
	D3DMULTISAMPLE_TYPE d3d9Type;
	switch (type)
	{
	case MULTISAMPLE_NONE:
		d3d9Type = D3DMULTISAMPLE_NONE;
		break;
	case MULTISAMPLE_NONMASKABLE:
		d3d9Type = D3DMULTISAMPLE_NONMASKABLE;
		break;
	case MULTISAMPLE_2_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_2_SAMPLES;
		break;
	case MULTISAMPLE_3_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_3_SAMPLES;
		break; 
	case MULTISAMPLE_4_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_4_SAMPLES;
		break;
	case MULTISAMPLE_5_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_5_SAMPLES;
		break;
	case MULTISAMPLE_6_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_6_SAMPLES;
		break;
	case MULTISAMPLE_7_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_7_SAMPLES;
		break;
	case MULTISAMPLE_8_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_8_SAMPLES;
		break;
	case MULTISAMPLE_9_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_9_SAMPLES;
		break;
	case MULTISAMPLE_10_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_10_SAMPLES;
		break;
	case MULTISAMPLE_11_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_11_SAMPLES;
		break;
	case MULTISAMPLE_12_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_12_SAMPLES;
		break;
	case MULTISAMPLE_13_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_13_SAMPLES;
		break;
	case MULTISAMPLE_14_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_14_SAMPLES;
		break;
	case MULTISAMPLE_15_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_15_SAMPLES;
		break;
	case MULTISAMPLE_16_SAMPLES:
		d3d9Type = D3DMULTISAMPLE_16_SAMPLES;
		break;
	case MULTISAMPLE_FORCE_DWORD:
		d3d9Type = D3DMULTISAMPLE_FORCE_DWORD;
		break;
	default:
		d3d9Type = D3DMULTISAMPLE_NONE;
		break;
	}
	return d3d9Type;
}