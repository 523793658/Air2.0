#include <renderstate.h>
#include <renderwrap.h>

PiBool PI_API pi_renderstate_set_default_rasterizer(RasterizerState *rs)
{
	rs->polygon_mode = PM_FILL;		
	rs->cull_mode = CM_BACK;
	rs->is_front_face_ccw = TRUE;
	rs->polygon_offset_factor = 0;
	rs->polygon_offset_units = 0;
	rs->is_depth_clip_enable = TRUE;
	rs->is_scissor_enable = TRUE;
	rs->is_multisample_enable = FALSE;
	rs->shading_mode = SM_GOURAUD;
	return TRUE;
}

PiBool PI_API pi_renderstate_set_default_blend(BlendState *bs)
{
	bs->is_alpha_to_coverage_enable = FALSE;
	bs->is_independent_blend_enable = FALSE;
	bs->is_blend_enable = FALSE;
	bs->blend_op = BOP_ADD;
	bs->src_blend = BF_ONE;
	bs->dest_blend = BF_ZERO;
	bs->blend_op_alpha = BOP_ADD;
	bs->src_blend_alpha = BF_ONE;
	bs->dest_blend_alpha = BF_ZERO;
	bs->color_write_mask = CMASK_ALL;
	return TRUE;
}

PiBool PI_API pi_renderstate_set_default_depthstencil(DepthStencilState *dss)
{
	dss->is_depth_enable = TRUE;
	dss->is_depth_write_mask = TRUE;
	dss->depth_func = CF_LESS;
	dss->is_stencil_enable = FALSE;
	dss->stencil_read_mask = 0xFF;
	dss->stencil_write_mask = 0xFF;
	dss->stencil_ref = 0;
	dss->front_stencil_fail = SOP_KEEP;
	dss->front_stencil_depth_fail = SOP_KEEP;
	dss->front_stencil_pass = SOP_KEEP;
	dss->front_stencil_func = CF_ALWAYSPASS;
	dss->back_stencil_fail = SOP_KEEP;
	dss->back_stencil_depth_fail = SOP_KEEP;
	dss->back_stencil_pass = SOP_KEEP;
	dss->back_stencil_func = CF_ALWAYSPASS;
	return TRUE;
}

PiBool PI_API pi_renderstate_set_default_sampler(SamplerState *ss)
{
	ss->tex = NULL;
	color_set(&ss->border_clr, 1.0f, 1.0f, 1.0f, 1.0f);
	ss->addr_mode_u = TAM_WRAP;
	ss->addr_mode_v = TAM_WRAP;
	ss->addr_mode_w = TAM_WRAP;
	ss->filter = TFO_MIN_MAG_POINT;
	ss->max_anisotropy = 0;
	ss->min_lod = 0;
	ss->max_lod = MAX_FLOAT;
	ss->mip_map_lod_bias = 0;
	ss->cmp_func = CF_ALWAYSFAIL;
	return TRUE;
}

PiBool PI_API pi_renderstate_set(RenderStateType key, uint32 value)
{
	return render_state_set(key, value);
}

PiBool PI_API pi_renderstate_set_list(StateList *lst)
{
	return render_state_set_list(lst);
}

uint32 PI_API pi_renderstate_get(RenderStateType key)
{
	return render_state_get(key);
}