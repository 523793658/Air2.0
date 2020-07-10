#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include <pi_lib.h>

#include <shader.h>
#include <renderutil.h>
#include <renderstate.h>
#define	MAX_UNIFORM_BUFFER_SIZE 1

typedef struct
{
	char *vs_key;				/* vs源码对应的键 */
	char *fs_key;				/* fs源码对应的键 */
	
	float cull_off;

	/* 宏，带define的 */
	uint num_defs;
	char *defs[MAX_DEFINE_NUM];
	
	/* set_def传进来的宏的名字的字符串常量的指针，无需释放 */
	char *def_names[MAX_DEFINE_NUM];
	
	void* program;		
	ShaderData *vs, *fs;
	
	StateList state;			/* 相对于环境状态的变化状态 */

	uint num_uniforms;
	uint valid_uniforms_num;	/* 目前uniforms中在program有效的个数 */
	Uniform *uniforms[MAX_UNIFORMS_NUM];	/* 相对于环境变量的变化变量 */
	Uniform *available_uniforms[MAX_UNIFORMS_NUM];		/*可用Uniform*/

	PiVector4I uniforms_buffer_int[MAX_UNIFORM_BUFFER_SIZE];
} PiMaterial;

PI_BEGIN_DECLS

/* -------------------  采样器 设置 ------------------- */

SamplerState* PI_API pi_sampler_new(void);

void PI_API pi_sampler_free(SamplerState *ss);
 
void PI_API pi_sampler_set_texture(SamplerState *ss, PiTexture *tex);

void PI_API pi_sampler_set_addr_mode(SamplerState *ss, TexAddressMode u, TexAddressMode v, TexAddressMode w);

void PI_API pi_sampler_set_border_color(SamplerState *ss, PiColor *color);

void PI_API pi_sampler_set_filter(SamplerState *ss, TexFilterOp filter);

void PI_API pi_sampler_set_lod(SamplerState *ss, float min_lod, float max_lod);

void PI_API pi_sampler_set_compare_func(SamplerState *ss, CompareFunction func);

void PI_API pi_sampler_set_mip_map_lod_bias(SamplerState *ss, float bias);

void PI_API pi_sampler_set_max_anisotropy(SamplerState *ss, uint8 max_anisotropy);

/* -------------------  材质 设置 ------------------- */

PiMaterial* PI_API pi_material_new(const char *vs_key, const char *fs_key);

void PI_API pi_material_free(PiMaterial *material);

/**
 * defs：不带"#define "
 * 注：def必须是pi_conststr返回值
 */
void PI_API pi_material_set_def(PiMaterial *material, const char *def, PiBool is_enable);

/**
 * 深拷贝，同名就替换掉老的值
 * 如果data为NULL，移除该uniform
 * is_copy: 是否拷贝data值
 */
void PI_API pi_material_set_uniform(PiMaterial *material, const char *name, UniformType type, uint32 count, void *data, PiBool is_copy);

void PI_API pi_material_set_uniform_pack_flag(PiMaterial *material, const char *name, UniformType type, uint32 count, void *data, PiBool is_copy, PiBool isPacked);

Uniform *PI_API pi_material_get_uniform(PiMaterial *material, const char *name);

void PI_API pi_material_set_cull_mode(PiMaterial *material, CullMode cull);

void PI_API pi_material_set_polygon_mode(PiMaterial *material, PolygonMode mode);

void PI_API pi_material_set_scissor_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_frontface(PiMaterial *material, PiBool is_ccw);

void PI_API pi_material_set_depthclip_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_multisample_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_polygon_offset(PiMaterial *material, float units, float factor);

void PI_API pi_material_set_depth_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_depthwrite_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_depth_compfunc(PiMaterial *material, CompareFunction func);

void PI_API pi_material_set_stencil_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_stencil(PiMaterial *material, uint8 read_mask, uint8 write_mask, uint8 ref);

void PI_API pi_material_set_front_stencil_op(PiMaterial *material, StencilOperation fail_op,
	StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func);

void PI_API pi_material_set_back_stencil_op(PiMaterial *material, StencilOperation fail_op,
	StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func);

void PI_API pi_material_set_a2c_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_independent_blend_enable(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_blend(PiMaterial *material, PiBool is_enable);

void PI_API pi_material_set_shade_mode(PiMaterial *material, ShadeMode mode);

void PI_API pi_material_set_blend_op(PiMaterial *material, BlendOperation alpha_op, BlendOperation color_op);

void PI_API pi_material_set_blend_func(PiMaterial *material, BlendMode mode);

void PI_API pi_material_set_alpha_cull_off(PiMaterial *material, float cull_off);

void PI_API pi_material_set_blend_factor(PiMaterial *material, 
	BlendFactor src_blend, BlendFactor dst_blend,
	BlendFactor src_alpha_blend, BlendFactor dst_alpha_blend);

void PI_API pi_material_set_color_mask(PiMaterial *material, uint8 color_mask);

/* 更新material，只有当不用renderpipeline，renderbatch，renderlist时候，才需要调用这个函数 */
PiBool PI_API pi_material_update(PiMaterial *material);

PI_END_DECLS

#endif /* INCLUDE_MATERIAL_H */