#include <gl_renderstate.h>

#include <gl_rendersystem.h>
#include <gl_shader.h>

#include <rendersystem.h>
#include <gl_convert.h>
#include <gl_texture.h>

#include <gl_interface.h>

#include <renderinfo.h>

extern PiRenderSystem *g_rsystem;

static void _gl_texture_active_unit(TextureCache *cache, sint unit)
{
	if (cache->current_active_unit != unit)
	{
		cache->current_active_unit = unit;
		gl2_ActiveTexture(GL2_TEXTURE0 + unit);
	}
}

static void _gl_texture_bind_texture(TextureCache *cache, sint unit, uint target, uint id)
{
	PI_ASSERT(unit >= 0 && unit < cache->max_num, "unit is invalid = %d", unit);

	pi_renderinfo_add_texture_change_num(1);
	_gl_texture_active_unit(cache, unit);

	if (cache->unit_slot[unit].id != 0 && cache->unit_slot[unit].target != target)
	{
		/* 渲染目标不一样，需要关掉老的目标 */
		gl2_BindTexture(cache->unit_slot[unit].target, 0);
	}

	gl2_BindTexture(target, id);

	cache->unit_slot[unit].id = id;
	cache->unit_slot[unit].target = target;
}

static void rs_set_cull_mode(CullMode cull)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->cull_mode != cull)
	{
		state->cull_mode = cull;
		switch (cull)
		{
		case CM_NO:
			gl2_Disable(GL2_CULL_FACE);
			break;
		case CM_FRONT:
			gl2_Enable(GL2_CULL_FACE);
			gl2_CullFace(GL2_FRONT);
			break;
		case CM_BACK:
			gl2_Enable(GL2_CULL_FACE);
			gl2_CullFace(GL2_BACK);
			break;
		default:
			break;
		}
	}
}

static void rs_set_polygon_mode(PolygonMode mode)
{
	RasterizerState *state = &g_rsystem->rs;
	if (gl_Self_GetInterfaceType() != RIT_GLES)
	{
		if (state->polygon_mode != mode)
		{
			uint val = gl_polygon_get(mode);
			state->polygon_mode = mode;
			gl_PolygonMode(GL2_FRONT_AND_BACK, val);
		}
	}
}

static void rs_set_scissor_enable(PiBool is_enable)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->is_scissor_enable != is_enable)
	{
		state->is_scissor_enable = is_enable;
		if (is_enable)
		{
			gl2_Enable(GL2_SCISSOR_TEST);
		}
		else
		{
			gl2_Disable(GL2_SCISSOR_TEST);
		}
	}
}

static void rs_set_frontface(PiBool is_ccw)
{
	RasterizerState *state = &g_rsystem->rs;
	if (state->is_front_face_ccw != is_ccw)
	{
		state->is_front_face_ccw = is_ccw;
		gl2_FrontFace(is_ccw ? GL2_CCW : GL2_CW);
	}
}

static void rs_set_depthclip_enable(PiBool is_enable)
{
	if (gl_Self_GetInterfaceType() == RIT_OPENGL)
	{
		if (gl_Self_IsDepthClamp())
		{
			RasterizerState *state = &g_rsystem->rs;
			if (state->is_depth_clip_enable != is_enable)
			{
				state->is_depth_clip_enable = is_enable;

				if (is_enable)
				{
					gl2_Disable(GL_DEPTH_CLAMP);
				}
				else
				{
					gl2_Enable(GL_DEPTH_CLAMP);
				}
			}
		}
	}
}

static void rs_set_multisample_enable(PiBool is_enable)
{
	if (gl_Self_GetInterfaceType() == RIT_OPENGL)
	{
		RasterizerState *state = &g_rsystem->rs;
		if (state->is_multisample_enable != is_enable)
		{
			state->is_multisample_enable = is_enable;
			if (is_enable)
			{
				gl2_Enable(GL_MULTISAMPLE);
			}
			else
			{
				gl2_Disable(GL_MULTISAMPLE);
			}
		}
	}
}

static void rs_set_polygon_offset(float factor, float units)
{
	RasterizerState *state = &g_rsystem->rs;
	if ((state->polygon_offset_factor != factor)
		|| (state->polygon_offset_units != units))
	{
		state->polygon_offset_units = units;
		state->polygon_offset_factor = factor;
		gl2_Enable(GL2_POLYGON_OFFSET_FILL);
		gl2_PolygonOffset(factor, units);
	}
	else
	{
		gl2_Disable(GL2_POLYGON_OFFSET_FILL);
	}
}

static void dss_set_depth_enable(PiBool is_enable)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->is_depth_enable != is_enable)
	{
		state->is_depth_enable = is_enable;
		if (is_enable)
		{
			gl2_Enable(GL2_DEPTH_TEST);
		}
		else
		{
			gl2_Disable(GL2_DEPTH_TEST);
		}
	}
}

static void dss_set_depthwrite_enable(PiBool is_enable)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->is_depth_write_mask != is_enable)
	{
		state->is_depth_write_mask = is_enable;
		gl2_DepthMask((unsigned char)is_enable);
	}
}

static void dss_set_depth_compfunc(CompareFunction func)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->depth_func != func)
	{
		uint val = gl_compare_func_get(func);
		state->depth_func = func;
		gl2_DepthFunc(val);
	}
}

static void dss_set_stencil(uint8 read_mask, uint8 write_mask, uint8 ref)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->stencil_write_mask != write_mask)
	{
		gl2_StencilMask(write_mask);
		state->stencil_write_mask = write_mask;
	}

	if (state->stencil_read_mask != read_mask ||
		state->stencil_ref != ref)
	{
		uint val = gl_compare_func_get(state->front_stencil_func);
		gl2_StencilFuncSeparate(GL2_FRONT, val, ref, read_mask);
		val = gl_compare_func_get(state->back_stencil_func);
		gl2_StencilFuncSeparate(GL2_BACK, val, ref, read_mask);
		state->stencil_read_mask = read_mask;
		state->stencil_ref = ref;
	}
}

static void dss_set_front_stencil_op(StencilOperation fail_op,
	StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	DepthStencilState *state = &g_rsystem->dss;
	if ((state->front_stencil_fail != fail_op)
		|| (state->front_stencil_depth_fail != depth_fail_op)
		|| (state->front_stencil_pass != stencil_pass_op))
	{
		uint val_1 = gl_stencil_op_get(fail_op);
		uint val_2 = gl_stencil_op_get(depth_fail_op);
		uint val_3 = gl_stencil_op_get(stencil_pass_op);

		state->front_stencil_fail = fail_op;
		state->front_stencil_depth_fail = depth_fail_op;
		state->front_stencil_pass = stencil_pass_op;

		gl2_StencilOpSeparate(GL2_FRONT, val_1, val_2, val_3);
	}

	if (state->front_stencil_func != func)
	{
		uint val = gl_compare_func_get(func);
		gl2_StencilFuncSeparate(GL2_FRONT, val, state->stencil_ref, state->stencil_read_mask);
		state->front_stencil_func = func;
	}
}

static void dss_set_stencil_enable(PiBool is_enable)
{
	DepthStencilState *state = &g_rsystem->dss;
	if (state->is_stencil_enable != is_enable)
	{
		if (is_enable)
		{
			gl2_Enable(GL2_STENCIL_TEST);
		}
		else
		{
			gl2_Disable(GL2_STENCIL_TEST);
		}
		state->is_stencil_enable = is_enable;
	}
}

static void dss_set_back_stencil_op(StencilOperation fail_op,
	StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	DepthStencilState *state = &g_rsystem->dss;
	if ((state->back_stencil_fail != fail_op)
		|| (state->back_stencil_depth_fail != depth_fail_op)
		|| (state->back_stencil_pass != stencil_pass_op))
	{
		uint val_1 = gl_stencil_op_get(fail_op);
		uint val_2 = gl_stencil_op_get(depth_fail_op);
		uint val_3 = gl_stencil_op_get(stencil_pass_op);

		state->back_stencil_fail = fail_op;
		state->back_stencil_depth_fail = depth_fail_op;
		state->back_stencil_pass = stencil_pass_op;

		gl2_StencilOpSeparate(GL2_BACK, val_1, val_2, val_3);
	}

	if (state->back_stencil_func != func)
	{
		uint val = gl_compare_func_get(func);
		gl2_StencilFuncSeparate(GL2_BACK, val, state->stencil_ref, state->stencil_read_mask);
		state->back_stencil_func = func;
	}
}

static void bs_set_a2c_enable(PiBool is_enable)
{
	BlendState *state = &g_rsystem->bs;
	if (state->is_alpha_to_coverage_enable != is_enable)
	{
		state->is_alpha_to_coverage_enable = is_enable;
		if (is_enable)
		{
			gl2_Enable(GL2_SAMPLE_ALPHA_TO_COVERAGE);
		}
		else
		{
			gl2_Disable(GL2_SAMPLE_ALPHA_TO_COVERAGE);
		}
	}
}

static void bs_set_independent_blend_enable(PiBool is_enable)
{
	BlendState *state = &g_rsystem->bs;
	if (state->is_independent_blend_enable != is_enable)
	{
		state->is_independent_blend_enable = is_enable;
		PI_ASSERT(FALSE, "opengl can't support this feature");
	}
}

static void bs_set_blend(PiBool is_enable)
{
	BlendState *state = &g_rsystem->bs;
	if (state->is_blend_enable != is_enable)
	{
		state->is_blend_enable = is_enable;
		if (is_enable)
		{
			gl2_Enable(GL2_BLEND);
		}
		else
		{
			gl2_Disable(GL2_BLEND);
		}
	}
}

static void bs_set_blend_op(BlendOperation alpha_op, BlendOperation color_op)
{
	BlendState *state = &g_rsystem->bs;
	if (state->blend_op != color_op || state->blend_op_alpha != alpha_op)
	{
		uint val_1 = gl_blend_op_get(color_op);
		uint val_2 = gl_blend_op_get(alpha_op);

		state->blend_op = color_op;
		state->blend_op_alpha = alpha_op;
		gl2_BlendEquationSeparate(val_1, val_2);
	}
}

static void bs_set_blend_factor(
	BlendFactor src_blend, BlendFactor dst_blend,
	BlendFactor src_alpha_blend, BlendFactor dst_alpha_blend)
{
	BlendState *state = &g_rsystem->bs;
	if ((state->src_blend != src_blend)
		|| (state->dest_blend != dst_blend)
		|| (state->src_blend_alpha != src_alpha_blend)
		|| (state->dest_blend_alpha != dst_alpha_blend))
	{
		uint val_1 = gl_blend_factor_get(src_blend);
		uint val_2 = gl_blend_factor_get(dst_blend);
		uint val_3 = gl_blend_factor_get(src_alpha_blend);
		uint val_4 = gl_blend_factor_get(dst_alpha_blend);

		state->src_blend = src_blend;
		state->dest_blend = dst_blend;
		state->src_blend_alpha = src_alpha_blend;
		state->dest_blend_alpha = dst_alpha_blend;

		gl2_BlendFuncSeparate(val_1, val_2, val_3, val_4);
	}
}

static void bs_set_color_mask(uint8 color_mask)
{
	BlendState *state = &g_rsystem->bs;
	if (state->color_write_mask != color_mask)
	{
		state->color_write_mask = color_mask;
		gl2_ColorMask((color_mask & CMASK_RED) != 0,
			(color_mask & CMASK_GREEN) != 0,
			(color_mask & CMASK_BLUE) != 0,
			(color_mask & CMASK_ALPHA) != 0);
	}
}

static void force_def_other(GLRenderState *state)
{
	float *c = state->clear_color.rgba;
	color_set(&state->clear_color, 1.0f, 1.0f, 1.0f, 1.0f);
	gl2_ClearColor(c[0], c[1], c[2], c[3]);

	state->clear_depth = 1.0f;

	gl2_ClearDepthf(state->clear_depth);

	state->clear_stencil = 0;
	gl2_ClearStencil(state->clear_stencil);
}

static void force_def_rs(RasterizerState *rs)
{
	uint val = 0;

	if (gl_Self_GetInterfaceType() != RIT_GLES)
	{
		val = gl_polygon_get(rs->polygon_mode);
		gl_PolygonMode(GL2_FRONT_AND_BACK, val);
	}


	gl2_FrontFace(rs->is_front_face_ccw ? GL2_CCW : GL2_CW);

	switch (rs->cull_mode)
	{
	case CM_NO:
		gl2_Disable(GL2_CULL_FACE);
		break;
	case CM_FRONT:
		gl2_Enable(GL2_CULL_FACE);
		gl2_CullFace(GL2_FRONT);
		break;
	case CM_BACK:
		gl2_Enable(GL2_CULL_FACE);
		gl2_CullFace(GL2_BACK);
		break;
	default:
		break;
	}

	gl2_PolygonOffset(rs->polygon_offset_factor, rs->polygon_offset_units);

	if (gl_Self_IsDepthClamp())
	{
		if (rs->is_depth_clip_enable)
		{
			gl2_Disable(GL_DEPTH_CLAMP);
		}
		else
		{
			gl2_Enable(GL_DEPTH_CLAMP);
		}
	}

	if (rs->is_scissor_enable)
	{
		gl2_Enable(GL2_SCISSOR_TEST);
	}
	else
	{
		gl2_Disable(GL2_SCISSOR_TEST);
	}

	if (gl_Self_GetInterfaceType() == RIT_OPENGL)
	{
		if (rs->is_multisample_enable)
		{
			gl2_Enable(GL_MULTISAMPLE);
		}
		else
		{
			gl2_Disable(GL_MULTISAMPLE);
		}
	}
}

static void force_def_dss(DepthStencilState *dss)
{
	uint val, val_1, val_2, val_3;

	if (dss->is_depth_enable)
	{
		gl2_Enable(GL2_DEPTH_TEST);
	}
	else
	{
		gl2_Disable(GL2_DEPTH_TEST);
	}
	gl2_DepthMask(dss->is_depth_write_mask ? TRUE : FALSE);
	val = gl_compare_func_get(dss->depth_func);
	gl2_DepthFunc(val);

	gl2_StencilMask(dss->stencil_write_mask);

	val = gl_compare_func_get(dss->front_stencil_func);
	gl2_StencilFuncSeparate(GL2_FRONT, val, dss->stencil_ref, dss->stencil_read_mask);

	val_1 = gl_stencil_op_get(dss->front_stencil_fail);
	val_2 = gl_stencil_op_get(dss->front_stencil_depth_fail);
	val_3 = gl_stencil_op_get(dss->front_stencil_pass);
	gl2_StencilOpSeparate(GL2_FRONT, val_1, val_2, val_3);

	val = gl_compare_func_get(dss->back_stencil_func);
	gl2_StencilFuncSeparate(GL2_BACK, val, dss->stencil_ref, dss->stencil_read_mask);

	val_1 = gl_stencil_op_get(dss->back_stencil_fail);
	val_2 = gl_stencil_op_get(dss->back_stencil_depth_fail);
	val_3 = gl_stencil_op_get(dss->back_stencil_pass);
	gl2_StencilOpSeparate(GL2_BACK, val_1, val_2, val_3);

	if (dss->is_stencil_enable)
	{
		gl2_Enable(GL2_STENCIL_TEST);
	}
	else
	{
		gl2_Disable(GL2_STENCIL_TEST);
	}
}

static void force_def_bs(BlendState *bs)
{
	uint val_1, val_2, val_3, val_4;
	if (bs->is_alpha_to_coverage_enable)
	{
		gl2_Enable(GL2_SAMPLE_ALPHA_TO_COVERAGE);
	}
	else
	{
		gl2_Disable(GL2_SAMPLE_ALPHA_TO_COVERAGE);
	}

	if (bs->is_blend_enable)
	{
		gl2_Enable(GL2_BLEND);
	}
	else
	{
		gl2_Disable(GL2_BLEND);
	}

	val_1 = gl_blend_op_get(bs->blend_op);
	val_2 = gl_blend_op_get(bs->blend_op_alpha);
	gl2_BlendEquationSeparate(val_1, val_2);

	val_1 = gl_blend_factor_get(bs->src_blend);
	val_2 = gl_blend_factor_get(bs->dest_blend);
	val_3 = gl_blend_factor_get(bs->src_blend_alpha);
	val_4 = gl_blend_factor_get(bs->dest_blend_alpha);
	gl2_BlendFuncSeparate(val_1, val_2, val_3, val_4);

	gl2_ColorMask((bs->color_write_mask & CMASK_RED) != 0,
		(bs->color_write_mask & CMASK_GREEN) != 0,
		(bs->color_write_mask & CMASK_BLUE) != 0,
		(bs->color_write_mask & CMASK_ALPHA) != 0);

	gl2_BlendColor(1, 1, 1, 1);
}

PiBool PI_API render_force_def_state(void)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;

	pi_renderstate_set_default_rasterizer(&impl->rs);
	pi_renderstate_set_default_blend(&impl->bs);
	pi_renderstate_set_default_depthstencil(&impl->dss);

	force_def_bs(&impl->bs);
	force_def_rs(&impl->rs);
	force_def_dss(&impl->dss);
	force_def_other(impl);
	pi_renderutil_init_fullstate(impl->def_state);
	return TRUE;
}

PiBool PI_API render_state_set_list(StateList *lst)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;

	/* 需要拷贝一份状态列表，因为底层需要修改last_state_list */
	pi_memcpy_inline(&impl->cache_state_list, &impl->last_state_list, sizeof(StateList));
	pi_renderutil_state_order_set(impl->def_state, &impl->cache_state_list, lst);
	return TRUE;
}

PiBool PI_API render_state_set(RenderStateType key, uint32 value)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;

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
	case RST_CULL_MODE:
		rs_set_cull_mode((CullMode)value);
		break;
	case RST_POLYGON_MODE:
		rs_set_polygon_mode((PolygonMode)value);
		break;
	case RST_IS_SCISSOR_ENABLE:
		rs_set_scissor_enable((PiBool)value);
		break;
	case RST_IS_FRONT_FACE_CCW:
		rs_set_frontface((PiBool)value);
		break;
	case RST_IS_DEPTH_CLIP_ENABLE:
		rs_set_depthclip_enable((PiBool)value);
		break;
	case RST_IS_MULTISAMPLE_ENABLE:
		rs_set_multisample_enable((PiBool)value);
		break;
	case RST_POLYGON_OFFSET:
		rs_set_polygon_offset((float)((int8)(value & 0xff)), ((float)(value >> 8)) / 65535);
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
		dss_set_front_stencil_op((StencilOperation)(value & 0xff),
			(StencilOperation)((value >> 8) & 0xff),
			(StencilOperation)((value >> 16) & 0xff),
			(CompareFunction)((value >> 24) & 0xff));
		break;
	case RST_BACK_STENCIL_OP:
		dss_set_back_stencil_op((StencilOperation)(value & 0xff),
			(StencilOperation)((value >> 8) & 0xff),
			(StencilOperation)((value >> 16) & 0xff),
			(CompareFunction)((value >> 24) & 0xff));
		break;
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
uint32 PI_API render_state_get(RenderStateType key)
{
	uint32 r = 0;
	BlendState *bs = &g_rsystem->bs;
	RasterizerState *rs = &g_rsystem->rs;
	DepthStencilState *dss = &g_rsystem->dss;

	switch (key)
	{
	case RST_CULL_MODE:
		r = rs->cull_mode;
		break;
	case RST_POLYGON_MODE:
		r = rs->polygon_mode;
		break;
	case RST_IS_SCISSOR_ENABLE:
		r = rs->is_scissor_enable;
		break;
	case RST_IS_FRONT_FACE_CCW:
		r = rs->is_front_face_ccw;
		break;
	case RST_IS_DEPTH_CLIP_ENABLE:
		r = rs->is_depth_clip_enable;
		break;
	case RST_IS_MULTISAMPLE_ENABLE:
		r = rs->is_multisample_enable;
		break;
	case RST_POLYGON_OFFSET:
		r = (int8)rs->polygon_offset_units | (((uint)(rs->polygon_offset_factor * 65535)) << 8);
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

// TODO
PiBool glstate_delete_rendertarget(PiRenderTarget *rs)
{
	PI_USE_PARAM(rs);
	return FALSE;
}

void glstate_set_viewport(uint left, uint bottom, uint width, uint height)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;
	if (impl->left != left || impl->bottom != bottom || impl->width != width || impl->height != height)
	{
		impl->left = left;
		impl->bottom = bottom;
		impl->width = width;
		impl->height = height;
		gl2_Scissor(left, bottom, width, height);
		gl2_Viewport(left, bottom, width, height);
	}
}

void glstate_bind_texture(PiTexture *tex)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;
	texturecache_insert(&impl->tex_cache, tex);
}

PiBool glstate_remove_texture(PiTexture *tex)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;
	texturecache_remove(&impl->tex_cache, tex);
	return TRUE;
}

uint32 glstate_bind_fbo(uint fbo)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;
	uint32 old = impl->fbo;
	if (old != fbo)
	{
		impl->fbo = fbo;
		pi_renderinfo_add_target_change_num(1);
		gl2_BindFramebuffer(GL2_FRAMEBUFFER, fbo);
	}
	return old;
}

static void _set_textures(GLRenderState *state, Uniform **curr, uint curr_num)
{
	sint i;

	for (i = 0; i < (sint)curr_num; ++i)
	{
		GLUniform *gl_u = curr[i]->impl;
		SamplerState *ss = curr[i]->value;

		if (ss->tex != NULL)
		{
			GLTexture *gl_tex = ss->tex->impl;
			texturecache_insert(&state->tex_cache, ss->tex);
			if (gl_u->sampler_id != gl_tex->bind_unit)
			{
				gl_u->sampler_id = gl_tex->bind_unit;
				gl2_Uniform1i(gl_u->location, gl_tex->bind_unit);
			}
		}
		else
		{
			pi_log_print(LOG_INFO, "warnning, uniform's texture is null, name = %s", curr[i]->name);
		}
	}
}

PiBool glstate_enable_fbo_srgb(PiBool is_srgb_enable)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;
	if (impl->is_srgb_enable != is_srgb_enable)
	{
		impl->is_srgb_enable = is_srgb_enable;
		if (gl_Self_IsFramebufferSRGB())
		{
			if (is_srgb_enable)
			{
				gl2_Enable(GL_FRAMEBUFFER_SRGB);
			}
			else
			{
				gl2_Disable(GL_FRAMEBUFFER_SRGB);
			}
		}
	}
	return TRUE;
}

PiBool glstate_use_shader(GpuProgram *program)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	GLRenderState *impl = &gl_rs->state;
	GLProgram *gl_program = program->impl;

	if (impl->program != program)
	{
		impl->program = program;
		pi_renderinfo_add_gpuprogram_change_num(1);
		gl2_UseProgram(gl_program->id);
	}

	/* 设置采样器和纹理 */
	_set_textures(impl, gl_program->samplers, gl_program->sampler_num);

	/* 设置program用到的uniform到opengl */
	gl_program_set_gluniforms(program);

	return TRUE;
}

PiBool glstate_init(GLRenderState *state)
{
	sint i;

	if (gl_Self_GetInterfaceType() != RIT_NULL)
	{
		gl2_GetIntegerv(GL2_MAX_VERTEX_ATTRIBS, (sint *)&state->max_attrib_num);
		gl2_GetIntegerv(GL2_MAX_TEXTURE_IMAGE_UNITS, (sint *)&state->max_texture_unit);
	}
	else
	{
		state->max_attrib_num = 16;
		state->max_texture_unit = 16;
	}

	if (state->max_attrib_num > EVS_NUM)
	{
		state->max_attrib_num = EVS_NUM;
	}

	if (state->max_attrib_num < EVS_TEXCOORD_7)
	{
		PI_ASSERT(FALSE, "attribute bind texture failed, attribute num is too slow, num = %d", state->max_attrib_num);
		return FALSE;
	}

	if (state->max_texture_unit > MAX_TEXCOORD_NUM)
	{
		state->max_texture_unit = MAX_TEXCOORD_NUM;
	}
	state->tex_cache.max_num = state->max_texture_unit;
	state->tex_cache.free_slot = pi_new0(uint, state->tex_cache.max_num);
	state->tex_cache.cache = pi_new0(PiTexture *, state->tex_cache.max_num);

	state->tex_cache.free_slot_num = state->max_texture_unit;
	for (i = state->tex_cache.free_slot_num - 1; i >= 0; --i)
	{
		state->tex_cache.free_slot[i] = state->tex_cache.free_slot_num - i - 1;
	}
	return TRUE;
}

void glstate_clear(GLRenderState *state)
{
	pi_free(state->tex_cache.cache);
	pi_free(state->tex_cache.free_slot);
}

PiBool texturecache_remove(TextureCache *cache, PiTexture *tex)
{
	sint i, j;
	GLTexture *gl_tex = tex->impl;

	if (gl_tex->bind_unit >= 0)
	{
		for (i = 0; i < cache->use_num; ++i)
		{
			if (cache->cache[i] == tex)
			{
				break;
			}
		}

		PI_ASSERT(i < cache->use_num, "texture cache should be exist");

		for (j = i; j < cache->use_num - 1; ++j)
		{
			cache->cache[j] = cache->cache[j + 1];
		}

		/* 放回空闲的纹理单元中 */
		cache->free_slot[cache->free_slot_num++] = gl_tex->bind_unit;
		_gl_texture_bind_texture(cache, gl_tex->bind_unit, gl_tex->gl_target, 0);

		gl_tex->bind_unit = -1;
		--cache->use_num;
	}
	return TRUE;
}

PiBool texturecache_insert(TextureCache *cache, PiTexture *tex)
{
	sint j, i = cache->use_num;
	GLTexture *gl_tex = tex->impl;

	if (gl_tex->bind_unit >= 0)
	{
		for (i = 0; i < cache->use_num; ++i)
		{
			if (cache->cache[i] == tex)
			{
				/* 激活它绑定的纹理单元 */
				_gl_texture_active_unit(cache, gl_tex->bind_unit);
				_gl_texture_bind_texture(cache, gl_tex->bind_unit, gl_tex->gl_target, gl_tex->gl_id);

				/* 找到缓存，将其移到缓存的最前面 */
				for (j = i; j > 0; --j)
				{
					cache->cache[j] = cache->cache[j - 1];
				}
				cache->cache[0] = tex;
				break;
			}
		}
	}

	if (i < cache->use_num)
	{
		PI_ASSERT(cache->unit_slot[gl_tex->bind_unit].target == gl_tex->gl_target && cache->unit_slot[gl_tex->bind_unit].id == gl_tex->gl_id, "texture cache is invalid");
	}
	else
	{
		/* 从缓存里没有找到 */

		if (cache->use_num >= cache->max_num)
		{
			/* 缓存已经满了，删除最后一个*/
			PiTexture *remove_tex = cache->cache[cache->use_num - 1];
			GLTexture *remove_gl_tex = remove_tex->impl;

			/* 放回空闲的纹理单元中 */
			cache->free_slot[cache->free_slot_num++] = remove_gl_tex->bind_unit;

			remove_gl_tex->bind_unit = -1;
			--cache->use_num;
		}

		gl_tex->bind_unit = cache->free_slot[--cache->free_slot_num];
		_gl_texture_bind_texture(cache, gl_tex->bind_unit, gl_tex->gl_target, gl_tex->gl_id);

		/* 将纹理加到缓存的第一个元素去 */
		for (i = cache->use_num; i > 0; --i)
		{
			cache->cache[i] = cache->cache[i - 1];
		}
		cache->cache[0] = tex;
		++cache->use_num;
	}
	return TRUE;
}
