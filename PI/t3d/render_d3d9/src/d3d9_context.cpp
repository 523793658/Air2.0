#include "d3d9_context.h"
#include "d3d9_convert.h"
#include "rendersystem.h"
// 创建context对应的d3d设备和表面

static PiBool _init_context(D3D9Context *context, PiBool is_create)
{
	D3DPRESENT_PARAMETERS d3dpp;
	PiRenderSystem *system = pi_rendersystem_get_instance();

	pi_memset(&d3dpp, 0, sizeof(d3dpp));

	d3dpp.Flags = 0;

	// 后备缓冲区，格式ARGB8
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = context->width;
	d3dpp.BackBufferHeight = context->height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;

	// 默认创建交换链的深度模板，D24S8
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	D3DMULTISAMPLE_TYPE multiType;
	multiType = d3d9_multi_sample_get(system->contextParams.multiSampleType);
	if (multiType != D3DMULTISAMPLE_NONE)
	{
		if (context->d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, context->is_window_mode, multiType, NULL) != D3D_OK)
		{
			multiType = D3DMULTISAMPLE_NONE;
		}
	}
	if (multiType == D3DMULTISAMPLE_FORCE_DWORD)
	{
		d3dpp.MultiSampleQuality = system->contextParams.multiSampleQuality;
	}

	d3dpp.MultiSampleType = multiType;

	
	// 开启 垂直同步，用：D3DPRESENT_INTERVAL_DEFAULT 或 D3DPRESENT_INTERVAL_ONE
	// 关闭 垂直同步，用：D3DPRESENT_INTERVAL_IMMEDIATE
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	d3dpp.Windowed = context->is_window_mode;
	
	if (context->is_window_mode)
	{// 窗口模式
		d3dpp.hDeviceWindow = context->hwnd;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}
	else 
	{// 全屏模式
		d3dpp.hDeviceWindow = 0;
		d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	}

	HRESULT hr = D3D_OK;
	if (is_create)
	{
		D3DADAPTER_IDENTIFIER9 ap;
		IDirect3D9_GetAdapterIdentifier(context->d3d, D3DADAPTER_DEFAULT, NULL, &ap);
		context->vender = (DeviceVender)ap.VendorId;
		hr = IDirect3D9_CreateDevice(context->d3d, D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, context->hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &context->device);
	}
	else 
	{
		hr = IDirect3DDevice9_Reset(context->device, &d3dpp);
		D3DADAPTER_IDENTIFIER9 ap;
		IDirect3D9_GetAdapterIdentifier(context->d3d, D3DADAPTER_DEFAULT, NULL, &ap);
		context->vender = (DeviceVender)ap.VendorId;
	}

	if (SUCCEEDED(hr))
	{
		// 获取后备缓冲区和深度模板缓冲区的句柄
		IDirect3DDevice9_GetBackBuffer(context->device, 0, 0, D3DBACKBUFFER_TYPE_MONO, &context->surface);
		IDirect3DDevice9_GetDepthStencilSurface(context->device, &context->depth_stencil_surface);
	}
	else
	{
		char *reason = "unkown";
		switch (hr)
		{
		case D3DERR_INVALIDCALL:
			reason = "D3DERR_INVALIDCALL";
			break;
		case D3DERR_DEVICELOST:
			reason = "D3DERR_DEVICELOST";
			break;
		case D3DERR_DRIVERINTERNALERROR:
			reason = "D3DERR_DRIVERINTERNALERROR";
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			reason = "D3DERR_OUTOFVIDEOMEMORY";
			break;
		default:
			break;
		}
		pi_log_print(LOG_WARNING, "IDirect3DDevice9_Reset failed, hr = %d, reason = %s\n", hr, reason);
	}

	return SUCCEEDED(hr);
}

D3D9Context *d3d9_context_new(HWND hwnd, PiBool is_window_mode, uint width, uint height)
{
	D3D9Context *context = NULL;
	IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
	{
		pi_log_print(LOG_WARNING, "Direct3DCreate9 failed, error = %d\n", GetLastError());
		return NULL;
	}

	context = pi_new0(D3D9Context, 1);
	context->d3d = d3d;
	context->hwnd = hwnd;
	context->width = width;
	context->height = height;
	context->is_window_mode = is_window_mode;

	if (!_init_context(context, TRUE))
	{
		IDirect3D9_Release(d3d);
		pi_free(context);
		context = NULL;
	}
	
	return context;
}

void d3d9_context_free(D3D9Context *context)
{
	IDirect3DSurface9_Release(context->surface);
	IDirect3DSurface9_Release(context->depth_stencil_surface);
	IDirect3DDevice9_Release(context->device);
	
	IDirect3D9_Release(context->d3d);
	pi_free(context);
}

PiBool d3d9_context_reset(D3D9Context *context, PiBool is_window_mode, uint width, uint height)
{
	context->width = width;
	context->height = height;
	context->is_window_mode = is_window_mode;

	IDirect3DSurface9_Release(context->surface);
	IDirect3DSurface9_Release(context->depth_stencil_surface);

	return _init_context(context, FALSE);
}

PiBool d3d9_context_swapbuffer(D3D9Context *context)
{
	return IDirect3DDevice9_Present(context->device, NULL, NULL, NULL, NULL);
}

void d3d9_context_get_size(D3D9Context *context, sint *w, sint *h)
{
	if (w != NULL)
		*w = context->width;
	if (h != NULL)
		*h = context->height;
}
