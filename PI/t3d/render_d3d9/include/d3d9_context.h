#ifndef INCLUDE_D3D9_CONTEXT_H
#define INCLUDE_D3D9_CONTEXT_H

/**
 * D3D9的环境：比如设备，后备表面等
 */

#include "pi_lib.h"
#include <d3d9.h>
#define FOURCC_INTZ ((D3DFORMAT)(MAKEFOURCC('I','N','T','Z')))
#define FOURCC_NULL ((D3DFORMAT)(MAKEFOURCC('N','U','L','L')))


typedef enum
{
	AMD = 0x1022,
	NVIDIA = 0x10DE,
	Intel = 0x8086,
}DeviceVender;

typedef struct
{
	HWND hwnd;									/* 窗口句柄 */
	uint width, height;							/* 设备的宽 高 */
	PiBool is_window_mode;						/* 是否窗口模式 */
		
	IDirect3D9 *d3d;							/* D3D9接口 */
	
	IDirect3DDevice9 *device;					/* D3D9设备 */
	IDirect3DSurface9 *surface;					/* SwapChain 颜色 BackBuffer */
	IDirect3DSurface9 *depth_stencil_surface;	/* SwapChain 深度模板 BackBuffer */
	DeviceVender vender;
} D3D9Context;

PI_BEGIN_DECLS

/* 创建d3d9环境：默认窗口模式 */
D3D9Context *d3d9_context_new(HWND hwnd, PiBool is_window_mode, uint width, uint height);

/* 释放d3d9环境 */
void d3d9_context_free(D3D9Context *context);

/* 重置d3d环境：宽、高、是否窗口模式
 * 注：调用这个函数之前，必须释放所有的D3DPOOL_DEFAULT资源（包括用CreateRenderTarget和CreateDepthStencilSurface方法创建的资源）
 * 注：调用这个函数之后，必须如果成功，需要自行修复先前释放的资源
 */
PiBool d3d9_context_reset(D3D9Context *context, PiBool is_window_mode, uint width, uint height);

/* 交换缓冲区，注：如果失败，则是设备丢失引起的 */
PiBool d3d9_context_swapbuffer(D3D9Context *context);

/* 取设备的宽高 */
void d3d9_context_get_size(D3D9Context *context, sint *w, sint *h);

PI_END_DECLS

#endif /* INCLUDE_D3D9_CONTEXT_H */