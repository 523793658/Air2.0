#ifndef INCLUDE_RENDERUTIL_H
#define INCLUDE_RENDERUTIL_H

#include <pi_lib.h>
#include <pi_matrix4.h>

#include <shader.h>
#include <renderstate.h>
#include <camera.h>
#include <pi_aabb.h>
#include "vector4.h"

#define G_TIME "g_Time"
#define G_WORLD_MATRIX "g_WorldMatrix"
#define G_VIEW_MATRIX "g_ViewMatrix"
#define G_PROJ_MATRIX "g_ProjMatrix"
#define G_WORLD_VIEW_MATRIX "g_WorldViewMatrix"
#define G_VIEWPROJ_MATRIX "g_ViewProjMatrix"
#define G_WORLD_VIEW_PROJ_MATRIX "g_WorldViewProjMatrix"
#define G_NORMAL_MATRIX "g_NormalMatrix"
#define G_VIEW_NORMAL_MATRIX "g_ViewNormalMatrix"
#define G_VIEWPORT_SIZE "g_ViewportSize"
#define G_ALPHACULLOFF "g_AlphaCullOff"
#define G_ShadowData "g_ShadowData"
#define G_ShadowMap "g_ShadowMap"
#define G_EnvironmentData "g_EnvironmentData"
#define G_ViewPosition "g_ViewPosition"


typedef enum 
{
	WVP_VIEW_PROJ = 1,			/* 当view或proj变化时，要重置 */
	WVP_WORLD_VIEW = 2,			/* 当world或view变化时，要重置 */
	WVP_VIEW_NORMAL = 4,		/* 当world或view变化时，要重置 */
	WVP_WORLD_VIEW_PROJ = 8,	/* 当world或view或proj变化时，要重置 */
	WVP_NORMAL = 16				/* 当world变化时，要重置 （该矩阵为世界矩阵的转的逆矩阵）*/
} WVPMaskType;
typedef struct _ShadowData
{
	PiVector4 params;
	SamplerState texture;
}ShadowData;

/* 缺省变量 */
typedef struct
{
	ShadowData g_ShadowData;
	PiVector3 g_viewPosition;
	float	g_Time;
	uint g_viewport[2];
	PiMatrix4 *g_world;
	PiMatrix4 *g_view;
	PiMatrix4 *g_proj;
	float g_alpha_cull_off;
	/* 以下矩阵是否已经已经计算 */
	uint wvp_mask;				/* WVP_MASK_TYPE 的组合 */
	PiMatrix4 g_view_proj;
	PiMatrix4 g_world_view;
	PiMatrix4 g_world_view_proj;
	PiMatrix4 g_normal;
	PiMatrix4 g_view_normal;

} GDefaultVariable;

PI_BEGIN_DECLS

/*设置阴影数据*/
void renderutil_set_shadow_data(GDefaultVariable *gdv, PiTexture* map);

/* 初始化全局状态 */
PiBool PI_API pi_renderutil_init_fullstate(uint32 *state);

void pi_uniform_value_pack(UniformType type, byte* dst, byte* src, uint count);

uint pi_uniform_value_size(UniformType type, uint32 array_num);

/* 处理默认变量 */
void renderutil_set_default_variable(void* shader, GDefaultVariable *gdv, Uniform *u);

uint renderutil_get_polygon_offset(float factor, float units);

uint renderutil_get_stencil(uint8 write_mask, uint8 read_mask, uint8 ref);

uint renderutil_get_stencil_op(StencilOperation fail_op, StencilOperation depth_fail_op, StencilOperation stencil_pass_op, CompareFunction func);

uint renderutil_get_blend_op(BlendOperation alpha_op, BlendOperation color_op);

uint renderutil_get_blend_factor(BlendFactor src_blend, BlendFactor dst_blend, BlendFactor src_alpha_blend, BlendFactor dst_alpha_blend);

/* 在状态列表中添加指定的状态，如果key有相同的则替换 */
PiBool PI_API pi_renderutil_state_add(StateList *list, RenderStateType key, uint32 value);

/* 在状态列表中移除指定的key状态 */
PiBool PI_API pi_renderutil_state_remove(StateList *list, RenderStateType key);

/* 有序比较并设置渲染状态 */
PiBool PI_API pi_renderutil_state_order_set(const uint32 *state, const StateList *last, const StateList *cur);

/* 合并有序的渲染状态 */
void renderutil_merge_order_set(StateList *dest, const StateList *last, const StateList *cur);

/* 在uniform列表中添加指定的uniform */
void renderutil_add_uniform(PiDvector *list, Uniform *uniform);

/* 在uniform列表中移除指定name的uniform */
void renderutil_remove_uniform(PiDvector *list, const char *name);

/* 释放uniform列表每个unifrom */
PiBool renderutil_clear_uniform(PiDvector *list);

/* 初始化unifrom，value被保留 */
void renderutil_init_uniform(Uniform *uniform, char* name, UniformType type, void *value, uint32 array_num);

/* 判断unifrom的类型，如果value为额外分配的内存，则释放 */
PiSelectR PI_API renderutil_free_uniform(void* user_data, Uniform *uniform);

/* 是否是合法的uniform */
PiBool renderutil_is_valid_uniform(UniformType type, void *value);

/* 比较uniform，返回是否相等 */
PiBool renderutil_is_same_uniform(Uniform *dest, const Uniform *src);

/* 复制uniform，如果src的hash为0，则自动计算hash */
PiBool renderutil_copy_uniform(Uniform *dest, const Uniform *src);

/* 比较、复制并设置uniform */
PiBool renderutil_program_cc_set(void* program, Uniform *dest, Uniform *src);

void renderutil_update_shadow_cam(PiCamera* view_cam, PiCamera* shadow_cam, float proj_width, float proj_height, float z_far);

void PI_API fit_shadow_to_pixel(PiAABBBox* shadow_aabb, float sm_width, float sm_height);

PI_END_DECLS

#endif /* INCLUDE_RENDERUTIL_H */