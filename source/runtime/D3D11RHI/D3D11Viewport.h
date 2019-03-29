#pragma once
#include "RHIResource.h"
#include "RenderResource.h"
#include "D3D11Texture.h"
namespace Air
{
	class D3D11EventQuery : public RenderResource
	{
	public:
		D3D11EventQuery(class D3D11DynamicRHI* inD3DRHI):mD3DRHI(inD3DRHI)
		{}

		void issueEvent();

		void waitForCompletion();

		virtual void initDynamicRHI() override;
		virtual void releaseDynamicRHI() override;
	private:
		D3D11DynamicRHI* mD3DRHI;
		TRefCountPtr<ID3D11Query> mQuery;
	};


	class D3D11DynamicRHI;
	class D3D11Viewport : public RHIViewport
	{
	public:
		D3D11Viewport(D3D11DynamicRHI* d3dRHI, HWND hwnd, uint32 width, uint32 height, bool isFullscreen, EPixelFormat format);

		D3D11Texture2D* getBackBuffer() const { return mBackBuffer; }
		
		bool present(bool lockToVsync);

		void resize(uint32 inSizeX, uint32 inSizeY, bool inFullscreen, EPixelFormat preferredPixelFormat);

	private:
		void presentWithVsyncDWM();

		bool presentChecked(int32 syncInterval);

		DXGI_MODE_DESC	setupDXGI_MODE_DESC();

		void conditionalResetSwapChain(bool bIgnoreFocus);
	private:
		TRefCountPtr<D3D11Texture2D> mBackBuffer;

		TRefCountPtr<IDXGISwapChain> mSwapChain;

		TRefCountPtr<IDXGIOutput>	mForcedFullscreenOutput;
		CustomPresentRHIRef	mCustomPresent;

		D3D11DynamicRHI* mD3DRHI;
		HWND mWindowHandle;

		uint32 mMaximumFrameLatency;
		uint64 mLastFlipTime;

		uint64 mLastCompleteTime;
		uint64 mLastFrameComplete;
		int32 mSyncCounter;

		bool mIsValid;
		bool bSyncedLastFrame;
		bool mForcedFullscreenDisplay;


		uint32 mSizeX{ 0 };
		uint32 mSizeY{ 0 };
		bool mIsFullscreen{ false };

		EPixelFormat mPixelFormat{ EPixelFormat::PF_Unknown };

		D3D11EventQuery mFrameSyncEvent;
	};

	template<>
	struct TD3D11ResourceTraits<RHIViewport>
	{
		typedef D3D11Viewport TConcreteType;
	};
}