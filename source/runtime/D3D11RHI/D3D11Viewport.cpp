#include "HAL/PlatformTime.h"
#include "HAL/PlatformProcess.h"
#include "Math/Math.h"
#include "RenderUtils.h"
#include "RenderingThread.h"
#include "D3D11DynamicRHI.h"
#include "D3D11Viewport.h"
#include "D3D11Util.h"
#include "D3D11Texture.h"
#include "RenderResource.h"

#include <dwmapi.h>
#include <dxsdk/Include/dxgi.h>
namespace Air
{

#ifndef D3D11_WITH_DWMAPI
#if WINVER > 0x502
#define D3D11_WITH_DWMAPI	1
#else
#define D3D11_WITH_DWMAPI	0
#endif
#endif

	extern void D3D11TextureAllocated2D(D3D11Texture2D& texture);

	
	D3D11Texture2D* getSwapChainSurface(D3D11DynamicRHI* d3dRHI, EPixelFormat pixelFormat, IDXGISwapChain* swapChain)
	{
		TRefCountPtr<ID3D11Texture2D> backBufferResource;
		VERIFYD3D11RESULT_EX(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBufferResource.getInitReference()), d3dRHI->getDevice());
		TRefCountPtr<ID3D11RenderTargetView> backBufferRenderTargetView;
		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.Format = DXGI_FORMAT_UNKNOWN;
		RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		VERIFYD3D11RESULT_EX(d3dRHI->getDevice()->CreateRenderTargetView(backBufferResource, &RTVDesc, backBufferRenderTargetView.getInitReference()), d3dRHI->getDevice());
		D3D11_TEXTURE2D_DESC textureDesc;
		backBufferResource->GetDesc(&textureDesc);
		TArray<TRefCountPtr<ID3D11RenderTargetView>> RendertargetViews;
		RendertargetViews.push_back(backBufferRenderTargetView);

		TRefCountPtr<ID3D11ShaderResourceView> backBufferShaderResourceView;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		VERIFYD3D11RESULT_EX(d3dRHI->getDevice()->CreateShaderResourceView(backBufferResource, &srvDesc, backBufferShaderResourceView.getInitReference()), d3dRHI->getDevice());

		D3D11Texture2D* newTexture = new D3D11Texture2D(
			d3dRHI,
			backBufferResource,
			backBufferShaderResourceView,
			false,
			1,
			RendertargetViews,
			NULL,
			textureDesc.Width,
			textureDesc.Height,
			1, 1, 1,
			pixelFormat,
			false,
			false,
			false,
			ClearValueBinding()
		);
		D3D11TextureAllocated2D(*newTexture);
		newTexture->doNotDeferDelete();
		return newTexture;
	}

	static DXGI_FORMAT getRenderTargetFormat(EPixelFormat pixelFormat)
	{
		DXGI_FORMAT dxFormat = (DXGI_FORMAT)GPixelFormats[pixelFormat].PlatformFormat;
		switch (dxFormat)
		{
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_BC1_TYPELESS:
			return DXGI_FORMAT_BC4_UNORM;
		case DXGI_FORMAT_BC2_TYPELESS:
			return DXGI_FORMAT_BC2_UNORM;
		case DXGI_FORMAT_BC3_TYPELESS:
			return DXGI_FORMAT_BC3_UNORM;
		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		default:
			return dxFormat;
		}
	}

	namespace RHIConsoleVariables
	{
		int32 bSyncWithDWM = 0;
		int32 MaximumFrameLatency = 3;

		int32 SyncInterval = 1;

		int32 TargetRefreshRate = 0;

		float SyncRefreshThreshold = 1.05f;

		int32 MaxSyncCounter = 8;

		int32 SyncThreshold = 7;

		float RefreshPercentageBeforePresent = 1.0f;
	}


	static bool isCompositionEnable()
	{
		BOOL bDwmEnable = FALSE;
#if D3D11_WITH_DWMAPI
		DwmIsCompositionEnabled(&bDwmEnable);;
#endif
		return !!bDwmEnable;
	}

	D3D11Viewport::D3D11Viewport(D3D11DynamicRHI* d3dRHI, HWND hwnd, uint32 width, uint32 height, bool isFullscreen, EPixelFormat format):
		mD3DRHI(d3dRHI),
		mLastFlipTime(0),
		mLastFrameComplete(0),
		mLastCompleteTime(0),
		mSyncCounter(0),
		bSyncedLastFrame(false),
		mWindowHandle(hwnd),
		mMaximumFrameLatency(3),
		mSizeX(width),
		mSizeY(height),
		mIsFullscreen(isFullscreen),
		mPixelFormat(format),
		mIsValid(true),
		mFrameSyncEvent(d3dRHI)
	{
		BOOST_ASSERT(isInGameThread());
		mD3DRHI->mViewports.push_back(this);
		mD3DRHI->initD3DDevice();
		TRefCountPtr<IDXGIDevice> dxgiDevice;
		VERIFYD3D11RESULT_EX(mD3DRHI->getDevice()->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.getInitReference()), mD3DRHI->getDevice());
		if (mPixelFormat == PF_FloatRGBA && mIsFullscreen)
		{
			mD3DRHI->enableHDR();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		Memory::memzero(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		swapChainDesc.BufferDesc = setupDXGI_MODE_DESC();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = mWindowHandle;
		swapChainDesc.Windowed = !mIsFullscreen;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		VERIFYD3D11RESULT_EX(mD3DRHI->getFactory()->CreateSwapChain(dxgiDevice, &swapChainDesc, mSwapChain.getInitReference()), mD3DRHI->getDevice());

		mD3DRHI->getFactory()->MakeWindowAssociation(mWindowHandle, DXGI_MWA_NO_WINDOW_CHANGES);
		mBackBuffer = getSwapChainSurface(mD3DRHI, mPixelFormat, mSwapChain);

		::PostMessage(mWindowHandle, WM_PAINT, 0, 0);
		beginInitResource(&mFrameSyncEvent);
	}


	Texture2DRHIRef D3D11DynamicRHI::RHIGetViewportBackBuffer(RHIViewport* viewportRHI)
	{
		D3D11Viewport* viewport = ResourceCast(viewportRHI);
		return viewport->getBackBuffer();
	}

	bool D3D11Viewport::present(bool lockToVsync)
	{
		bool nativelyPresented = true;
#if D3D11_WITH_DWMAPI
		if (mIsValid)
		{
			BOOL bSwapChainFullScreenState;
			TRefCountPtr<IDXGIOutput> swapChainOutput;
			VERIFYD3D11RESULT_EX(mSwapChain->GetFullscreenState(&bSwapChainFullScreenState, swapChainOutput.getInitReference()), mD3DRHI->getDevice());
			if ((!!bSwapChainFullScreenState) != mIsFullscreen)
			{
				mIsValid = false;
				::ShowWindow(mWindowHandle, SW_MINIMIZE);
			}
		}
		if (mMaximumFrameLatency != RHIConsoleVariables::MaximumFrameLatency)
		{
			mMaximumFrameLatency = RHIConsoleVariables::MaximumFrameLatency;
			TRefCountPtr<IDXGIDevice1> dxgiDevice;
			VERIFYD3D11RESULT_EX(mD3DRHI->getDevice()->QueryInterface(__uuidof(IDXGIDevice1), (void**)dxgiDevice.getInitReference()), mD3DRHI->getDevice());
			dxgiDevice->SetMaximumFrameLatency(mMaximumFrameLatency);
		}
		const bool bSyncWithDWM = lockToVsync && !mIsFullscreen && RHIConsoleVariables::bSyncWithDWM && isCompositionEnable();
		if (bSyncWithDWM)
		{
			presentWithVsyncDWM();
		}
		else
#endif
		{
			nativelyPresented = presentChecked(lockToVsync ? RHIConsoleVariables::SyncInterval : 0);
		}
		return nativelyPresented;
	}

	void D3D11Viewport::presentWithVsyncDWM()
	{
#if D3D11_WITH_DWMAPI
		LARGE_INTEGER Cycles;
		DWM_TIMING_INFO TimingInfo;
		QueryPerformanceCounter(&Cycles);
		Memory::memzero(TimingInfo);
		TimingInfo.cbSize = sizeof(DWM_TIMING_INFO);
		DwmGetCompositionTimingInfo(nullptr, &TimingInfo);
		uint64 QpcAtFlip = Cycles.QuadPart;
		uint64 CyclesSincleLastFlip = Cycles.QuadPart - mLastFlipTime;
		float CPUTime = PlatformTime::toMilliseconds(CyclesSincleLastFlip);
		float GPUTime = PlatformTime::toMilliseconds(TimingInfo.qpcFrameComplete - mLastCompleteTime);
		float displayRefreshPeriod = PlatformTime::toMilliseconds(TimingInfo.qpcRefreshPeriod);
		float refreshPeriod = displayRefreshPeriod;
		if (RHIConsoleVariables::TargetRefreshRate > 0 && refreshPeriod > 1.0f)
		{
			while (refreshPeriod - (1000.0f / RHIConsoleVariables::TargetRefreshRate) < -1.0f)
			{
				refreshPeriod *= 2.0f;
			}
		}
		bool bValidGPUTime = (TimingInfo.cFrameComplete > mLastCompleteTime);
		if (bValidGPUTime)
		{
			GPUTime /= (float)(TimingInfo.cFrameComplete - mLastCompleteTime);
		}
		float frameTime = std::max<float>(CPUTime, GPUTime);
		if (frameTime >= RHIConsoleVariables::SyncRefreshThreshold * refreshPeriod)
		{
			mSyncCounter--;
		}
		else if(bValidGPUTime)
		{
			mSyncCounter++;
		}
		mSyncCounter = Math::clamp<int32>(mSyncCounter, 0, RHIConsoleVariables::MaxSyncCounter);

		bool bSync = (mSyncCounter >= RHIConsoleVariables::SyncThreshold);
		if (bSync)
		{
			mD3DRHI->getDeviceContext()->Flush();
			::DwmFlush();
			float minFrameTime = refreshPeriod * RHIConsoleVariables::RefreshPercentageBeforePresent;
			float timeToSleep;
			do 
			{
				QueryPerformanceCounter(&Cycles);
				float timeSinceFlip = PlatformTime::toMilliseconds(Cycles.QuadPart - mLastFlipTime);
				if (timeToSleep > 0.0f)
				{
					PlatformProcess::sleep(timeToSleep * 0.001f);
				}
			} while (timeToSleep > 0.0f);
		}
		presentChecked(0);
		if (bSync)
		{
			LARGE_INTEGER LocalCycles;
			float timeToSleep;
			bool bSaveCycles = false;
			do 
			{
				QueryPerformanceCounter(&LocalCycles);
				float timeScineFlip = PlatformTime::toMilliseconds(LocalCycles.QuadPart - mLastFlipTime);
				timeToSleep = (refreshPeriod - timeScineFlip);
				if (timeToSleep > 0.0f)
				{
					bSaveCycles = true;
					PlatformProcess::sleep(timeToSleep * 0.001f);
				}
			} while (timeToSleep > 0.0f);

			if (bSaveCycles)
			{
				Cycles = LocalCycles;
			}
		}

		if (!bSync && bSyncedLastFrame)
		{

		}
		bSyncedLastFrame = bSync;
		mLastFlipTime = Cycles.QuadPart;
		mLastFrameComplete = TimingInfo.cFrameComplete;
		mLastCompleteTime = TimingInfo.qpcFrameComplete;
#endif
	}

	bool D3D11Viewport::presentChecked(int32 syncInterval)
	{
		HRESULT result = S_OK;
		bool bNeedNativePresent = true;
		const bool bHasCustomPresent = isValidRef(mCustomPresent);
		if (bHasCustomPresent)
		{
			bNeedNativePresent = mCustomPresent->present(syncInterval);
			
		}
		if (bNeedNativePresent)
		{
			result = mSwapChain->Present(syncInterval, 0);
			if (bHasCustomPresent)
			{
				mCustomPresent->postPresent();
			}
		}
		VERIFYD3D11RESULT_EX(result, mD3DRHI->getDevice());
		return bNeedNativePresent;
	}

	void D3D11Viewport::conditionalResetSwapChain(bool bIgnoreFocus)
	{
		if (!mIsValid)
		{
			HWND focusWindow = ::GetFocus();
			const bool bisFocused = focusWindow == mWindowHandle;
			const bool bIsIconic = !!::IsIconic(mWindowHandle);
			if (bIgnoreFocus || (bisFocused && !bIsIconic))
			{
				flushRenderingCommands();

				RECT originalCursorRect;
				GetClipCursor(&originalCursorRect);
				bool bNeedsForcedDisplay = mIsFullscreen && (mForcedFullscreenDisplay || mPixelFormat == PF_FloatRGBA);
				HRESULT result = mSwapChain->SetFullscreenState(mIsFullscreen, bNeedsForcedDisplay ? mForcedFullscreenOutput : nullptr);
				if (SUCCEEDED(result))
				{
					ClipCursor(&originalCursorRect);
					mIsValid = true;
				}
				else
				{

				}
			}
		}
	}

	void D3D11Viewport::resize(uint32 inSizeX, uint32 inSizeY, bool inFullscreen, EPixelFormat preferredPixelFormat)
	{
		mD3DRHI->RHISetRenderTargets(0, nullptr, nullptr, 0, nullptr);
		mD3DRHI->clearState();
		mD3DRHI->getDeviceContext()->Flush();
		if (isValidRef(mCustomPresent))
		{
			mCustomPresent->OnBackBufferResize();
		}
		if (isValidRef(mBackBuffer))
		{
			BOOST_ASSERT(mBackBuffer->GetRefCount() == 1);
		}
		mBackBuffer.safeRelease();
		if (mSizeX != inSizeX || mSizeY != inSizeY || mPixelFormat != preferredPixelFormat)
		{
			mSizeX = inSizeX;
			mSizeY = inSizeY;
			mPixelFormat = preferredPixelFormat;
			VERIFYD3D11RESULT_EX(mSwapChain->ResizeBuffers(1, mSizeX, mSizeY, getRenderTargetFormat(mPixelFormat), DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH), mD3DRHI->getDevice());
			if (inFullscreen)
			{
				DXGI_MODE_DESC bufferDesc = setupDXGI_MODE_DESC();
				if (FAILED(mSwapChain->ResizeTarget(&bufferDesc)))
				{
					conditionalResetSwapChain(true);
				}
			}
		}
		if (mIsFullscreen != inFullscreen)
		{
			mIsFullscreen = inFullscreen;
			mIsValid = false;
			conditionalResetSwapChain(true);
		}

		if (mPixelFormat == PF_FloatRGBA && mIsFullscreen)
		{
			mD3DRHI->enableHDR();
		}
		else
		{
			mD3DRHI->shutdownHDR();
		}
		mBackBuffer = getSwapChainSurface(mD3DRHI, mPixelFormat, mSwapChain);

		::PostMessage(mWindowHandle, WM_PAINT, 0, 0);
		
		beginInitResource(&mFrameSyncEvent);
	}

	DXGI_MODE_DESC D3D11Viewport::setupDXGI_MODE_DESC()
	{
		DXGI_MODE_DESC desc;
		desc.Width = mSizeX;
		desc.Height = mSizeY;
		desc.Format = getRenderTargetFormat(mPixelFormat);
		desc.RefreshRate.Denominator = 0;
		desc.RefreshRate.Numerator = 0;
		desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		return desc;
	}

	void D3D11EventQuery::releaseDynamicRHI()
	{
		mQuery = NULL;
	}

	void D3D11EventQuery::initDynamicRHI()
	{
		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_EVENT;
		desc.MiscFlags = 0;
		VERIFYD3D11RESULT_EX(mD3DRHI->getDevice()->CreateQuery(&desc, mQuery.getInitReference()), mD3DRHI->getDevice());
		issueEvent();
	}

	void D3D11EventQuery::issueEvent()
	{
		mD3DRHI->getDeviceContext()->End(mQuery);
	}
}