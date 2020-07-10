#ifndef INCLUDE_RENDERSTATE_H
#define INCLUDE_RENDERSTATE_H

/**
 * 渲染状态
 */
#include <pi_lib.h>

#include <color.h>
#include <texture.h>

/* 纹理采样器单元的数目 */
#define SAMPLER_STATE_NUM 16

typedef enum 
{
	SM_FLAT,
	SM_GOURAUD,
	SM_PHONG,
	SM_FORCE_DWORD
} ShadeMode;

typedef enum
{
	PM_POINT,
	PM_LINE,
	PM_FILL
} PolygonMode;

typedef enum
{
	CM_NO,
	CM_FRONT,
	CM_BACK
} CullMode;

typedef enum
{
	BOP_ADD		= 1,
	BOP_SUB		= 2,
	BOP_REV_SUB	= 3,
	BOP_MIN		= 4,
	BOP_MAX		= 5,
} BlendOperation;

typedef enum
{
	BM_NONE,
	BM_ALPHA,
	BM_ADDITIVE,
	BM_MODULATE,
	BM_ALPHA_R

}BlendMode;

typedef enum
{
	BF_ZERO,
	BF_ONE,
	BF_SRC_ALPHA,
	BF_DST_ALPHA,
	BF_INV_SRC_ALPHA,
	BF_INV_DST_ALPHA,
	BF_SRC_COLOR,
	BF_DST_COLOR,
	BF_INV_SRC_COLOR,
	BF_INV_DST_COLOR,
	BF_SRC_ALPHA_SAT
} BlendFactor;

typedef enum
{
	CF_ALWAYSFAIL,
	CF_ALWAYSPASS,
	CF_LESS,
	CF_LESSEQUAL,
	CF_EQUAL,
	CF_NOTEQUAL,
	CF_GREATEREQUAL,
	CF_GREATER
} CompareFunction;

// Enum describing the various actions which can be taken on the stencil buffer
typedef enum
{
	// Leave the stencil buffer unchanged
	SOP_KEEP,
	// Set the stencil value to zero
	SOP_ZERO,
	// Set the stencil value to the reference value
	SOP_REPLACE,
	// Increase the stencil value by 1, clamping at the maximum value
	SOP_INCR,
	// Decrease the stencil value by 1, clamping at 0
	SOP_DECR,
	// Invert the bits of the stencil buffer
	SOP_INVERT,
	// Increase the stencil value by 1, wrap the result if necessary
	SOP_INCR_WRAP,
	// Decrease the stencil value by 1, wrap the result if necessary
	SOP_DECR_WRAP
} StencilOperation;

typedef enum
{
	CMASK_NONE  = 0,
	CMASK_RED   = 1UL << 0,
	CMASK_GREEN = 1UL << 1,
	CMASK_BLUE  = 1UL << 2,
	CMASK_ALPHA = 1UL << 3,
	CMASK_ALL   = CMASK_RED | CMASK_GREEN | CMASK_BLUE | CMASK_ALPHA
} ColorMask;

// Sampler addressing modes - default is TAM_Wrap.
typedef enum
{
	// Texture wraps at values over 1.0
	TAM_WRAP,
	// Texture mirrors (flips) at joins over 1.0
	TAM_MIRROR,
	// Texture clamps at 1.0
	TAM_CLAMP,
	// Texture coordinates outside the range [0.0, 1.0] are set to the border color.
	TAM_BORDER
} TexAddressMode;

typedef enum
{
	/* Don't use these enum directly */
	TFOE_MIP_POINT = 0X1,
	TFOE_MIP_LINEAR = 0X2,
	TFOE_MAG_POINT = 0X4,
	TFOE_MAG_LINEAR = 0X8,
	TFOE_MIN_POINT = 0X10,
	TFOE_MIN_LINEAR = 0X20,
	TFOE_ANISOTROPIC = 0X40,
	TFOE_COMPARISON = 0X80,

	/* use these */
	TFO_MIN_MAG_POINT				= TFOE_MIN_POINT  | TFOE_MAG_POINT,
	TFO_MIN_POINT_MAG_LINEAR		= TFOE_MIN_POINT  | TFOE_MAG_LINEAR,
	TFO_MIN_LINEAR_MAG_POINT		= TFOE_MIN_LINEAR | TFOE_MAG_POINT,
	TFO_MIN_MAG_LINEAR				= TFOE_MIN_LINEAR | TFOE_MAG_LINEAR,

	TFO_MIN_MAG_MIP_POINT				= TFOE_MIN_POINT  | TFOE_MAG_POINT  | TFOE_MIP_POINT,
	TFO_MIN_MAG_POINT_MIP_LINEAR		= TFOE_MIN_POINT  | TFOE_MAG_POINT  | TFOE_MIP_LINEAR,
	TFO_MIN_POINT_MAG_LINEAR_MIP_POINT	= TFOE_MIN_POINT  | TFOE_MAG_LINEAR | TFOE_MIP_POINT,
	TFO_MIN_POINT_MAG_MIP_LINEAR		= TFOE_MIN_POINT  | TFOE_MAG_LINEAR | TFOE_MIP_LINEAR,
	TFO_MIN_LINEAR_MAG_MIP_POINT		= TFOE_MIN_LINEAR | TFOE_MAG_POINT  | TFOE_MIP_POINT,
	TFO_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= TFOE_MIN_LINEAR | TFOE_MAG_POINT  | TFOE_MIP_LINEAR,
	TFO_MIN_MAG_LINEAR_MIP_POINT		= TFOE_MIN_LINEAR | TFOE_MAG_LINEAR | TFOE_MIP_POINT,
	TFO_MIN_MAG_MIP_LINEAR				= TFOE_MIN_LINEAR | TFOE_MAG_LINEAR | TFOE_MIP_LINEAR,
	TFO_ANISOTROPIC						= TFOE_ANISOTROPIC,

	TFO_CMP_MIN_MAG_MIP_POINT				= TFOE_COMPARISON | TFO_MIN_MAG_MIP_POINT,
	TFO_CMP_MIN_MAG_POINT_MIP_LINEAR		= TFOE_COMPARISON | TFO_MIN_MAG_POINT_MIP_LINEAR,
	TFO_CMP_MIN_POINT_MAG_LINEAR_MIP_POINT	= TFOE_COMPARISON | TFO_MIN_POINT_MAG_LINEAR_MIP_POINT,
	TFO_CMP_MIN_POINT_MAG_MIP_LINEAR		= TFOE_COMPARISON | TFO_MIN_POINT_MAG_MIP_LINEAR,
	TFO_CMP_MIN_LINEAR_MAG_MIP_POINT		= TFOE_COMPARISON | TFO_MIN_LINEAR_MAG_MIP_POINT,
	TFO_CMP_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= TFOE_COMPARISON | TFO_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	TFO_CMP_MIN_MAG_LINEAR_MIP_POINT		= TFOE_COMPARISON | TFO_MIN_MAG_LINEAR_MIP_POINT,
	TFO_CMP_MIN_MAG_MIP_LINEAR				= TFOE_COMPARISON | TFO_MIN_MAG_MIP_LINEAR,
	TFO_CMP_ANISOTROPIC						= TFOE_COMPARISON | TFO_ANISOTROPIC
} TexFilterOp;

typedef struct
{
	CullMode			cull_mode;
	PolygonMode			polygon_mode;

	PiBool				is_front_face_ccw;

	float				polygon_offset_units;
	float				polygon_offset_factor;

	PiBool				is_depth_clip_enable;
	PiBool				is_scissor_enable;
	PiBool				is_multisample_enable;
	ShadeMode			shading_mode;
} RasterizerState;

typedef struct
{
	PiBool				is_depth_enable;
	PiBool				is_depth_write_mask;
	CompareFunction		depth_func;

	PiBool				is_stencil_enable;
	uint8				stencil_read_mask;
	uint8				stencil_write_mask;
	uint8				stencil_ref;

	StencilOperation	front_stencil_fail;
	StencilOperation	front_stencil_depth_fail;
	StencilOperation	front_stencil_pass;
	CompareFunction		front_stencil_func;

	StencilOperation	back_stencil_fail;
	StencilOperation	back_stencil_depth_fail;
	StencilOperation	back_stencil_pass;
	CompareFunction		back_stencil_func;
} DepthStencilState;

/**
 * TODO: D3D11里面，多渲染目标允许每个目标有自己的混合参数
 */
typedef struct
{
	PiBool				is_alpha_to_coverage_enable;
	PiBool				is_independent_blend_enable;	/* D3D11才有 */

	PiBool				is_blend_enable;
	BlendOperation		blend_op;
	BlendFactor			src_blend;
	BlendFactor			dest_blend;
	BlendOperation		blend_op_alpha;
	BlendFactor			src_blend_alpha;
	BlendFactor			dest_blend_alpha;
	uint8				color_write_mask;
} BlendState;

typedef struct
{
	PiTexture *tex;

	PiColor border_clr;

	TexAddressMode addr_mode_u;
	TexAddressMode addr_mode_v;
	TexAddressMode addr_mode_w;

	TexFilterOp filter;

	uint8 max_anisotropy;
	float min_lod;
	float max_lod;
	float mip_map_lod_bias;

	CompareFunction cmp_func;
} SamplerState;

typedef enum
{
	/* RasterizerStateType */
	RST_POLYGON_MODE = 1,
	RST_CULL_MODE,
	RST_IS_FRONT_FACE_CCW,
	RST_POLYGON_OFFSET,
	RST_IS_DEPTH_CLIP_ENABLE,
	RST_IS_SCISSOR_ENABLE,
	RST_IS_MULTISAMPLE_ENABLE,
	RST_SHADING_MODE,

	/* DepthStencilStateType */
	RST_IS_DEPTH_ENABLE,
	RST_IS_DEPTH_WRITE_MASK,
	RST_DEPTH_FUNC,
	RST_IS_STENCIL_ENABLE,
	RST_STENCIL,
	RST_FRONT_STENCIL_OP,
	RST_BACK_STENCIL_OP,

	/* BlendStateType */
	RST_IS_ALPHA_TO_COVERAGE_ENABLE,
	RST_IS_INDEPENDENT_BLEND_ENABLE,
	RST_IS_BLEND_ENABLE,
	RST_BLEND_OP,
	RST_BLEND_FACTOR,
	RST_COLOR_WRITE_MASK
} RenderStateType;

typedef enum
{
	MULTISAMPLE_NONE = 0,
	MULTISAMPLE_NONMASKABLE = 1,
	MULTISAMPLE_2_SAMPLES = 2,
	MULTISAMPLE_3_SAMPLES = 3,
	MULTISAMPLE_4_SAMPLES = 4,
	MULTISAMPLE_5_SAMPLES = 5,
	MULTISAMPLE_6_SAMPLES = 6,
	MULTISAMPLE_7_SAMPLES = 7,
	MULTISAMPLE_8_SAMPLES = 8,
	MULTISAMPLE_9_SAMPLES = 9,
	MULTISAMPLE_10_SAMPLES = 10,
	MULTISAMPLE_11_SAMPLES = 11,
	MULTISAMPLE_12_SAMPLES = 12,
	MULTISAMPLE_13_SAMPLES = 13,
	MULTISAMPLE_14_SAMPLES = 14,
	MULTISAMPLE_15_SAMPLES = 15,
	MULTISAMPLE_16_SAMPLES = 16,
	MULTISAMPLE_FORCE_DWORD = 0x7fffffff
} MULTISAMPLE_TYPE;


/* 最大状态的长度 */
#define MAX_STATE_LEN 30

/* 状态列表 */
typedef struct
{
	uint len;								/* 变化状态的长度 */
	RenderStateType key[MAX_STATE_LEN];		/* 相对于默认状态的变化状态键，从小到大排序 */
	uint32 value[MAX_STATE_LEN];			/* 相对于默认状态的变化状态值 */
} StateList;

/**
 * 全状态信息
 */
typedef struct
{
	StateList diff;								/* 相对于默认状态的变化状态 */
	uint32 state[MAX_STATE_LEN];				/* 全状态 */
} FullState;



PI_BEGIN_DECLS

PiBool PI_API pi_renderstate_set_default_rasterizer(RasterizerState *rs);

PiBool PI_API pi_renderstate_set_default_blend(BlendState *bs);

PiBool PI_API pi_renderstate_set_default_depthstencil(DepthStencilState *dss);

PiBool PI_API pi_renderstate_set_default_sampler(SamplerState *ss);

PiBool PI_API pi_renderstate_set(RenderStateType key, uint32 value);

PiBool PI_API pi_renderstate_set_list(StateList *lst);

uint32 PI_API pi_renderstate_get(RenderStateType key);

PI_END_DECLS

#endif /* INCLUDE_RENDERSTATE_H */