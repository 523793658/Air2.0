#ifndef INCLUDE_D3D9_RENDERSYSTEM_H
#define INCLUDE_D3D9_RENDERSYSTEM_H

/** 
 * 渲染入口和单例
 */

#include "d3d9_context.h"
#include "d3d9_renderstate.h"


typedef struct
{
	D3D9Context *context;    /* 渲染环境 */

	D3D9RenderState state;   /* 渲染状态 */
} D3D9RenderSystem;

#endif /* INCLUDE_D3D9_RENDERSYSTEM_H */
