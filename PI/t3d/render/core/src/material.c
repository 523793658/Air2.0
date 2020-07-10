#include <material.h>
#include <rendersystem.h>
#include <texture.h>

/* 无效的uniform索引 */
#define INVALID_UNIFORM_INDEX -1

SamplerState* PI_API pi_sampler_new(void)
{
	SamplerState *ss = pi_new0(SamplerState, 1);
	pi_renderstate_set_default_sampler(ss);
	return ss;
}

void PI_API pi_sampler_free(SamplerState *ss)
{
	if(ss != NULL)
	{
		pi_free(ss);
	}
}

void PI_API pi_sampler_set_texture(SamplerState *ss, PiTexture *tex)
{
	ss->tex = tex;
}

void PI_API pi_sampler_set_addr_mode(SamplerState *ss, TexAddressMode u, TexAddressMode v, TexAddressMode w)
{
	ss->addr_mode_u = u;
	ss->addr_mode_v = v;
	ss->addr_mode_w = w;
}

void PI_API pi_sampler_set_border_color(SamplerState *ss, PiColor *color)
{
	color_copy(&ss->border_clr, color);
}

void PI_API pi_sampler_set_filter(SamplerState *ss, TexFilterOp filter)
{
	ss->filter = filter;
}

void PI_API pi_sampler_set_lod(SamplerState *ss, float min_lod, float max_lod)
{
	ss->min_lod = min_lod;
	ss->max_lod = max_lod;
}

void PI_API pi_sampler_set_compare_func(SamplerState *ss, CompareFunction func)
{
	ss->cmp_func = func;
}

void PI_API pi_sampler_set_mip_map_lod_bias(SamplerState *ss, float bias)
{
	ss->mip_map_lod_bias = bias;
}

void PI_API pi_sampler_set_max_anisotropy(SamplerState *ss, uint8 max_anisotropy)
{
	ss->max_anisotropy = max_anisotropy;
}

PiMaterial* PI_API pi_material_new(const char *vs_key, const char *fs_key)
{
	PiMaterial *material = pi_new0(PiMaterial, 1);
	
	material->vs_key = pi_str_dup(vs_key);
	material->fs_key = pi_str_dup(fs_key);

	return material;
}

void PI_API pi_material_free(PiMaterial *material)
{
	uint i;
	if(material != NULL)
	{
		pi_free(material->vs_key);
		pi_free(material->fs_key);

		if(material->vs != NULL)
		{
			shader_release(material->vs);
		}

		if(material->fs != NULL)
		{
			shader_release(material->fs);
		}

		for(i = 0; i < material->num_defs; ++i)
		{
			pi_free(material->defs[i]);
		}

		for(i = 0; i < material->num_uniforms; ++i)
		{
			if(material->uniforms[i]->is_copy)
			{
				pi_free(material->uniforms[i]->value);
			}
			pi_free(material->uniforms[i]);
		}
		pi_free(material);
	}
}

void PI_API pi_material_set_def(PiMaterial *material, const char *def, PiBool is_enable)
{
	uint i;
	PiBool is_release_shader = FALSE;
	
	for(i = 0; i < material->num_defs; ++i)
	{
		if(material->def_names[i] == def)
		{
			break;
		}
	}

	if(is_enable)
	{
		/* 添加没有的宏 */
		if(i >= material->num_defs)
		{
			uint len = pi_strlen("#define "), size = len + pi_strlen(def) + pi_strlen("\n") + 1;
			char *prefix_def = pi_malloc0(size);
			pi_strcpy(prefix_def, "#define ", len);

			pi_str_cat(prefix_def, size, def);
			pi_str_cat(prefix_def, size, "\n");

			is_release_shader = TRUE;
			material->def_names[material->num_defs] = (char *)def;
			material->defs[material->num_defs++] = pi_str_dup(prefix_def);
			pi_free(prefix_def);
		}
	}
	else
	{
		/* 删除已有的宏 */
		if(i < material->num_defs)
		{
			is_release_shader = TRUE;
			pi_free(material->defs[i]);
			material->defs[i] = material->defs[material->num_defs - 1];
			material->def_names[i] = material->def_names[material->num_defs - 1];

			--material->num_defs;
		}
	}

	if(is_release_shader)
	{
		if(material->vs != NULL)
		{
			shader_release(material->vs);
		}

		if(material->fs != NULL)
		{
			shader_release(material->fs);			
		}
		
		material->vs = NULL;
		material->fs = NULL;
		material->program = NULL;
	}
}
void PI_API pi_material_set_uniform_pack_flag(PiMaterial *material, const char *name, UniformType type, uint32 count, void *data, PiBool is_copy, PiBool isPacked)
{
	uint i;
	uint size;
	size = pi_uniform_value_size(type, count);
	for (i = 0; i < material->num_uniforms; ++i)
	{
		if (material->uniforms[i]->name == name)
		{
			break;
		}
	}

	if (data != NULL)
	{/* 设置uniform的值 */
		Uniform *u = NULL;

		if (i < material->num_uniforms)
		{/* 释放掉原值 */
			u = material->uniforms[i];

			if (!is_copy && !u->is_copy && u->count == count && u->type == type && u->value == data)
			{/* 快速返回 */
				return;
			}

			if (!u->is_copy)
			{
				u->value = NULL;
			}
			else
			{
				PiBool is_not_free = is_copy && (size == u->size);
				if (!is_not_free)
				{
					pi_free(u->value);
					u->size = 0;
					u->value = NULL;
				}
			}
		}
		else
		{/* 创建新值 */
			PI_ASSERT(material->num_uniforms < MAX_UNIFORMS_NUM, "");

			u = pi_new0(Uniform, 1);
			u->name = (char *)name;
			material->uniforms[material->num_uniforms++] = u;

			if (material->program != NULL)
			{/* 找program的uniform索引更新 */
				PiDvector *program_uniforms = program_get_uniforms(material->program, FALSE);
				uint j, num_program_uniforms = pi_dvector_size(program_uniforms);
				for (j = 0; j < num_program_uniforms; ++j)
				{
					Uniform *program_uniform = (Uniform *)pi_dvector_get(program_uniforms, j);
					if (pi_str_equal(u->name, program_uniform->name, FALSE))
					{
						u->reference_uniform = program_uniform;
						material->available_uniforms[material->valid_uniforms_num++] = u;
						break;
					}
				}
			}
		}

		u->type = type;
		u->count = count;
		u->is_copy = is_copy;
		u->size = size;
		u->is_packed = is_copy || isPacked;
		if (!is_copy)
		{
			u->value = data;
		}
		else
		{
			if (u->value == NULL)
			{
				u->value = pi_malloc(size);
			}
			pi_uniform_value_pack(type, u->value, data, count);
		}
	}
	else
	{
		/* 释放掉老的值 */
		if (i < material->num_uniforms)
		{
			Uniform *u = material->uniforms[i];
			int j;
			if (u->is_copy)
			{
				pi_free(u->value);
			}
			material->uniforms[i] = material->uniforms[--material->num_uniforms];

			for (j = material->valid_uniforms_num - 1; j >= 0; --j)
			{
				if (material->available_uniforms[j] == u)
				{
					material->available_uniforms[j] = material->available_uniforms[--material->valid_uniforms_num];
				}
			}

			pi_free(u);
		}
	}
}

void PI_API pi_material_set_uniform(PiMaterial *material, const char *name, UniformType type, uint32 count, void *data, PiBool is_copy)
{
	pi_material_set_uniform_pack_flag(material, name, type, count, data, is_copy, FALSE);
}

Uniform *PI_API pi_material_get_uniform(PiMaterial *material, const char *name)
{
	uint i;

	for (i = 0; i < material->num_uniforms; ++i)
	{
		if (material->uniforms[i]->name == name)
		{
			return material->uniforms[i];
		}
	}

	return NULL;
}

void PI_API pi_material_set_cull_mode(PiMaterial *material, CullMode cull)
{
	pi_renderutil_state_add(&material->state, RST_CULL_MODE, cull);
}

void PI_API pi_material_set_polygon_mode(PiMaterial *material, PolygonMode mode)
{
	pi_renderutil_state_add(&material->state, RST_POLYGON_MODE, mode);
}

void PI_API pi_material_set_scissor_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_SCISSOR_ENABLE, is_enable);
}

void PI_API pi_material_set_frontface(PiMaterial *material, PiBool is_ccw)
{
	pi_renderutil_state_add(&material->state, RST_IS_FRONT_FACE_CCW, is_ccw);
}

void PI_API pi_material_set_depthclip_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_DEPTH_CLIP_ENABLE, is_enable);
}

void PI_API pi_material_set_multisample_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_MULTISAMPLE_ENABLE, is_enable);
}

void PI_API pi_material_set_polygon_offset(PiMaterial *material, float factor, float units)
{
	uint value = renderutil_get_polygon_offset(factor, units);
	pi_renderutil_state_add(&material->state, RST_POLYGON_OFFSET, value);
}

void PI_API pi_material_set_depth_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_DEPTH_ENABLE, is_enable);
}

void PI_API pi_material_set_depthwrite_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_DEPTH_WRITE_MASK, is_enable);
}

void PI_API pi_material_set_depth_compfunc(PiMaterial *material, CompareFunction func)
{
	pi_renderutil_state_add(&material->state, RST_DEPTH_FUNC, func);
}

void PI_API pi_material_set_stencil_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_STENCIL_ENABLE, is_enable);
}

void PI_API pi_material_set_stencil(PiMaterial *material, uint8 read_mask, uint8 write_mask, uint8 ref)
{
	uint value = renderutil_get_stencil(read_mask, write_mask, ref);
	pi_renderutil_state_add(&material->state, RST_STENCIL, value);
}

void PI_API pi_material_set_front_stencil_op(PiMaterial *material, StencilOperation fail_op, 
	StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	uint value = renderutil_get_stencil_op(fail_op, depth_fail_op, stencil_pass_op, func);
	pi_renderutil_state_add(&material->state, RST_FRONT_STENCIL_OP, value);
}

void PI_API pi_material_set_back_stencil_op(PiMaterial *material, StencilOperation fail_op, 
	StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	uint value = renderutil_get_stencil_op(fail_op, depth_fail_op, stencil_pass_op, func);
	pi_renderutil_state_add(&material->state, RST_BACK_STENCIL_OP, value);
}

void PI_API pi_material_set_a2c_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_ALPHA_TO_COVERAGE_ENABLE, is_enable);
}

void PI_API pi_material_set_independent_blend_enable(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_INDEPENDENT_BLEND_ENABLE, is_enable);
}

void PI_API pi_material_set_blend(PiMaterial *material, PiBool is_enable)
{
	pi_renderutil_state_add(&material->state, RST_IS_BLEND_ENABLE, is_enable);
}

void PI_API pi_material_set_blend_op(PiMaterial *material, BlendOperation alpha_op, BlendOperation color_op)
{
	uint value = renderutil_get_blend_op(alpha_op, color_op);
	pi_renderutil_state_add(&material->state, RST_BLEND_OP, value);
}

void PI_API pi_material_set_blend_func(PiMaterial *material, BlendMode mode)
{
	pi_material_set_blend(material, mode != BM_NONE);
	switch (mode)
	{
	case BM_NONE:
		pi_material_set_blend_factor(material, BF_ONE, BF_ONE, BF_ONE, BF_ONE);
		break;
	case BM_ALPHA:
		pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
		break;
	case BM_ADDITIVE:
		pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_ONE, BF_ZERO, BF_ONE);
		break;
	case BM_MODULATE:
		pi_material_set_blend_factor(material, BF_DST_ALPHA, BF_ZERO, BF_DST_COLOR, BF_ZERO);
		break;
	case BM_ALPHA_R:
		pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ONE, BF_ZERO);
	default:
		break;
	}
}

void PI_API pi_material_set_alpha_cull_off(PiMaterial *material, float cull_off)
{
	material->cull_off = cull_off;
}

void PI_API pi_material_set_blend_factor(PiMaterial *material, 
	BlendFactor src_blend, BlendFactor dst_blend,
	BlendFactor src_alpha_blend, BlendFactor dst_alpha_blend)
{
	uint value = renderutil_get_blend_factor(src_blend, dst_blend, src_alpha_blend, dst_alpha_blend);
	pi_renderutil_state_add(&material->state, RST_BLEND_FACTOR, value);	
}

void PI_API pi_material_set_color_mask(PiMaterial *material, uint8 color_mask)
{
	pi_renderutil_state_add(&material->state, RST_COLOR_WRITE_MASK, color_mask);
}

void PI_API pi_material_set_shade_mode(PiMaterial *material, ShadeMode mode)
{
	pi_renderutil_state_add(&material->state, RST_SHADING_MODE, mode);
}

static void _apply_uniforms(void *program, GDefaultVariable *gdv, Uniform **uniforms, uint uniforms_num)
{
	/* 全局 */
	Uniform *u = NULL;
	PiDvector *vec;
	uint i, len;
	
	vec = program_get_uniforms(program, TRUE);
	len = pi_dvector_size(vec);
	for(i = 0; i < len; ++i)
	{
		u = (Uniform *)pi_dvector_get(vec, i);
		renderutil_set_default_variable(program, gdv, u);
	}

	/* material的uniform */
	vec = program_get_uniforms(program, FALSE);

	for (i = 0; i < uniforms_num; ++i)
	{
		u = uniforms[i];
		program_set_uniform(program, u->reference_uniform, u);
	}
}

/* 根据program的uniform的索引更新到uniforms */
static uint _update_uniform_index(void *program, Uniform **uniforms, uint num_uniforms, Uniform **available_uniforms)
{
	PiDvector *program_uniforms = program_get_uniforms(program, FALSE);
	uint i, j, num_program_uniforms = pi_dvector_size(program_uniforms);
	uint num_valid_uniform = 0;

	for (i = 0; i < num_uniforms; ++i)
	{
		for (j = 0; j < num_program_uniforms; ++j)
		{
			Uniform *u = (Uniform *)pi_dvector_get(program_uniforms, j);
			if (pi_str_equal(uniforms[i]->name, u->name, FALSE) &&
				uniforms[i]->type == u->type &&
				uniforms[i]->count <= u->count)
			{
				uniforms[i]->reference_uniform = u;
				available_uniforms[num_valid_uniform++] = uniforms[i];
				break;
			}
		}
		if (j >= num_program_uniforms)
		{
			uniforms[i]->reference_uniform = NULL;
		}
	}
	if (num_program_uniforms != num_valid_uniform)
	{
		pi_log_print(LOG_WARNING, "program's uniform isn't set, name");
	}
	return num_valid_uniform;
}

/* 查看哪个uniform无效，给出警告 */
static void _warn_invalid_index_uniform(PiDvector *program_uniforms, uint num_program_uniforms, Uniform **uniforms, uint num_uniforms)
{
	uint i, j;
	for (i = 0; i < num_program_uniforms; ++i)
	{
		Uniform *u = (Uniform *)pi_dvector_get(program_uniforms, i);
		for (j = 0; j < num_uniforms; ++j)
		{
			if (pi_str_equal(uniforms[j]->name, u->name, FALSE))
			{
				break;
			}
		}

		if (j >= num_uniforms)
		{
			pi_log_print(LOG_WARNING, "program's uniform isn't set, name = %s", u->name);
		}
	}
}

PiBool PI_API pi_material_update(PiMaterial *material)
{
	PiBool r = TRUE;
	PiRenderSystem *sys = pi_rendersystem_get_instance();
	if(material->program == NULL)
	{
		material->vs = shader_get(ST_VS, material->vs_key, material->num_defs, material->def_names);
		material->fs = shader_get(ST_PS, material->fs_key, material->num_defs, material->def_names);
		material->program = program_get(material->vs, material->fs);
		material->valid_uniforms_num = _update_uniform_index(material->program, material->uniforms, material->num_uniforms, material->available_uniforms);
	}

	pi_renderstate_set_list(&material->state);

	{
		PiDvector *us = program_get_uniforms(material->program, FALSE);
		uint len = pi_dvector_size(us);
		if (len != material->valid_uniforms_num)
		{
			r = FALSE;
			
			_warn_invalid_index_uniform(us, len, material->uniforms, material->num_uniforms);
		}
	}
	
	r = r && pi_rendersystem_set_program(material->program);
	
	if (r)
	{
		_apply_uniforms(material->program, &sys->gdv, material->available_uniforms, material->valid_uniforms_num);
		
	}
	return r;
}