#include "d3d9_renderstate.h"
#include "d3d9_convert.h"
#include "d3d9_rendersystem.h"
#include "d3d9_shader.h"
#include "d3d9_texture.h"
#include "d3d9_renderview.h"
#include "rendersystem.h"
#include "renderinfo.h"

extern "C" extern PiRenderSystem *g_rsystem;

static void rs_set_polygon_mode(PolygonMode mode)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->polygon_mode != mode)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		D3DFILLMODE fill_mode = d3d9_polygon_get(mode);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_FILLMODE, fill_mode);
		state->polygon_mode = mode;
	}
}

// 注：因为要涉及到正面和背面的问题，所以这里不能用比较的方式去减少设置
static void rs_set_cull_mode(CullMode cull)
{
	RasterizerState *state = &g_rsystem->rs;
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	D3DCULL cull_mode = d3d9_cull_mode_get(cull);

	IDirect3DDevice9_SetRenderState(context->device, D3DRS_CULLMODE, cull_mode);
	state->cull_mode = cull;
}

static void rs_set_shade_mode(ShadeMode mode)
{
	RasterizerState *state = &g_rsystem->rs;
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem*)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	D3DSHADEMODE shadeMode = d3d9_shade_mode_get(mode);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_SHADEMODE, shadeMode);
	state->shading_mode = mode;
}

static void rs_set_frontface(PiBool is_ccw)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->is_front_face_ccw != is_ccw)
	{
		state->is_front_face_ccw = is_ccw;
		rs_set_cull_mode(state->cull_mode);
	}
}

static void rs_set_polygon_offset(float factor, float bias)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->polygon_offset_factor != factor)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD *)&factor);
		state->polygon_offset_factor = factor;
	}

	if (state->polygon_offset_units != bias)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_DEPTHBIAS, *(DWORD *)&bias);
		state->polygon_offset_units = bias;
	}
}

static void rs_set_depthclip_enable(PiBool is_enable)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->is_depth_clip_enable != is_enable)
	{
		state->is_depth_clip_enable = is_enable;
		PI_ASSERT(FALSE, "D3D9 can't support this feature: depth clip togd3d9_e");
	}
}

static void rs_set_scissor_enable(PiBool is_enable)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->is_scissor_enable != is_enable)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_SCISSORTESTENABLE, is_enable);
		state->is_scissor_enable = is_enable;
	}
}

static void rs_set_multisample_enable(PiBool is_enable)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->is_multisample_enable != is_enable)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_MULTISAMPLEANTIALIAS, is_enable);
		state->is_multisample_enable = is_enable;
	}
}

static void dss_set_depth_enable(PiBool is_enable)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->is_depth_enable != is_enable)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_ZENABLE, is_enable);
		state->is_depth_enable = is_enable;
	}
}

static void dss_set_depthwrite_enable(PiBool is_enable)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->is_depth_write_mask != is_enable)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_ZWRITEENABLE, is_enable);
		state->is_depth_write_mask = is_enable;
	}
}

static void dss_set_depth_compfunc(CompareFunction func)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->depth_func != func)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		D3DCMPFUNC val = d3d9_compare_func_get(func);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_ZFUNC, val);
		state->depth_func = func;
	}
}

static void dss_set_stencil_enable(PiBool is_enable)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->is_stencil_enable != is_enable)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILENABLE, is_enable);
		state->is_stencil_enable = is_enable;
	}
}

static void dss_set_stencil(uint8 read_mask, uint8 write_mask, uint8 ref)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	DepthStencilState *state = &g_rsystem->dss;
	if (state->stencil_read_mask != read_mask)
	{
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILMASK, read_mask);
		state->stencil_read_mask = read_mask;
	}
	if (state->stencil_write_mask != write_mask)
	{
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILWRITEMASK, write_mask);
		state->stencil_write_mask = write_mask;
	}
	if (state->stencil_ref != ref)
	{
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILREF, ref);
		state->stencil_ref = ref;
	}
}

static void dss_set_ccw_stencil_op(DepthStencilState *state, D3D9Context *context,
	StencilOperation fail_op, StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	if (state->front_stencil_fail != fail_op)
	{
		D3DSTENCILOP stencil_op = d3d9_stencil_op_get(fail_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILFAIL, stencil_op);
		state->front_stencil_fail = fail_op;
	}
	if (state->front_stencil_depth_fail != depth_fail_op)
	{
		D3DSTENCILOP stencil_op = d3d9_stencil_op_get(depth_fail_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILZFAIL, stencil_op);
		state->front_stencil_depth_fail = depth_fail_op;
	}
	if (state->front_stencil_pass != stencil_pass_op)
	{
		D3DSTENCILOP stencil_op = d3d9_stencil_op_get(stencil_pass_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILPASS, stencil_op);
		state->front_stencil_pass = stencil_pass_op;
	}
	if (state->front_stencil_func != func)
	{
		D3DCMPFUNC cmp_func = d3d9_compare_func_get(func);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILFUNC, cmp_func);
		state->front_stencil_func = func;
	}
}

static void dss_set_cw_stencil_op(DepthStencilState *state, D3D9Context *context, 
	StencilOperation fail_op, StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	if (state->back_stencil_fail != fail_op)
	{
		D3DSTENCILOP stencil_op = d3d9_stencil_op_get(fail_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILFAIL, stencil_op);
		state->back_stencil_fail = fail_op;
	}
	if (state->back_stencil_depth_fail != depth_fail_op)
	{
		D3DSTENCILOP stencil_op = d3d9_stencil_op_get(depth_fail_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILZFAIL, stencil_op);
		state->back_stencil_depth_fail = depth_fail_op;
	}
	if (state->back_stencil_pass != stencil_pass_op)
	{
		D3DSTENCILOP stencil_op = d3d9_stencil_op_get(stencil_pass_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILPASS, stencil_op);
		state->back_stencil_pass = stencil_pass_op;
	}
	if (state->back_stencil_func != func)
	{
		D3DCMPFUNC cmp_func = d3d9_compare_func_get(func);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILFUNC, cmp_func);
		state->back_stencil_func = func;
	}
}

static void bs_set_a2c_enable(PiBool is_enable)
{
	BlendState *state = &g_rsystem->bs;
	if (state->is_alpha_to_coverage_enable != is_enable)
	{
		state->is_alpha_to_coverage_enable = is_enable;
		PI_ASSERT(FALSE, "D3D9 can't support this feature: alpha to coverage");
	}
}

static void bs_set_independent_blend_enable(PiBool is_enable)
{
	BlendState *state = &g_rsystem->bs;
	if (state->is_independent_blend_enable != is_enable)
	{
		state->is_independent_blend_enable = is_enable;
		PI_ASSERT(FALSE, "D3D9 can't support this feature: independent blend");
	}
}

static void bs_set_blend(PiBool is_enable)
{
	BlendState *state = &g_rsystem->bs;
	if (state->is_blend_enable != is_enable)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		if (is_enable)
		{
			IDirect3DDevice9_SetRenderState(context->device, D3DRS_ALPHABLENDENABLE, TRUE);
		}
		else
		{
			IDirect3DDevice9_SetRenderState(context->device, D3DRS_ALPHABLENDENABLE, FALSE);
		}

		state->is_blend_enable = is_enable;
	}
}

static void bs_set_blend_op(BlendOperation alpha_op, BlendOperation color_op)
{
	BlendState *state = &g_rsystem->bs;
	if (state->blend_op != color_op)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DBLENDOP blend_op = d3d9_blend_op_get(color_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_BLENDOP, blend_op);

		state->blend_op = color_op;
	}

	if (state->blend_op_alpha != alpha_op)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DBLENDOP blend_op = d3d9_blend_op_get(alpha_op);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_BLENDOPALPHA, blend_op);

		state->blend_op_alpha = alpha_op;
	}
}

static void bs_set_blend_factor(BlendFactor src_blend, BlendFactor dest_blend, BlendFactor src_blend_alpha, BlendFactor dest_blend_alpha)
{
	BlendState *state = &g_rsystem->bs;

	if (state->src_blend != src_blend)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DBLEND blend_factor = d3d9_blend_factor_get(src_blend);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_SRCBLEND, blend_factor);

		state->src_blend = src_blend;
	}

	if (state->dest_blend != dest_blend)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DBLEND blend_factor = d3d9_blend_factor_get(dest_blend);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_DESTBLEND, blend_factor);

		state->dest_blend = dest_blend;
	}

	if (state->src_blend_alpha != src_blend_alpha)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DBLEND blend_factor = d3d9_blend_factor_get(src_blend_alpha);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_SRCBLENDALPHA, blend_factor);

		state->src_blend_alpha = src_blend_alpha;
	}

	if (state->dest_blend_alpha != dest_blend_alpha)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DBLEND blend_factor = d3d9_blend_factor_get(dest_blend_alpha);
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_DESTBLENDALPHA, blend_factor);

		state->dest_blend_alpha = dest_blend_alpha;
	}
}

static void bs_set_color_mask(uint8 color_mask)
{
	BlendState *state = &g_rsystem->bs;
	if (state->color_write_mask != color_mask)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		uint mask = d3d9_color_write_mask_get(color_mask);

		IDirect3DDevice9_SetRenderState(context->device, D3DRS_COLORWRITEENABLE, mask);

		state->color_write_mask = color_mask;
	}
}

static void force_def_rs(RasterizerState *rs)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	D3DFILLMODE fill_mode;
	D3DCULL cull_mode;

	fill_mode = d3d9_polygon_get(rs->polygon_mode);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_FILLMODE, fill_mode);

	cull_mode = d3d9_cull_mode_get(rs->cull_mode);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_CULLMODE, cull_mode);

	IDirect3DDevice9_SetRenderState(context->device, D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD *)&rs->polygon_offset_factor);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_DEPTHBIAS, *(DWORD *)&rs->polygon_offset_units);

	IDirect3DDevice9_SetRenderState(context->device, D3DRS_SCISSORTESTENABLE, rs->is_scissor_enable);

	IDirect3DDevice9_SetRenderState(context->device, D3DRS_MULTISAMPLEANTIALIAS, rs->is_multisample_enable);
}

static void force_def_dss(RasterizerState *rs, DepthStencilState *dss)
{
	D3DCMPFUNC cmp_func;
	D3DSTENCILOP stencil_op;
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;

	// 开启双面模板
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_TWOSIDEDSTENCILMODE, TRUE);

	IDirect3DDevice9_SetRenderState(context->device, D3DRS_ZENABLE, dss->is_depth_enable ? TRUE : FALSE);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_ZWRITEENABLE, dss->is_depth_write_mask ? TRUE : FALSE);
	cmp_func = d3d9_compare_func_get(dss->depth_func);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_ZFUNC, cmp_func);

	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILENABLE, dss->is_stencil_enable ? TRUE : FALSE);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILMASK, dss->stencil_read_mask);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILWRITEMASK, dss->stencil_write_mask);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILREF, dss->stencil_ref);

	stencil_op = d3d9_stencil_op_get(dss->front_stencil_fail);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILFAIL, stencil_op);

	stencil_op = d3d9_stencil_op_get(dss->front_stencil_depth_fail);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILZFAIL, stencil_op);

	stencil_op = d3d9_stencil_op_get(dss->front_stencil_pass);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILPASS, stencil_op);

	cmp_func = d3d9_compare_func_get(dss->front_stencil_func);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_CCW_STENCILFUNC, cmp_func);

	stencil_op = d3d9_stencil_op_get(dss->back_stencil_fail);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILFAIL, stencil_op);

	stencil_op = d3d9_stencil_op_get(dss->back_stencil_depth_fail);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILZFAIL, stencil_op);

	stencil_op = d3d9_stencil_op_get(dss->back_stencil_pass);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILPASS, stencil_op);

	cmp_func = d3d9_compare_func_get(dss->back_stencil_func);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_STENCILFUNC, cmp_func);
}

static void force_def_bs(BlendState *bs)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;

	D3DBLENDOP blend_op;
	D3DBLEND blend_factor;
	uint color_write_mask;

	// 开启color和alpha的blend分离
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

	if (bs->is_blend_enable)
	{
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_ALPHABLENDENABLE, TRUE);
	}
	else
	{
		IDirect3DDevice9_SetRenderState(context->device, D3DRS_ALPHABLENDENABLE, FALSE);
	}

	blend_op = d3d9_blend_op_get(bs->blend_op);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_BLENDOP, blend_op);

	blend_op = d3d9_blend_op_get(bs->blend_op_alpha);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_BLENDOPALPHA, blend_op);

	blend_factor = d3d9_blend_factor_get(bs->src_blend);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_SRCBLEND, blend_factor);

	blend_factor = d3d9_blend_factor_get(bs->dest_blend);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_DESTBLEND, blend_factor);

	blend_factor = d3d9_blend_factor_get(bs->src_blend_alpha);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_SRCBLENDALPHA, blend_factor);

	blend_factor = d3d9_blend_factor_get(bs->dest_blend_alpha);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_DESTBLENDALPHA, blend_factor);

	color_write_mask = d3d9_color_write_mask_get(bs->color_write_mask);
	IDirect3DDevice9_SetRenderState(context->device, D3DRS_COLORWRITEENABLE, color_write_mask);
}

extern "C" PiBool PI_API render_force_def_state(void)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *impl = &d3d9_system->state;

	pi_renderstate_set_default_rasterizer(&impl->rs);
	pi_renderstate_set_default_depthstencil(&impl->dss);
	pi_renderstate_set_default_blend(&impl->bs);

	pi_renderstate_set_default_rasterizer(&g_rsystem->rs);
	pi_renderstate_set_default_depthstencil(&g_rsystem->dss);
	pi_renderstate_set_default_blend(&g_rsystem->bs);

	force_def_rs(&impl->rs);
	force_def_dss(&impl->rs, &impl->dss);
	force_def_bs(&impl->bs);

	pi_renderutil_init_fullstate(impl->def_state);
	return TRUE;
}

extern "C" PiBool PI_API render_state_set_list(StateList *lst)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *impl = &d3d9_system->state;

	/* 需要拷贝一份状态列表，因为底层需要修改last_state_list */
	pi_memcpy_inline(&impl->cache_state_list, &impl->last_state_list, sizeof(StateList));
	pi_renderutil_state_order_set(impl->def_state, &impl->cache_state_list, lst);
	return TRUE;
}

extern "C" PiBool PI_API render_state_set(RenderStateType key, uint32 value)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *impl = &d3d9_system->state;

	if (impl->def_state[key] == value)
	{
		pi_renderutil_state_remove(&impl->last_state_list, key);
	}
	else
	{
		pi_renderutil_state_add(&impl->last_state_list, key, value);
	}
	switch (key)
	{
	case RST_POLYGON_MODE:
		rs_set_polygon_mode((PolygonMode)value);
		break;
	case RST_CULL_MODE:
		rs_set_cull_mode((CullMode)value);
		break;
	case RST_IS_FRONT_FACE_CCW:
		rs_set_frontface((PiBool)value);
		break;
	case RST_POLYGON_OFFSET:
		rs_set_polygon_offset((float)((int8)(value & 0xff)), ((float)(value >> 8)) / 65535);
		break;
	case RST_IS_DEPTH_CLIP_ENABLE:
		rs_set_depthclip_enable((PiBool)value);
		break;
	case RST_IS_SCISSOR_ENABLE:
		rs_set_scissor_enable((PiBool)value);
		break;
	case RST_IS_MULTISAMPLE_ENABLE:
		rs_set_multisample_enable((PiBool)value);
		break;
	case RST_SHADING_MODE:
		rs_set_shade_mode((ShadeMode)value);
		break;
	case RST_IS_DEPTH_ENABLE:
		dss_set_depth_enable((PiBool)value);
		break;
	case RST_IS_DEPTH_WRITE_MASK:
		dss_set_depthwrite_enable((PiBool)value);
		break;
	case RST_DEPTH_FUNC:
		dss_set_depth_compfunc((CompareFunction)value);
		break;
	case RST_IS_STENCIL_ENABLE:
		dss_set_stencil_enable((PiBool)value);
		break;
	case RST_STENCIL:
		dss_set_stencil((uint8)(value & 0xff),
		                (uint8)((value >> 8) & 0xff),
		                (uint8)((value >> 16) & 0xff));
		break;

	case RST_FRONT_STENCIL_OP:
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		RasterizerState *rs = &g_rsystem->rs;
		DepthStencilState *dss = &g_rsystem->dss;

		StencilOperation fail_op = (StencilOperation)(value & 0xff);
		StencilOperation depth_fail_op = (StencilOperation)((value >> 8) & 0xff);
		StencilOperation stencil_pass_op = (StencilOperation)((value >> 16) & 0xff);
		CompareFunction func = (CompareFunction)((value >> 24) & 0xff);
		if (rs->is_front_face_ccw)
		{
			dss_set_ccw_stencil_op(dss, context, fail_op, depth_fail_op, stencil_pass_op, func);
		}
		else
		{
			dss_set_cw_stencil_op(dss, context, fail_op, depth_fail_op, stencil_pass_op, func);
		}
		break;
	}
	case RST_BACK_STENCIL_OP:
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		RasterizerState *rs = &g_rsystem->rs;
		DepthStencilState *dss = &g_rsystem->dss;

		StencilOperation fail_op = (StencilOperation)(value & 0xff);
		StencilOperation depth_fail_op = (StencilOperation)((value >> 8) & 0xff);
		StencilOperation stencil_pass_op = (StencilOperation)((value >> 16) & 0xff);
		CompareFunction func = (CompareFunction)((value >> 24) & 0xff);
		if (rs->is_front_face_ccw)
		{
			dss_set_cw_stencil_op(dss, context, fail_op, depth_fail_op, stencil_pass_op, func);
		}
		else
		{
			dss_set_ccw_stencil_op(dss, context, fail_op, depth_fail_op, stencil_pass_op, func);
		}
		break;
	}
	case RST_IS_ALPHA_TO_COVERAGE_ENABLE:
		bs_set_a2c_enable((PiBool)value);
		break;
	case RST_IS_INDEPENDENT_BLEND_ENABLE:
		bs_set_independent_blend_enable((PiBool)value);
		break;
	case RST_IS_BLEND_ENABLE:
		bs_set_blend((PiBool)value);
		break;
	case RST_BLEND_OP:
		bs_set_blend_op((BlendOperation)(value & 0xffff), (BlendOperation)(value >> 16));
		break;
	case RST_BLEND_FACTOR:
		bs_set_blend_factor((BlendFactor)(value & 0xff),
		                    (BlendFactor)((value >> 8) & 0xff),
		                    (BlendFactor)((value >> 16) & 0xff),
		                    (BlendFactor)((value >> 24) & 0xff));
		break;
	case RST_COLOR_WRITE_MASK:
		bs_set_color_mask((uint8)value);
		break;
	default:
		break;
	}
	return TRUE;
}

extern "C" uint32 PI_API render_state_get(RenderStateType key)
{
	uint32 r = 0;
	BlendState *bs = &g_rsystem->bs;
	RasterizerState *rs = &g_rsystem->rs;
	DepthStencilState *dss = &g_rsystem->dss;

	switch (key)
	{
	case RST_POLYGON_MODE:
		r = rs->polygon_mode;
		break;
	case RST_CULL_MODE:
		r = rs->cull_mode;
		break;
	case RST_IS_FRONT_FACE_CCW:
		r = rs->is_front_face_ccw;
		break;
	case RST_POLYGON_OFFSET:
		r = (int8)rs->polygon_offset_units | (((uint)(rs->polygon_offset_factor * 65535)) << 8);
		break;
	case RST_IS_DEPTH_CLIP_ENABLE:
		r = rs->is_depth_clip_enable;
		break;
	case RST_IS_SCISSOR_ENABLE:
		r = rs->is_scissor_enable;
		break;
	case RST_IS_MULTISAMPLE_ENABLE:
		r = rs->is_multisample_enable;
		break;
	case RST_IS_DEPTH_ENABLE:
		r = dss->is_depth_enable;
		break;
	case RST_IS_DEPTH_WRITE_MASK:
		r = dss->is_depth_write_mask;
		break;
	case RST_DEPTH_FUNC:
		r = dss->depth_func;
		break;
	case RST_IS_STENCIL_ENABLE:
		r = dss->is_stencil_enable;
		break;
	case RST_STENCIL:
		r = dss->stencil_read_mask | (dss->stencil_write_mask << 8) | (dss->stencil_ref << 16);
		break;
	case RST_FRONT_STENCIL_OP:
		r = dss->front_stencil_fail | (dss->front_stencil_depth_fail << 8) | (dss->front_stencil_pass << 16) | (dss->front_stencil_func << 24);
		break;
	case RST_BACK_STENCIL_OP:
		r = dss->back_stencil_fail | (dss->back_stencil_depth_fail << 8) | (dss->back_stencil_pass << 16) | (dss->back_stencil_func << 24);
		break;
	case RST_IS_ALPHA_TO_COVERAGE_ENABLE:
		r = bs->is_alpha_to_coverage_enable;
		break;
	case RST_IS_INDEPENDENT_BLEND_ENABLE:
		r = bs->is_independent_blend_enable;
		break;
	case RST_IS_BLEND_ENABLE:
		r = bs->is_blend_enable;
		break;
	case RST_BLEND_OP:
		r = bs->blend_op_alpha | (bs->blend_op << 16);
		break;
	case RST_BLEND_FACTOR:
		r = bs->src_blend | (bs->dest_blend << 8) | (bs->src_blend_alpha << 16) | (bs->dest_blend_alpha << 24);
		break;
	case RST_COLOR_WRITE_MASK:
		r = bs->color_write_mask;
		break;
	default:
		break;
	}
	return r;
}

void d3d9_state_set_viewport(uint32 left, uint32 bottom, uint32 width, uint32 height)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *impl = &d3d9_system->state;
	if (impl->left != left || impl->bottom != bottom || impl->width != width || impl->height != height)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;
		D3DVIEWPORT9 vp;
		RECT rect;

		vp.X = left;
		vp.Y = bottom;
		vp.Width = width;
		vp.Height = height;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		IDirect3DDevice9_SetViewport(context->device, &vp);

		rect.left = left;
		rect.top = bottom;
		rect.right = left + width;
		rect.bottom = bottom + height;
		IDirect3DDevice9_SetScissorRect(context->device, &rect);

		impl->left = left;
		impl->bottom = bottom;
		impl->width = width;
		impl->height = height;
	}
}

static void _switch_sampler_state(SamplerState *old_ss, SamplerState *curr_ss, DWORD index)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;

	if (old_ss->addr_mode_u != curr_ss->addr_mode_u)
	{
		D3DTEXTUREADDRESS addr = d3d9_tex_addr_get(curr_ss->addr_mode_u);
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_ADDRESSU, addr);
		old_ss->addr_mode_u = curr_ss->addr_mode_u;
	}
	if (old_ss->addr_mode_v != curr_ss->addr_mode_v)
	{
		D3DTEXTUREADDRESS addr = d3d9_tex_addr_get(curr_ss->addr_mode_v);
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_ADDRESSV, addr);
		old_ss->addr_mode_v = curr_ss->addr_mode_v;
	}
	if (old_ss->addr_mode_w != curr_ss->addr_mode_w)
	{
		D3DTEXTUREADDRESS addr = d3d9_tex_addr_get(curr_ss->addr_mode_w);
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_ADDRESSW, addr);
		old_ss->addr_mode_w = curr_ss->addr_mode_w;
	}

	if ((old_ss->addr_mode_u == TAM_BORDER || old_ss->addr_mode_v == TAM_BORDER || old_ss->addr_mode_w == TAM_BORDER) && pi_memcmp_inline(&old_ss->border_clr, &curr_ss->border_clr, sizeof(PiColor)))
	{
		D3DCOLOR color = D3DCOLOR_COLORVALUE(curr_ss->border_clr.rgba[0], curr_ss->border_clr.rgba[1], curr_ss->border_clr.rgba[2], curr_ss->border_clr.rgba[3]);
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_BORDERCOLOR, color);
		old_ss->border_clr = curr_ss->border_clr;
	}

	if (old_ss->filter != curr_ss->filter)
	{
		D3DTEXTUREFILTERTYPE mag_filter;
		D3DTEXTUREFILTERTYPE min_filter;
		D3DTEXTUREFILTERTYPE mip_filter;
		d3d9_tex_filter_get(curr_ss->filter, &mag_filter, &min_filter, &mip_filter);

		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MAGFILTER, mag_filter);
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MINFILTER, min_filter);
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MIPFILTER, mip_filter);
		old_ss->filter = curr_ss->filter;
	}

	if (old_ss->mip_map_lod_bias != curr_ss->mip_map_lod_bias)
	{
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MIPMAPLODBIAS, *(DWORD *)&curr_ss->mip_map_lod_bias);
		old_ss->mip_map_lod_bias = curr_ss->mip_map_lod_bias;
	}

	if (old_ss->max_anisotropy != curr_ss->max_anisotropy)
	{
		IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MAXANISOTROPY, curr_ss->max_anisotropy);
		old_ss->max_anisotropy = curr_ss->max_anisotropy;
	}
}

static void _set_uniform(Uniform *u, void *value, uint32 count, PiBool is_packed)
{
	
}

void d3d9_state_set_uniform(GpuProgram *program, Uniform *u, Uniform *material_unifom)
{
	void *value = material_unifom->value;
	uint32 count = material_unifom->count;
	PiBool is_packed = material_unifom->is_packed;
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	D3D9Uniform *impl = (D3D9Uniform *)u->impl;
	switch (u->type)
	{
	case UT_SAMPLER_2D:
	{
		SamplerState *ss = (SamplerState *)value;
		if (impl->vs_register)
		{
			SamplerState *old_ss = d3d9_system->state.vs_unit_slot + impl->vs_register_index;
			if (old_ss->tex != ss->tex)
			{
				D3D9Texture *tex_impl = (D3D9Texture *)ss->tex->impl;
				IDirect3DDevice9_SetTexture(context->device, D3DVERTEXTEXTURESAMPLER0 + impl->vs_register_index, tex_impl->handle.texture_2d);
				old_ss->tex = ss->tex;
			}
			_switch_sampler_state(old_ss, ss, D3DVERTEXTEXTURESAMPLER0 + impl->vs_register_index);
		}
		if (impl->ps_register)
		{
			SamplerState *old_ss = d3d9_system->state.ps_unit_slot + impl->fs_register_index;
			if (old_ss->tex != ss->tex)
			{
				D3D9Texture *tex_impl = (D3D9Texture *)ss->tex->impl;
				IDirect3DDevice9_SetTexture(context->device, impl->fs_register_index, tex_impl->handle.texture_2d);
				old_ss->tex = ss->tex;
			}
			_switch_sampler_state(d3d9_system->state.ps_unit_slot + impl->fs_register_index, ss, impl->fs_register_index);
		}
	}
	break;
	case UT_SAMPLER_3D:
	{
		SamplerState *ss = (SamplerState *)value;
		if (impl->vs_register)
		{
			SamplerState *old_ss = d3d9_system->state.vs_unit_slot + impl->vs_register_index;
			if (old_ss->tex != ss->tex)
			{
				D3D9Texture *tex_impl = (D3D9Texture *)ss->tex->impl;
				IDirect3DDevice9_SetTexture(context->device, D3DVERTEXTEXTURESAMPLER0 + impl->vs_register_index, tex_impl->handle.texture_3d);
				old_ss->tex = ss->tex;
			}
			_switch_sampler_state(old_ss, ss, D3DVERTEXTEXTURESAMPLER0 + impl->vs_register_index);
		}
		if (impl->ps_register)
		{
			SamplerState *old_ss = d3d9_system->state.ps_unit_slot + impl->fs_register_index;
			if (old_ss->tex != ss->tex)
			{
				D3D9Texture *tex_impl = (D3D9Texture *)ss->tex->impl;
				IDirect3DDevice9_SetTexture(context->device, impl->fs_register_index, tex_impl->handle.texture_3d);
				old_ss->tex = ss->tex;
			}
			_switch_sampler_state(d3d9_system->state.ps_unit_slot + impl->fs_register_index, ss, impl->fs_register_index);
		}
	}
	break;
	case UT_SAMPLER_CUBE:
	{
		SamplerState *ss = (SamplerState *)value;
		if (impl->vs_register)
		{
			SamplerState *old_ss = d3d9_system->state.vs_unit_slot + impl->vs_register_index;
			if (old_ss->tex != ss->tex)
			{
				D3D9Texture *tex_impl = (D3D9Texture *)ss->tex->impl;
				IDirect3DDevice9_SetTexture(context->device, D3DVERTEXTEXTURESAMPLER0 + impl->vs_register_index, tex_impl->handle.texture_cube);
				old_ss->tex = ss->tex;
			}
			_switch_sampler_state(old_ss, ss, D3DVERTEXTEXTURESAMPLER0 + impl->vs_register_index);
		}
		if (impl->ps_register)
		{
			SamplerState *old_ss = d3d9_system->state.ps_unit_slot + impl->fs_register_index;
			if (old_ss->tex != ss->tex)
			{
				D3D9Texture *tex_impl = (D3D9Texture *)ss->tex->impl;
				IDirect3DDevice9_SetTexture(context->device, impl->fs_register_index, tex_impl->handle.texture_cube);
				old_ss->tex = ss->tex;
			}
			_switch_sampler_state(d3d9_system->state.ps_unit_slot + impl->fs_register_index, ss, impl->fs_register_index);
		}
	}
	break;

	case UT_MATRIX3:
	{

		if (impl->vs_register)
		{

			if (impl->vs_register_count != 3 || !is_packed)
			{
				uint vector_count = count * impl->vs_register_count;
				float *vector = (float *)pi_malloc0(vector_count * 4 * sizeof(float));
				uint dstSize = 4 * sizeof(float);
				uint srcSize = 3 * sizeof(float);
				uint i, j;
				for (i = 0; i < count; i++)
				{
					for (j = 0; j < impl->vs_register_count; j++)
					{
						pi_memcpy_inline(((byte*)vector) + (i * impl->vs_register_count + j) * dstSize, ((byte*)value) + (i * 3 + j) * srcSize, srcSize);
					}
				}
				if (impl->vs_register_set== RS_FLOAT)
				{
					IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float*)vector, impl->vs_register_count * count);
				}
				else
				{
					IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int*)vector, impl->vs_register_count * count);
				}
				pi_free(vector);
			}
			else
			{
				if (impl->vs_register_set == RS_FLOAT)
				{
					IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float*)value, 3 * count);
				}
				else
				{
					IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int*)value, 3 * count);
				}
			}
		}
		if (impl->ps_register)
		{
			if (impl->fs_register_count != 3 || !is_packed)
			{
				uint vector_count = count * impl->fs_register_count;
				float *vector = (float *)pi_malloc0(vector_count * 4 * sizeof(float));
				uint dstSize = 4 * sizeof(float);
				uint srcSize = 3 * sizeof(float);
				uint i, j;
				for (i = 0; i < count; i++)
				{
					for (j = 0; j < impl->fs_register_count; j++)
					{
						pi_memcpy_inline(((byte*)vector) + (i * impl->fs_register_count + j) * dstSize, ((byte*)value) + (i * 3 + j) * srcSize, srcSize);
					}
				}
				if (impl->fs_register_set == RS_FLOAT)
				{
					IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float*)vector, impl->fs_register_count * count);
				}
				else
				{
					IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int*)vector, impl->fs_register_count * count);
				}
				pi_free(vector);
			}
			else
			{
				if (impl->fs_register_set == RS_FLOAT)
				{
					IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float*)value, 3 * count);
				}
				else
				{
					IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int*)value, 3 * count);
				}
			}
		}
	}
	break;
	case UT_MATRIX4:
	{
		if (impl->vs_register)
		{
			if (impl->vs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float *)value, min(4 * count, impl->vs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int *)value, min(4 * count, impl->vs_register_count));
			}
		}
		if (impl->ps_register)
		{
			if (impl->fs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float *)value, min(4 * count, impl->fs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int *)value, min(4 * count, impl->fs_register_count));
			}
		}
	}
	break;
	case UT_MATRIX4x3:
		if (impl->vs_register)
		{
			if (impl->vs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float*)value, min(3 * count, impl->vs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int*)value, min(3 * count, impl->vs_register_count));
			}
		}
		if (impl->ps_register)
		{
			if (impl->fs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float*)value, min(3 * count, impl->fs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int*)value, min(3 * count, impl->fs_register_count));
			}
		}
		break;
	case UT_FLOAT:
	case UT_VEC2:
	case UT_VEC3:
	{
		float *vector;
		if (is_packed)
		{
			vector = (float*)value;
		}
		else{
			vector = (float *)pi_malloc0(count * 4 * sizeof(float));
			pi_uniform_value_pack(u->type, (byte*)vector, (byte*)value, count);
		}
		if (impl->vs_register)
		{
			if (impl->vs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, vector, min(count, impl->vs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int*)vector, min(count, impl->vs_register_count));
			}
		}
		if (impl->ps_register)
		{
			if (impl->fs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, vector, min(count, impl->fs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int*)vector, min(count, impl->fs_register_count));
			}
		}
		if (!is_packed){
			pi_free(vector);
		}
	}
	break;
	case UT_IVEC4:
	case UT_VEC4:
		if (impl->vs_register)
		{
			if (impl->vs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float *)value, min(count, impl->vs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int *)value, min(count, impl->vs_register_count));
			}
		}
		if (impl->ps_register)
		{
			if (impl->fs_register_set == RS_FLOAT)
			{
				IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float *)value, min(count, impl->fs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int *)value, min(count, impl->fs_register_count));
			}
		}
		break;
	case UT_INT:
	case UT_IVEC2:
	case UT_IVEC3:
	{
		int *vector;
		if (is_packed)
		{
			vector = (int*)value;
		}
		else{
			vector = (int *)pi_malloc0(count * 4 * sizeof(int));
			pi_uniform_value_pack(u->type, (byte*)vector, (byte*)value, count);
		}
		if (impl->vs_register)
		{
			if (impl->vs_register_set == RS_INT)
			{
				IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, vector, min(count, impl->vs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float*)vector, min(count, impl->vs_register_count));
			}
		}
		if (impl->ps_register)
		{
			if (impl->fs_register_set == RS_INT)
			{
				IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, vector, min(count, impl->fs_register_count));
			}
			else
			{
				IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float*)vector, min(count, impl->fs_register_count));
			}
		}
		if (!is_packed){
			pi_free(vector);
		}
	}
	break;
	
		//结构体默认都是打包数据
	case UT_STRUCT:
		if (impl->vs_register)
		{
			IDirect3DDevice9_SetVertexShaderConstantF(context->device, impl->vs_register_index, (float *)value, impl->vs_register_count);
		}
		if (impl->ps_register)
		{
			IDirect3DDevice9_SetPixelShaderConstantF(context->device, impl->fs_register_index, (float *)value, impl->fs_register_count);
		}
		break;
		//结构体默认都是打包数据
	case UT_ISTRUCT:
		if (impl->vs_register)
		{
			IDirect3DDevice9_SetVertexShaderConstantI(context->device, impl->vs_register_index, (int *)value, impl->vs_register_count);
		}
		if (impl->ps_register)
		{
			IDirect3DDevice9_SetPixelShaderConstantI(context->device, impl->fs_register_index, (int *)value, impl->fs_register_count);
		}
		break;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", u->type);
		break;
	}

}

void d3d9_state_set_default_uniform(uint index, void* value, uint32 count, PiBool is_vs, PiBool is_ps)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	if (is_vs)
	{
		IDirect3DDevice9_SetVertexShaderConstantF(context->device, index, (float*)value, count);
	}
	if (is_ps)
	{
		IDirect3DDevice9_SetPixelShaderConstantF(context->device, index, (float*)value, count);
	}
}

PiBool d3d9_state_use_shader(GpuProgram *program)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;

	D3D9Program *d3d9_program = (D3D9Program *)program->impl;
	D3D9Shader *vs = d3d9_program->shaders[ST_VS];
	D3D9Shader *ps = d3d9_program->shaders[ST_PS];

	if (d3d9_system->state.vertex_shader != vs->handle.vertex_shader)
	{
		IDirect3DDevice9_SetVertexShader(context->device, vs->handle.vertex_shader);
		d3d9_system->state.vertex_shader = vs->handle.vertex_shader;
	}

	if (d3d9_system->state.pixel_shader != ps->handle.pixel_shader)
	{
		IDirect3DDevice9_SetPixelShader(context->device, ps->handle.pixel_shader);
		d3d9_system->state.pixel_shader = ps->handle.pixel_shader;
	}

	return TRUE;
}

PiBool d3d9_state_remove_texture(PiTexture *tex)
{
	return TRUE;
}

void _set_sampler_state(SamplerState *curr_ss, DWORD index)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;

	D3DTEXTUREADDRESS addr = d3d9_tex_addr_get(curr_ss->addr_mode_u);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_ADDRESSU, addr);

	addr = d3d9_tex_addr_get(curr_ss->addr_mode_v);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_ADDRESSV, addr);

	addr = d3d9_tex_addr_get(curr_ss->addr_mode_w);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_ADDRESSW, addr);

	D3DCOLOR color = D3DCOLOR_COLORVALUE(curr_ss->border_clr.rgba[0], curr_ss->border_clr.rgba[1], curr_ss->border_clr.rgba[2], curr_ss->border_clr.rgba[3]);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_BORDERCOLOR, color);

	D3DTEXTUREFILTERTYPE mag_filter;
	D3DTEXTUREFILTERTYPE min_filter;
	D3DTEXTUREFILTERTYPE mip_filter;
	d3d9_tex_filter_get(curr_ss->filter, &mag_filter, &min_filter, &mip_filter);

	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MAGFILTER, mag_filter);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MINFILTER, min_filter);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MIPFILTER, mip_filter);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MIPMAPLODBIAS, *(DWORD *)&curr_ss->mip_map_lod_bias);
	IDirect3DDevice9_SetSamplerState(context->device, index, D3DSAMP_MAXANISOTROPY, curr_ss->max_anisotropy);
}

void d3d9_state_force_sampler_state(void)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;

	for (uint i = 0; i < MAX_VERTEX_SAMPLER; i++)
	{
		SamplerState *curr_ss = d3d9_system->state.vs_unit_slot + i;
		pi_renderstate_set_default_sampler(curr_ss);
		_set_sampler_state(curr_ss, i + D3DVERTEXTEXTURESAMPLER0);
	}

	for (uint i = 0; i < MAX_PIXEL_SAMPLER; i++)
	{
		SamplerState *curr_ss = d3d9_system->state.ps_unit_slot + i;
		pi_renderstate_set_default_sampler(curr_ss);
		_set_sampler_state(curr_ss, i);
	}
}

void d3d9_state_before_reset(void)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_texture_vector);
	for (uint i = 0; i < len; ++i)
	{
		PiTexture *tex = (PiTexture *)pi_vector_get(&state->default_texture_vector, i);
		d3d9_texture_clear(tex);
	}

	len = pi_vector_size(&state->view_vector);
	for (uint i = 0; i < len; ++i)
	{
		PiRenderView *view = (PiRenderView *)pi_vector_get(&state->view_vector, i);
		d3d9_view_clear(view);
	}

	len = pi_vector_size(&state->default_ib_vector);
	for (uint i = 0; i < len; ++i)
	{
		D3D9RenderLayout *ib = (D3D9RenderLayout *)pi_vector_get(&state->default_ib_vector, i);
		d3d9_release_ib(ib);
	}

	len = pi_vector_size(&state->default_vb_vector);
	for (uint i = 0; i < len; ++i)
	{
		D3D9VertexElement *vb = (D3D9VertexElement *)pi_vector_get(&state->default_vb_vector, i);
		d3d9_release_vb(vb);
	}

	pi_rendertarget_detach(&g_rsystem->main_target, ATT_COLOR0);
	pi_rendertarget_detach(&g_rsystem->main_target, ATT_DEPTHSTENCIL);

	d3d9_free_main_view(state->back_buffer_view);
	d3d9_free_main_view(state->back_depth_stencil_view);
	state->back_buffer_view = NULL;
	state->back_depth_stencil_view = NULL;
}

// 重置之后，所有状态都要回到初始值
static void _reset_state(D3D9RenderState *state)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	uint w = d3d9_system->context->width;
	uint h = d3d9_system->context->height;

	state->curr_ib = NULL;
	state->curr_vertex_declaration = NULL;
	for (int i = 0; i < EVS_NUM; ++i)
	{
		state->curr_stream_source[i] = NULL;
	}

	state->curr_stream_source;
	state->back_buffer_view = d3d9_new_main_view(d3d9_system->context->surface, w, h, RF_ABGR8, RVT_COLOR);
	state->back_depth_stencil_view = d3d9_new_main_view(d3d9_system->context->depth_stencil_surface, w, h, RF_D24S8, RVT_DEPTH_STENCIL);

	pi_rendertarget_attach(&g_rsystem->main_target, ATT_COLOR0, state->back_buffer_view);
	pi_rendertarget_attach(&g_rsystem->main_target, ATT_DEPTHSTENCIL, state->back_depth_stencil_view);
	
	render_force_def_state();
	d3d9_state_force_sampler_state();
}

void d3d9_state_after_reset(void)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_texture_vector);
	for (uint i = 0; i < len; ++i)
	{
		PiTexture *tex = (PiTexture *)pi_vector_get(&state->default_texture_vector, i);
		d3d9_texture_init(tex);
	}

	len = pi_vector_size(&state->default_vb_vector);
	for (uint i = 0; i < len; ++i)
	{
		D3D9VertexElement *vb = (D3D9VertexElement *)pi_vector_get(&state->default_vb_vector, i);
		d3d9_create_vb(vb);
	}

	len = pi_vector_size(&state->default_ib_vector);
	for (uint i = 0; i < len; ++i)
	{
		D3D9RenderLayout *ib = (D3D9RenderLayout *)pi_vector_get(&state->default_ib_vector, i);
		d3d9_create_ib(ib);
	}

	len = pi_vector_size(&state->view_vector);
	for (uint i = 0; i < len; ++i)
	{
		PiRenderView *view = (PiRenderView *)pi_vector_get(&state->view_vector, i);
		d3d9_view_init(view);
	}

	_reset_state(state);
}

void d3d9_state_add_default_texture(PiTexture *tex)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_texture_vector);
	for (uint i = 0; i < len; ++i)
	{
		PiTexture *p = (PiTexture *)pi_vector_get(&state->default_texture_vector, i);
		PI_ASSERT(p != tex, "default texture has the texture");
	}
	pi_vector_push(&state->default_texture_vector, tex);
}

void d3d9_state_remove_default_texture(PiTexture *tex)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_texture_vector);

	sint index = -1;
	for (uint i = 0; i < len; ++i)
	{
		PiTexture *p = (PiTexture *)pi_vector_get(&state->default_texture_vector, i);
		if (p == tex)
		{
			index = i;
			break;
		}
	}
	if (index >= 0 && index < (sint)len)
	{
		pi_vector_remove(&state->default_texture_vector, index);
	}
}

void d3d9_state_add_default_vb(D3D9VertexElement *vb)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_vb_vector);
	for (uint i = 0; i < len; ++i)
	{
		D3D9VertexElement *p = (D3D9VertexElement *)pi_vector_get(&state->default_vb_vector, i);
		PI_ASSERT(p != vb, "default vb has the vb");
	}
	pi_vector_push(&state->default_vb_vector, vb);
}

void d3d9_state_remove_default_vb(D3D9VertexElement *vb)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_vb_vector);

	sint index = -1;
	for (uint i = 0; i < len; ++i)
	{
		D3D9VertexElement *p = (D3D9VertexElement *)pi_vector_get(&state->default_vb_vector, i);
		if (p == vb)
		{
			index = i;
			break;
		}
	}
	if (index >= 0 && index < (sint)len)
	{
		pi_vector_remove(&state->default_vb_vector, index);
	}
}

void d3d9_state_add_default_ib(D3D9RenderLayout *ib)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_ib_vector);
	for (uint i = 0; i < len; ++i)
	{
		D3D9RenderLayout *p = (D3D9RenderLayout *)pi_vector_get(&state->default_ib_vector, i);
		PI_ASSERT(p != ib, "default ib has the ib");
	}
	pi_vector_push(&state->default_ib_vector, ib);
}

void d3d9_state_remove_default_ib(D3D9RenderLayout *ib)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->default_ib_vector);

	sint index = -1;
	for (uint i = 0; i < len; ++i)
	{
		D3D9RenderLayout *p = (D3D9RenderLayout *)pi_vector_get(&state->default_ib_vector, i);
		if (p == ib)
		{
			index = i;
			break;
		}
	}
	if (index >= 0 && index < (sint)len)
	{
		pi_vector_remove(&state->default_ib_vector, index);
	}
}

void d3d9_state_add_view(PiRenderView *view)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->view_vector);
	for (uint i = 0; i < len; ++i)
	{
		PiRenderView *p = (PiRenderView *)pi_vector_get(&state->view_vector, i);
		PI_ASSERT(p != view, "default texture has the texture");
	}
	pi_vector_push(&state->view_vector, view);
}

void d3d9_state_remove_view(PiRenderView *view)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9RenderState *state = &d3d9_system->state;

	uint len = pi_vector_size(&state->view_vector);

	sint index = -1;
	for (uint i = 0; i < len; ++i)
	{
		PiRenderView *p = (PiRenderView *)pi_vector_get(&state->view_vector, i);
		if (p == view)
		{
			index = i;
			break;
		}
	}
	if (index >= 0 && index < (sint)len)
	{
		pi_vector_remove(&state->view_vector, index);
	}
}

void d3d9_state_init(D3D9RenderState *state)
{
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;

	uint w = d3d9_system->context->width;
	uint h = d3d9_system->context->height;

	pi_vector_init(&state->default_vb_vector);
	pi_vector_init(&state->default_ib_vector);
	pi_vector_init(&state->default_texture_vector);
	pi_vector_init(&state->view_vector);

	render_force_def_state();
	d3d9_state_force_sampler_state();

	state->back_buffer_view = d3d9_new_main_view(d3d9_system->context->surface, w, h, RF_ABGR8, RVT_COLOR);
	state->back_depth_stencil_view = d3d9_new_main_view(d3d9_system->context->depth_stencil_surface, w, h, RF_D24S8, RVT_DEPTH_STENCIL);
}

void d3d9_state_clear(D3D9RenderState *state)
{
	pi_vector_clear(&state->default_vb_vector, TRUE);
	pi_vector_clear(&state->default_ib_vector, TRUE);
	pi_vector_clear(&state->default_texture_vector, TRUE);
	pi_vector_clear(&state->view_vector, TRUE);

	d3d9_free_main_view(state->back_buffer_view);
	d3d9_free_main_view(state->back_depth_stencil_view);
}