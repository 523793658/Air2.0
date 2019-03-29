#include "D3D11Util.h"
namespace Air
{
#ifndef MAKE_D3DHRESULT
#define _FACD3D	0x876
#define MAKE_D3DHRESULT(code) MAKE_HRESULT(1, _FACD3D, code)
#endif


#if WITH_D3DX_LIBS
#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL MAKE_D3DHRESULT(2156)
#endif
#ifndef D3DERR_WASSTILLDRAWING
#define D3DERR_WASSTILLDRAWING MAKE_D3DHRESULT(540)
#endif


#endif


#define D3DERR(x) case x: errorCodeText = TEXT(#x);break;


	static wstring getD3D11DeviceHungErrorString(HRESULT ErrorCode)
	{
		wstring errorCodeText;
		switch (ErrorCode)
		{
			D3DERR(DXGI_ERROR_DEVICE_HUNG)
				D3DERR(DXGI_ERROR_DEVICE_REMOVED)
				D3DERR(DXGI_ERROR_DEVICE_RESET)
				D3DERR(DXGI_ERROR_DRIVER_INTERNAL_ERROR)
				D3DERR(DXGI_ERROR_INVALID_CALL)
		default:
			errorCodeText = printf(TEXT("%08X"), (int32)ErrorCode);
		}
		return errorCodeText;
	}

	static wstring getD3D11ErrorString(HRESULT ErrorCode, ID3D11Device* device)
	{
		wstring errorCodeText;
		switch (ErrorCode)
		{
			D3DERR(S_OK)
			D3DERR(D3D11_ERROR_FILE_NOT_FOUND)
			D3DERR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS)
			D3DERR(D3DERR_INVALIDCALL)
			D3DERR(D3DERR_WASSTILLDRAWING)
			D3DERR(E_FAIL)
			D3DERR(E_INVALIDARG)
			D3DERR(E_OUTOFMEMORY)
			D3DERR(DXGI_ERROR_INVALID_CALL)
			D3DERR(E_NOINTERFACE)
			D3DERR(DXGI_ERROR_DEVICE_REMOVED)
		default:
			errorCodeText = printf(TEXT("%08X"), (int32)ErrorCode);
		}
		if (ErrorCode == DXGI_ERROR_DEVICE_REMOVED && device)
		{
			HRESULT hResDeviceRemoved = device->GetDeviceRemovedReason();
			errorCodeText += TEXT(" ") + getD3D11DeviceHungErrorString(hResDeviceRemoved);
		}
		return errorCodeText;
	}

	void verifyD3D11Result(HRESULT result, const ANSICHAR* code, const ANSICHAR* filename, uint32 line, ID3D11Device* device)
	{
		BOOST_ASSERT(FAILED(result));
		const wstring & errorString = getD3D11ErrorString(result, device);
		
	}

	void verifyD3D11CreateTextureResult(HRESULT d3dResult, const ANSICHAR* code, const ANSICHAR* filename, uint32 line, uint32 width, uint32 height, uint32 depth, uint8 d3dFormat, uint32 numMips, uint32 flags, ID3D11Device* device)
	{

	}

	D3D11BoundRenderTargets::D3D11BoundRenderTargets(ID3D11DeviceContext* inDeviceContext)
	{
		Memory::memzero(mRenderTargetViews, sizeof(mRenderTargetViews));
		mDepthStencilView = nullptr;
		inDeviceContext->OMGetRenderTargets(MaxSimultaneousRenderTargets, &mRenderTargetViews[0], &mDepthStencilView);
		for (mNumActiveTargets = MaxSimultaneousRenderTargets; mNumActiveTargets > 0; --mNumActiveTargets)
		{
			if (mRenderTargetViews[mNumActiveTargets - 1] != nullptr)
			{
				break;
			}
		}
	}

	D3D11BoundRenderTargets::~D3D11BoundRenderTargets()
	{
		for (int32 targetIndex = 0; targetIndex < mNumActiveTargets; ++targetIndex)
		{
			if (mRenderTargetViews[targetIndex] != nullptr)
			{
				mRenderTargetViews[targetIndex]->Release();
			}
		}
		if (mDepthStencilView)
		{
			mDepthStencilView->Release();
		}
	}
}