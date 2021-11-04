#pragma once
#include "D3D11Typedefs.h"
#include "D3D11Resource.h"
#ifdef D3D11RHI_EXPORTS
#define D3D11RHI_API	DLLEXPORT
#else
#define D3D11RHI_API	DLLIMPORT
#endif


namespace Air
{
	




	struct D3D11Adapter
	{
		int32 mAdapterIndex;
		D3D_FEATURE_LEVEL mMaxSupportedFeatureLevel;
		D3D11Adapter(int32 InAdapterIndex = -1, D3D_FEATURE_LEVEL InMaxSupportedFeatureLevel = (D3D_FEATURE_LEVEL)0)
			:mAdapterIndex(InAdapterIndex)
			, mMaxSupportedFeatureLevel(InMaxSupportedFeatureLevel)
		{

		}

		bool isValid() const
		{
			return mMaxSupportedFeatureLevel != (D3D_FEATURE_LEVEL)0 && mAdapterIndex >= 0;
		}
	};

	struct D3D11GlobalStats
	{
		static int64 GDedicatedVideoMemory;
		static int64 GDedicatedSystemMemory;
		static int64 GSharedSystemMemory;

		static int64 GTotalGraphicsMemory;
	};


	class D3D11DynamicRHIModule : public IDynamicRHIModule
	{
		virtual bool isSupported() override;

		virtual DynamicRHI* createRHI(ERHIFeatureLevel::Type requestedFeatureLevel = ERHIFeatureLevel::Num) override;

	private:
		D3D11Adapter mChosenAdapter;
		DXGI_ADAPTER_DESC mChosenDescription;
		void findAdapter();
	};

	inline DXGI_FORMAT findUnorderedAccessDXGIFormat(DXGI_FORMAT inFormat)
	{
		switch (inFormat)
		{
		case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		return inFormat;
	}

	inline DXGI_FORMAT findShaderResourceDXGIFormat(DXGI_FORMAT inFormat, bool bSRGB)
	{
		if (bSRGB)
		{
			switch (inFormat)
			{
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case DXGI_FORMAT_BC1_TYPELESS:
				return DXGI_FORMAT_BC1_UNORM_SRGB;
			case DXGI_FORMAT_BC2_TYPELESS:
				return DXGI_FORMAT_BC2_UNORM_SRGB;
			case DXGI_FORMAT_BC3_TYPELESS:
				return DXGI_FORMAT_BC3_UNORM_SRGB;
			case DXGI_FORMAT_BC7_TYPELESS:
				return DXGI_FORMAT_BC7_UNORM_SRGB;
			};
		}
		else
		{
			switch (inFormat)
			{
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_BC1_TYPELESS:
				return DXGI_FORMAT_BC1_UNORM;
			case DXGI_FORMAT_BC2_TYPELESS:
				return DXGI_FORMAT_BC2_UNORM;
			case DXGI_FORMAT_BC3_TYPELESS:
				return DXGI_FORMAT_BC3_UNORM;
			case DXGI_FORMAT_BC7_TYPELESS:
				return DXGI_FORMAT_BC7_UNORM;
			};
		}
		switch (inFormat)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_R32_TYPELESS:
			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_R16_UNORM;
		}
		return inFormat;
	}

	class FastVRAMAllocator
	{
	public:
		FastVRAMAllocator() {}
		virtual ~FastVRAMAllocator() {}
		virtual VRamAllocation allocTexture2D(D3D11_TEXTURE2D_DESC& textureDesc)
		{
			return VRamAllocation();
		}

		virtual VRamAllocation allocTexture3D(D3D11_TEXTURE3D_DESC& textureDesc)
		{
			return VRamAllocation();
		}

		virtual VRamAllocation allocUAVBuffer(D3D11_BUFFER_DESC& bufferDesc)
		{
			return VRamAllocation();
		}

		template<typename t_A, typename t_B>
		static t_A roundUpToNextMultiple(const t_A& a, const t_B& b)
		{
			return ((a - 1) / b + 1) * b;
		}

		static FastVRAMAllocator* getFastVRAMAllocator();
	};

	inline DXGI_FORMAT findDepthStencilDXGIFormat(DXGI_FORMAT inFormat)
	{
		switch (inFormat)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R32_TYPELESS:
			return DXGI_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_D16_UNORM;
		}
		return inFormat;
	}

	inline bool hasStencilBits(DXGI_FORMAT inFormat)
	{
		switch (inFormat)
		{
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return true;
		}
		return false;
	}

	enum {MAX_CONSTANT_BUFFERS_PER_SHADER_STAGE = 14};
}