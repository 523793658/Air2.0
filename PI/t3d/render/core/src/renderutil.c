/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "renderutil.h"
#include "material.h"

/* 默认变量的最大长度 */
#define DEFAULT_VARIABLE_MAX_LEN 20

/* 在着色器上绑定的位置 */
typedef enum
{
	PBT_GLOBAL = 0,
	PBT_FRAME = 1,
	PBT_UNIFORM = 2,
	PBT_PROGRAM = 3,
}ProgramBindType;

/* 设置program上的uniform变量 */
typedef PiBool (*UniformSetFunc ) (void *program, GDefaultVariable *gdv,
	PiDhash *global, PiDvector *last, uint size, uint *index, Uniform *u1);

/*打包Uniform数据*/
void pi_uniform_value_pack(UniformType type, byte* dst, byte* src, uint count)
{
	uint i;
	uint srcSize, dstSize, row;
	switch (type)
	{
	case UT_INT:
		srcSize = 1 * INT_SIZE;
		dstSize = 4 * INT_SIZE;
		row = count;
		break;
	case UT_IVEC2:
		srcSize = 2 * INT_SIZE;
		dstSize = 4 * INT_SIZE;
		row = count;
		break;
	case UT_IVEC3:
		srcSize = 3 * INT_SIZE;
		dstSize = 4 * INT_SIZE;
		row = count;
		break;
	case UT_IVEC4:
		srcSize = 4 * INT_SIZE;
		dstSize = 4 * INT_SIZE;
		row = count;
		break;
	case UT_FLOAT:
		srcSize = 1 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = count;
		break;
	case UT_VEC2:
		srcSize = 2 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = count;
		break;
	case UT_VEC3:
		srcSize = 3 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = count;
		break;
	case UT_VEC4:
		srcSize = 4 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = count;
		break;
	case UT_MATRIX2:
		srcSize = 2 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = 2 * count;
		break;
	case UT_MATRIX3:
		srcSize = 3 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = 3 * count;
		break;
	case UT_MATRIX4:
		srcSize = 4 * sizeof(float);
		dstSize = 4 * sizeof(float);
		row = 4 * count;
		break;
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_SAMPLER_2D_SHADOW:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		dstSize = srcSize = sizeof(SamplerState);
		row = count;
		break;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", type);
		return;
	}
	for (i = 0; i < row; i++)
	{
		pi_memcpy_inline(dst + dstSize * i, src + srcSize * i, srcSize);
	}
}


/* 获取uniform值的字节大小 */
uint pi_uniform_value_size(UniformType type, uint32 array_num)
{
	uint r = 0;
	switch(type)
	{
	case UT_INT:
	case UT_IVEC2:
	case UT_IVEC3:
	case UT_IVEC4:
		r = 4 * INT_SIZE;
		break;
	case UT_FLOAT:
	case UT_VEC2:
	case UT_VEC3:
	case UT_VEC4:
		r = 4 * sizeof(float);
		break;
	case UT_MATRIX2:
		r = 4 * 2 * sizeof(float);
		break;
	case UT_MATRIX3:
		r = 4 * 3 * sizeof(float);
		break;
	case UT_MATRIX4:
		r = 4 * 4 * sizeof(float);
		break;
	case UT_MATRIX4x3:
		r = 4 * 3 * sizeof(float);
		break;
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_SAMPLER_2D_SHADOW:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		return sizeof(SamplerState);
	case UT_ISTRUCT:
	case UT_STRUCT:
		return 0;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", type);
		return 0;
		break;
	}
	return r * array_num;
}

/* 查找上次是否设置过该变量，如果设置过则还原为global的值，否则重用 */
static void _uniform_global_last_set(void *program, GDefaultVariable *gdv,
	PiDhash *global, PiDvector *last, uint size, uint *index, Uniform *u)
{
	PiCompR comp;
	Uniform *last_u, *g_u;
	
	if(pi_str_start_with(u->name, "g_"))
	{
		renderutil_set_default_variable(program, gdv, u);
		return;
	}

	/* 在global一定要有u->name */
	if(global == NULL || !pi_dhash_lookup(global, u, &g_u))
	{
		pi_log_print(LOG_WARNING, "global haven't the uniform, name = %s", u->name);
		return;
	}
	
	comp = PI_COMP_GREAT;
	while(*index < size && comp == PI_COMP_GREAT)
	{
		*index++;
		last_u = pi_dvector_get(last, *index);
		comp = pi_str_compare(u->name, last_u->name);
	}
	
	if(comp == PI_COMP_EQUAL)
	{/* 设置了上次材质的uniform，还原成global的 */
		renderutil_program_cc_set(program, u, g_u);
	}
}

/* 从全局中取该变量的值 */
static void _uniform_global_set(void *program, GDefaultVariable *gdv,
	PiDhash *global, PiDvector *last, uint size, uint *index, Uniform *u)
{
	Uniform *tmp;
	PI_USE_PARAM(last);
	PI_USE_PARAM(size);
	PI_USE_PARAM(index);
	
	if(pi_str_start_with(u->name, "g_"))
	{
		renderutil_set_default_variable(program, gdv, u);
		return;
	}
	
	/* 设置global的元素的值 */
	if(global != NULL && pi_dhash_lookup(global, u, &tmp))
	{
		renderutil_program_cc_set(program, u, tmp);
		return;
	}

	pi_log_print(LOG_WARNING, "global haven't the uniform, name = %s", u->name);
}

/* 有序比较并设置program的uniform */
/* func是将global的元素设置到program中 */
static PiBool _program_order_set(void *program, GDefaultVariable *gdv,
	PiDhash *env_us, PiDvector *last_mat_us, PiDvector *cur_mat_us, UniformSetFunc func)
{
	PiBool r = TRUE;
	PiCompR comp;
	Uniform *dst_u = NULL, *cur_u = NULL;
	PiDvector *dst_us = program_get_uniforms(program, FALSE);
	
	uint index_last = 0, n_last = 0;
	uint index_dst = 0, n_dst = pi_dvector_size(dst_us);
	uint index_cur = 0, n_cur = pi_dvector_size(cur_mat_us);

	if(last_mat_us)
	{
		n_last = pi_dvector_size(last_mat_us);
	}

	if(n_dst > 0)
	{
		dst_u = (Uniform *)pi_dvector_get(dst_us, 0);
	}
	
	if(n_cur > 0)
	{
		cur_u = (Uniform *)pi_dvector_get(cur_mat_us, 0);
	}
	
	while (index_dst < n_dst && index_cur < n_cur)
	{
		comp = pi_str_compare(dst_u->name, cur_u->name);
		
		if(comp == PI_COMP_LESS)
		{/* dst的元素小于cur，所以在env中取对应的uniform */
			r = func(program, gdv, env_us, last_mat_us, n_last, &index_last, dst_u);
			if(!r)
			{
				pi_log_print(LOG_WARNING, "set uniform failed, name = %s", dst_u->name);
				break;
			}
			
			if(++index_dst < n_dst)
			{
				dst_u = (Uniform *)pi_dvector_get(dst_us, index_dst);
			}
		}
		else if(comp == PI_COMP_GREAT)
		{
			if(++index_cur < n_cur)
			{
				cur_u = (Uniform *)pi_dvector_get(cur_mat_us, index_cur);
			}
		}
		else
		{
			/* 键相同，直接设置dst_u */
			r = renderutil_program_cc_set(program, dst_u, cur_u);
			if(!r)
			{
				pi_log_print(LOG_WARNING, "set uniform failed, name = %s", dst_u->name);
				break;
			}

			if(++index_dst < n_dst)
			{
				dst_u = (Uniform *)pi_dvector_get(dst_us, index_dst);
			}
			if(++index_cur < n_cur)
			{
				cur_u = (Uniform *)pi_dvector_get(cur_mat_us, index_cur);
			}		
		}
	}
	
	if(r)
	{
		for(; index_dst < n_dst; ++index_dst)
		{
			dst_u = pi_dvector_get(dst_us, index_dst);
			/* 从default, last和global中找参数 */
			r = func(program, gdv, env_us, last_mat_us, n_last, &index_last, dst_u);
			if(!r)
			{
				pi_log_print(LOG_WARNING, "set uniform failed, name = %s", dst_u->name);
				break;
			}
		}
	}
	return r;
}

void renderutil_set_shadow_data(GDefaultVariable *gdv, PiTexture* map)
{
	pi_sampler_set_texture(&gdv->g_ShadowData.texture, map);
}

PiBool PI_API pi_renderutil_init_fullstate(uint32 *state)
{
	uint val;
	BlendState bs;
	RasterizerState rs;
	DepthStencilState dss;

	pi_renderstate_set_default_rasterizer(&rs);
	state[RST_POLYGON_MODE] = rs.polygon_mode;
	state[RST_CULL_MODE] = rs.cull_mode;

	state[RST_IS_FRONT_FACE_CCW] = rs.is_front_face_ccw;

	val = renderutil_get_polygon_offset(rs.polygon_offset_factor, rs.polygon_offset_units);
	state[RST_POLYGON_OFFSET] = val;

	state[RST_IS_DEPTH_CLIP_ENABLE] = rs.is_depth_clip_enable;

	state[RST_IS_SCISSOR_ENABLE] = rs.is_scissor_enable; 
	state[RST_IS_MULTISAMPLE_ENABLE] = rs.is_multisample_enable;

	pi_renderstate_set_default_depthstencil(&dss);
	state[RST_IS_DEPTH_ENABLE] = dss.is_depth_enable;
	state[RST_IS_DEPTH_WRITE_MASK] = dss.is_depth_write_mask;
	state[RST_DEPTH_FUNC] = dss.depth_func;

	state[RST_IS_STENCIL_ENABLE] = dss.is_stencil_enable;

	val = renderutil_get_stencil(
		dss.stencil_read_mask, dss.stencil_write_mask, dss.stencil_ref);
	state[RST_STENCIL] = val;

	val = renderutil_get_stencil_op(
		dss.front_stencil_fail, dss.front_stencil_depth_fail, dss.front_stencil_pass, dss.front_stencil_func);
	state[RST_FRONT_STENCIL_OP] = val;

	val = renderutil_get_stencil_op(
		dss.back_stencil_fail, dss.back_stencil_depth_fail, dss.back_stencil_pass, dss.back_stencil_func);
	state[RST_BACK_STENCIL_OP] = val;

	pi_renderstate_set_default_blend(&bs);
	state[RST_IS_ALPHA_TO_COVERAGE_ENABLE] = bs.is_alpha_to_coverage_enable;
	state[RST_IS_INDEPENDENT_BLEND_ENABLE] = bs.is_independent_blend_enable;

	state[RST_IS_BLEND_ENABLE] = bs.is_blend_enable;

	val = renderutil_get_blend_op(bs.blend_op_alpha, bs.blend_op);
	state[RST_BLEND_OP] = val;

	val = renderutil_get_blend_factor(
		bs.src_blend, bs.dest_blend, bs.src_blend_alpha, bs.dest_blend_alpha);
	state[RST_BLEND_FACTOR] = val;

	state[RST_COLOR_WRITE_MASK] = bs.color_write_mask;
	return TRUE;
}

void renderutil_set_default_variable(void *program, GDefaultVariable *gdv, Uniform *u)
{
	void *value = NULL;
		
	switch(u->d_type)
	{
	case DUT_WORLD_MATRIX:
		value = gdv->g_world->m;
		break;
	case DUT_VIEW_MATRIX:
		value = gdv->g_view->m;
		break;
	case DUT_PROJ_MATRIX:
		value = gdv->g_proj->m;
		break;
	case DUT_WORLD_VIEW_MATRIX:
		if(!(gdv->wvp_mask & WVP_WORLD_VIEW)) 
		{
			gdv->wvp_mask |= WVP_WORLD_VIEW;
			pi_mat4_mul(&gdv->g_world_view, gdv->g_view, gdv->g_world);
		}
		value = gdv->g_world_view.m;
		break;
	case DUT_VIEW_PROJ_MATRIX:
		if(!(gdv->wvp_mask & WVP_VIEW_PROJ)) 
		{
			gdv->wvp_mask |= WVP_VIEW_PROJ;
			pi_mat4_mul(&gdv->g_view_proj, gdv->g_proj, gdv->g_view);
		}
		value = gdv->g_view_proj.m;
		break;
	case DUT_WORLD_VIEW_PROJ_MATRIX:
		if(!(gdv->wvp_mask & WVP_WORLD_VIEW_PROJ)) 
		{
			gdv->wvp_mask |= WVP_WORLD_VIEW_PROJ;
			if(!(gdv->wvp_mask & WVP_VIEW_PROJ)) 
			{
				gdv->wvp_mask |= WVP_VIEW_PROJ;
				pi_mat4_mul(&gdv->g_view_proj, gdv->g_proj, gdv->g_view);
			}
			pi_mat4_mul(&gdv->g_world_view_proj, &gdv->g_view_proj, gdv->g_world);
		}
		value = gdv->g_world_view_proj.m;
		break;
	case DUT_NORMAL_MATRIX:
		if(!(gdv->wvp_mask & WVP_NORMAL)) 
		{
			gdv->wvp_mask |= WVP_NORMAL;
			pi_mat4_transpose(&gdv->g_normal, gdv->g_world);
			pi_mat4_inverse(&gdv->g_normal, &gdv->g_normal);
		}
		value = gdv->g_normal.m;
		break;
	case DUT_VIEW_NORMAL_MATRIX:
		if(!(gdv->wvp_mask & WVP_VIEW_NORMAL)) 
		{
			gdv->wvp_mask |= WVP_VIEW_NORMAL;
			pi_mat4_mul(&gdv->g_view_normal, gdv->g_view, gdv->g_world);
			pi_mat4_transpose(&gdv->g_view_normal, &gdv->g_view_normal);
			pi_mat4_inverse(&gdv->g_view_normal, &gdv->g_view_normal);
		}
		value = gdv->g_view_normal.m;		
		break;
	case DUT_ALPHA_CULLOFF:
		value = &gdv->g_alpha_cull_off;
		break;
	case DUT_TIME:
		value = &gdv->g_Time;
		break;
	case DUT_SHADOW_DATA:
		value = &gdv->g_ShadowData.params;
		break;
	case DUT_SHADOW_MAP:
		value = &gdv->g_ShadowData.texture;
		break;
	case DUT_VIEW_POSITION:
		value = &gdv->g_viewPosition;
		break;
	case DUT_ENVIRONMENT:
		return;
	case DUT_VIEWPORT_SIZE:
		PI_ASSERT(FALSE, "invalid type");
		break;
	default:
		PI_ASSERT(FALSE, "invalid type");
		break;
	}
	u->value = value;
	u->count = 1;
	u->is_packed = TRUE;
	program_set_uniform(program, u, u);
}

/* 二分插入 */
PiBool PI_API pi_renderutil_state_add(StateList *list, RenderStateType key, uint32 value)
{
	uint low = 0;
	uint count = list->len;
	while(count > 0)
	{
		uint count2 = count / 2;
		uint mid = low + count2;
		
		if(list->key[mid] == key)
		{/* 找到了，赋值并返回 */
			list->value[mid] = value;
			return TRUE;
		}
		else if (key > list->key[mid])
		{/* 找到的大于中间元素，在后半段 */
			low = ++mid;
			count -= (count2 + 1);
		}
		else
		{/* 找到的小于中间元素，在前半段 */
			count = count2;
		}
	}
		
	/* 元素组中没有找到，需要插入的情况 */
	if(low < list->len)
	{
		count = (sint)list->len;
		pi_memmove_inline(list->value + low + 1, list->value + low, (count - low) * sizeof(uint32));
		pi_memmove_inline(list->key + low + 1, list->key + low, (count - low) * sizeof(RenderStateType));
	}
	++list->len;
	list->key[low] = key;
	list->value[low] = value;	
	return TRUE;
}

uint renderutil_get_polygon_offset(float factor, float units)
{
	return (int8)factor | (((uint)(units * 65535)) << 8);
}

uint renderutil_get_stencil(uint8 write_mask, uint8 read_mask, uint8 ref)
{
	return write_mask | (read_mask << 8) | (ref << 16);
}

uint renderutil_get_stencil_op(StencilOperation fail_op, StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func)
{
	return fail_op | (depth_fail_op << 8) | (stencil_pass_op << 16) | (func << 24);
}

uint renderutil_get_blend_op(BlendOperation alpha_op, BlendOperation color_op)
{
	return alpha_op | (color_op << 16);
}

uint renderutil_get_blend_factor(BlendFactor src_blend, BlendFactor dst_blend, BlendFactor src_alpha_blend, BlendFactor dst_alpha_blend)
{
	return src_blend | (dst_blend << 8) | (src_alpha_blend << 16) | (dst_alpha_blend << 24);
}

/* 二分移除 */
PiBool PI_API pi_renderutil_state_remove(StateList *list, RenderStateType key)
{
	uint low = 0;
	uint count = (sint)list->len;
	while(count > 0)
	{
		uint count2 = count / 2;
		uint mid = low + count2;
		
		if (key > list->key[mid])
		{/* 找到的大于中间元素，在后半段 */
			low = ++mid;
			count -= (count2 + 1);
		}
		else if (key < list->key[mid])
		{/* 找到的小于中间元素，在前半段 */
			count = count2;
		}
		else
		{/* 找到了要删除的元素 */
			
			--list->len;
			count = list->len;
			if(mid < count)
			{
				pi_memmove_inline(list->key + mid, 
					list->key + mid + 1, (count - mid) * sizeof(RenderStateType));
				
				pi_memmove_inline(list->value + mid, 
					list->value + mid + 1, (count - mid) * sizeof(uint32));
			}
			return TRUE;
		}
	}
	return TRUE;
}

/* 根据last和cur，设置状态 */
PiBool PI_API pi_renderutil_state_order_set(const uint32 *state, const StateList *last, const StateList *cur)
{
	RenderStateType k;
	uint i = 0, j = 0;
	uint n1 = last->len, n2 = cur->len;

	while(i < n1 && j < n2)
	{
		k = last->key[i];
		if(k < cur->key[j])
		{
			/* 注：如果last有而cur没有的状态，要还原回state值 */
			pi_renderstate_set(k, state[k]);
			++i;
		}
		else if(k > cur->key[j])
		{
			pi_renderstate_set(cur->key[j], cur->value[j]);
			++j;	
		}
		else
		{
			/* 不做值比较，依赖底层的缓冲来判断 */
			pi_renderstate_set(cur->key[j], cur->value[j]);
			++i;
			++j;
		}
	}

	if(i < n1)
	{
		for(; i < n1; ++i)
		{/* 注：如果last有而cur没有的状态，要还原回state值 */
			k = last->key[i];
			pi_renderstate_set(k, state[k]);
		}
	}
	else if(j < n2)
	{
		for(; j < n2; ++j)
		{
			pi_renderstate_set(cur->key[j], cur->value[j]);
		}
	}
	return TRUE;
}

// 如果某个状态last和cur都有，cur优先
// StateList中的key已经排好序
void renderutil_merge_order_set(StateList *dest, const StateList *last, const StateList *cur)
{
	uint i = 0, j = 0, k = 0;
	uint n1 = last->len, n2 = cur->len;
	uint n = n1 + n2;

	while(i < n1 && j < n2)
	{
		if(last->key[i] < cur->key[j])
		{
			dest->key[k] = last->key[i];
			dest->value[k++] = last->value[i++];
		}
		else if(last->key[i] > cur->key[j])
		{
			dest->key[k] = cur->key[j];
			dest->value[k++] = cur->value[j++];
		}
		else
		{/* 某个状态cur和last都存在，cur优先 */
			dest->key[k] = cur->key[j];
			dest->value[k++] = cur->value[j++];
			--n;	++i;
		}
	}

	if(i < n1)
	{
		pi_memcpy_inline(dest->key + k, last->key + i, (n1 - i) * sizeof(RenderStateType));
		pi_memcpy_inline(dest->value + k, last->value + i, (n1 - i) * sizeof(uint32));
	}
	else if(j < n2)
	{
		pi_memcpy_inline(dest->key + k, cur->key + j, (n2 - j) * sizeof(RenderStateType));
		pi_memcpy_inline(dest->value + k, cur->value + j, (n2 - j) * sizeof(uint32));
	}
	
	dest->len = n;
}

/* 二分法插入 */
void renderutil_add_uniform(PiDvector *list, Uniform *uniform)
{
	Uniform *u;
	uint low = 0;
	char *name = uniform->name;
	uint count = pi_dvector_size(list);
	
	while(count > 0)
	{
		PiCompR comp;
		uint count2 = count / 2;
		uint mid = low + count2;
		
		u = (Uniform *)pi_dvector_get(list, mid);
		comp = pi_str_compare(name, u->name);
		
		if(comp == PI_COMP_EQUAL)
		{/* 已经找到 */
			renderutil_copy_uniform(u, uniform);
			return;
		}
		else if(comp == PI_COMP_GREAT)
		{/* 找到的大于中间元素，在后半段 */
			low = ++mid;
			count -= (count2 + 1);
		}
		else if(comp == PI_COMP_LESS)
		{/* 找到的小于中间元素，在前半段 */
			count = count2;
		}
	}
	
	/* 没有找到，在low中插入该元素 */
	pi_dvector_insert(list, low, uniform);
	u = pi_dvector_get(list, low);
	u->value = NULL;
	u->name = pi_str_dup(uniform->name);
	renderutil_copy_uniform(u, uniform);
}

/* 二分查找 */
void renderutil_remove_uniform(PiDvector *list, const char *name)
{
	uint low = 0;
	uint count = pi_dvector_size(list);
	while(count > 0)
	{
		uint count2 = count / 2;
		uint mid = low + count2;
		
		Uniform *u = pi_dvector_get(list, mid);
		PiCompR comp = pi_str_compare(name, u->name);
		
		if(comp == PI_COMP_GREAT)
		{/* 找到的大于中间元素，在后半段 */
			low = ++mid;
			count -= (count2 + 1);
		}
		else if(comp == PI_COMP_LESS)
		{/* 找到的小于中间元素，在前半段 */
			count = count2;
		}
		else
		{/* 找到删除的元素 */
			renderutil_free_uniform(NULL, u);
			pi_dvector_remove(list, mid);
			return;
		}
	}
}

PiBool renderutil_clear_uniform(PiDvector *list)
{
	pi_dvector_foreach(list, renderutil_free_uniform, NULL);
	pi_dvector_clear(list, TRUE);
	return TRUE;
}

void renderutil_init_uniform(Uniform *uniform, char* name, UniformType type, void *value, uint32 count)
{
	uniform->name = name;
	uniform->type = type;
	uniform->value = value;
	uniform->count = count;
}

PiSelectR PI_API renderutil_free_uniform(void* user_data, Uniform *uniform)
{
	if(uniform->name != NULL)
		pi_free(uniform->name);
	if(uniform->value != NULL)
		pi_free(uniform->value);
	return SELECT_NEXT;
}

PiBool renderutil_is_valid_uniform(UniformType type, void *value)
{
	PiBool is_valid = TRUE;
	SamplerState *ss = NULL;
	switch(type)
	{
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_2D_SHADOW:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		ss = (SamplerState *)value;
		is_valid = (ss->tex != NULL);
		break;
	default:
		break;
	}
	return is_valid;
}

PiBool renderutil_is_same_uniform(Uniform *dest, const Uniform *src)
{
	uint len;
	PiBool is_same = TRUE;
	UniformType type = dest->type;
	
	if(type != src->type)
		return FALSE;
	if(dest->count != src->count)
		return FALSE;
	
	len = pi_uniform_value_size(type, dest->count);
	switch(type)
	{
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_2D_SHADOW:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		{
			SamplerState *d_ss = dest->value;
			if(d_ss->tex == NULL)
			{
				is_same = FALSE;	
			}
			else 
			{
				SamplerState *last = texture_get_curr_sampler(d_ss->tex);
				is_same = pi_memcmp_inline(last, dest->value, len) == PI_COMP_EQUAL;
			}
			break;
		}
	default:
		break;
	}
	return is_same && (pi_memcmp_inline(dest->value, src->value, len) == PI_COMP_EQUAL);
}

PiBool renderutil_copy_uniform(Uniform *dest, const Uniform *src)
{
	UniformType type = dest->type;
	if(type != src->type || src->value == NULL || src->count == 0)
	{
		PI_ASSERT(FALSE, "src uniform's type or count isn't invalid");
		return FALSE;
	}

	if(dest->value && dest->count < src->count)
	{
		pi_free(dest->value);
		dest->value = NULL;
	}
	
	dest->count = src->count;
	if(dest->value == NULL)
	{
		dest->size = pi_uniform_value_size(type, src->count);
		dest->value = pi_malloc0(dest->size);
	}
	pi_memcpy_inline(dest->value, src->value, src->size);
	
	return TRUE;
}

void get_frustum_points(PiCamera* cam, float z_far, PiVector3 result[8])
{
	uint i;
	PiMatrix4 view_proj_mat_inverse;
	PiVector3 z_point = { 0, 0, -z_far };
	pi_mat4_apply_point(&z_point, &z_point, pi_camera_get_projection_matrix(cam));
	pi_mat4_inverse(&view_proj_mat_inverse, pi_camera_get_view_projection_matrix(cam));
	pi_vec3_set(&result[0], -1, -1, 0);
	pi_vec3_set(&result[1], 1, -1, 0);
	pi_vec3_set(&result[2], 1, 1, 0);
	pi_vec3_set(&result[3], -1, 1, 0);
	pi_vec3_set(&result[4], -1, -1, z_point.z);
	pi_vec3_set(&result[5], 1, -1, z_point.z);
	pi_vec3_set(&result[6], 1, 1, z_point.z);
	pi_vec3_set(&result[7], -1, 1, z_point.z);
	for (i = 0; i < 8; i++) {
		pi_mat4_apply_point(&result[i], &result[i], &view_proj_mat_inverse);
	}
}

void max_vec3(PiVector3* vec0, PiVector3* vec1, PiVector3* result)
{
	result->x = MAX(vec0->x, vec1->x);
	result->y = MAX(vec0->y, vec1->y);
	result->z = MAX(vec0->z, vec1->z);
}

void min_vec3(PiVector3* vec0, PiVector3* vec1, PiVector3* result)
{
	result->x = MIN(vec0->x, vec1->x);
	result->y = MIN(vec0->y, vec1->y);
	result->z = MIN(vec0->z, vec1->z);
}

void PI_API fit_shadow_to_pixel(PiAABBBox* shadow_aabb, float sm_width, float sm_height)
{
	float world_units_pre_pixel_x, world_units_pre_pixel_y;
	world_units_pre_pixel_x = (shadow_aabb->maxPt.x - shadow_aabb->minPt.x) / sm_width;
	world_units_pre_pixel_y = (shadow_aabb->maxPt.y - shadow_aabb->minPt.y) / sm_height;
	shadow_aabb->maxPt.x = pi_math_floor(shadow_aabb->maxPt.x / world_units_pre_pixel_x) * world_units_pre_pixel_x;
	shadow_aabb->minPt.x = pi_math_floor(shadow_aabb->minPt.x / world_units_pre_pixel_x) * world_units_pre_pixel_x;
	shadow_aabb->maxPt.y = pi_math_floor(shadow_aabb->maxPt.y / world_units_pre_pixel_y) * world_units_pre_pixel_y;
	shadow_aabb->minPt.y = pi_math_floor(shadow_aabb->minPt.y / world_units_pre_pixel_y) * world_units_pre_pixel_y;
}

void renderutil_update_shadow_cam(PiCamera* view_cam, PiCamera* shadow_cam, float sm_width, float sm_height, float z_far)
{
	uint i;
	PiAABBBox shadow_aabb;
	PiVector3 points[8];
	PiMatrix4* view_mat;
	PiVector3* location = pi_camera_get_location(view_cam);
	pi_camera_set_location(shadow_cam, location->x, location->y, location->z);

	pi_aabb_init(&shadow_aabb);
	get_frustum_points(view_cam, z_far, points);
	view_mat = pi_camera_get_view_matrix(shadow_cam);
	for (i = 0; i < 8; i++) {
		pi_mat4_apply_point(&points[i], &points[i], view_mat);
		max_vec3(&points[i], &shadow_aabb.maxPt, &shadow_aabb.maxPt);
		min_vec3(&points[i], &shadow_aabb.minPt, &shadow_aabb.minPt);
	}
	fit_shadow_to_pixel(&shadow_aabb, sm_width, sm_height);
	pi_camera_set_frustum(shadow_cam, shadow_aabb.minPt.x, shadow_aabb.maxPt.x, shadow_aabb.minPt.y, shadow_aabb.maxPt.y, shadow_aabb.minPt.z-100, shadow_aabb.maxPt.z+100, TRUE);
}