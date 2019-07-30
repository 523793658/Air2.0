#pragma once
#include "RenderCore.h"
#include "Containers/LinkList.h"
#include "RHIResource.h"
#include "HAL/CriticalSection.h"
#include "RHI.h"
#include <limits>
namespace Air
{

	class RENDER_CORE_API RenderResource
	{
	public:
		static TLinkedList<RenderResource*>*& getResourceList();

		virtual void initRHI() {}

		virtual void initDynamicRHI() {};

		virtual void releaseResource();

		void updateRHI();

		virtual void initResource();

		virtual void releaseRHI() {}

		virtual void releaseDynamicRHI() {};

		virtual wstring getFriendlyName()const { return TEXT(""); };

		FORCEINLINE bool isInitialized() const { return mInitialized; }

		void initResourceFromPossiblyParallelRendering();

	protected:
		ERHIFeatureLevel::Type getFeatureLevel() const { return mFeatureLevel == ERHIFeatureLevel::Num ? GMaxRHIFeatureLevel : mFeatureLevel; }

		FORCEINLINE bool hasValidFeatureLevel()const { return mFeatureLevel < ERHIFeatureLevel::Num; }

	protected:
		bool mInitialized{ false };

		ERHIFeatureLevel::Type mFeatureLevel;

		TLinkedList<RenderResource*> mResourceLink;

	};

	template<class ResourceType>
	class TGlobalResource : public ResourceType
	{
	public:
		TGlobalResource()
		{
			initGlobalResource(); 
		}
		template<typename T1>
		explicit TGlobalResource(T1 Param1)
			:ResourceType(Param1)
		{
			initGlobalResource();
		}

		template<typename T1, typename T2>
		explicit TGlobalResource(T1 Param1, T2 Param2)
			:ResourceType(Param1, Param2)
		{
			initGlobalResource();
		}

		virtual ~TGlobalResource()
		{
			releaseGlobalResource();
		}


	private:
		void initGlobalResource()
		{
			if (isInRenderingThread())
			{
				((ResourceType*)this)->initResource();
			}
			else
			{
				beginInitResource((ResourceType*)this);
			}
		}

		void releaseGlobalResource()
		{
			((ResourceType*)this)->releaseResource();
		}
	};

	class IndexBuffer : public RenderResource
	{
	public:
		IndexBufferRHIRef	mIndexBufferRHI;
		virtual ~IndexBuffer() {}

		virtual void releaseRHI() override
		{
			mIndexBufferRHI.safeRelease();
		}

		virtual wstring getFriendlyName() const override { return TEXT("IndexBuffer"); }


	};

	class RENDER_CORE_API VertexBuffer : public RenderResource
	{
	public:
		VertexBufferRHIRef	mVertexBufferRHI;
		virtual ~VertexBuffer(){}
		virtual void releaseRHI() override
		{
			mVertexBufferRHI.safeRelease();
		}
		virtual wstring getFriendlyName() const override { return TEXT("VertexBuffer"); }
	};


	class NullColorVertexBuffer : public VertexBuffer
	{
	public:
		virtual void initRHI() override;


		virtual void releaseRHI() override
		{
			mVertexBufferSRV.safeRelease();
			VertexBuffer::releaseRHI();
		}

		ShaderResourceViewRHIRef mVertexBufferSRV;
	};

	extern RENDER_CORE_API TGlobalResource<NullColorVertexBuffer> GNullColorVertexBuffer;

	template<uint32 Size, bool TThreadSafe = true>
	class TBoundShaderStateHistory : public RenderResource
	{
	public:
		TBoundShaderStateHistory():
			mNextBoundShaderStateIndex(0)
		{}
		FORCEINLINE void add(BoundShaderStateRHIRef boundShaderState)
		{
			if (TThreadSafe && GRHISupportsParallelRHIExecute)
			{
				mBoundShaderStateHistoryLock.lock();
			}
			mBoundShaderStates[mNextBoundShaderStateIndex] = boundShaderState;
			mNextBoundShaderStateIndex = (mNextBoundShaderStateIndex + 1) % Size;
			if (TThreadSafe && GRHISupportsParallelRHIExecute)
			{
				mBoundShaderStateHistoryLock.unlock();
			}
		}

		BoundShaderStateRHIParamRef getLast()
		{
			BOOST_ASSERT(!GRHISupportsParallelRHIExecute);
			uint32 lastIndex = mNextBoundShaderStateIndex == 0? Size - 1: mNextBoundShaderStateIndex - 1;
			return mBoundShaderStates[lastIndex];
		}

		virtual void releaseRHI()
		{
			if (TThreadSafe && GRHISupportsParallelRHIExecute)
			{
				mBoundShaderStateHistoryLock.lock();
			}
			for (uint32 index = 0; index < Size; index++)
			{
				mBoundShaderStates[index].safeRelease();
			}
			if (TThreadSafe && GRHISupportsParallelRHIExecute)
			{
				mBoundShaderStateHistoryLock.unlock();
			}
		}



	private:
		BoundShaderStateRHIRef mBoundShaderStates[Size];
		uint32 mNextBoundShaderStateIndex;
		CriticalSection mBoundShaderStateHistoryLock;
	};

	enum EMipFadeSettings
	{
		MipFade_Normal = 0,
		MipFade_Slow,
		MipFade_NumSettings
	};
	
	extern RENDER_CORE_API float GEnableMipLevelFading;

	struct MipBaseFade 
	{
		MipBaseFade()
		{}

		float mTotalMipCount{ 0.0f };
		float mMipCountDelta{ 0.0f };
		float mStartTime{ 0.0f };
		float mMipCountFadingRate{ 0.0f }; 
		float mBiasOffset{ 0.0f };

		RENDER_CORE_API void setNewMipCount(float actualMipCount, float targetMipCount, double lastRenderTime, EMipFadeSettings fadeSetting);

		inline float calcMipBias() const
		{
			float deltaTime = GRenderingRealtimeClock.getCurrentTime() - mStartTime;
			float timeFactor = Math::min<float>(deltaTime * mMipCountFadingRate, 1.0f);
			float mipBias = mBiasOffset - mMipCountDelta * timeFactor;
			return Math::floatSelect(GEnableMipLevelFading, mipBias, 0.0f);
		}

		inline bool isFading() const {
			float deltaTime = GRenderingRealtimeClock.getCurrentTime() - mStartTime;
			float timeFactor = deltaTime * mMipCountFadingRate;
			return (Math::abs<float>(mMipCountDelta) > SMALL_NUMBER && timeFactor < 1.0f);
		}
	
	};


	class Texture : public RenderResource
	{
	public:
		TextureRHIRef	mTextureRHI{ nullptr };
		SamplerStateRHIRef	mSamplerStateRHI{ nullptr };
		SamplerStateRHIRef	mDeferredPassSamplerStateRHI{ nullptr };
		mutable double		mLastRenderTime{ -1.0f };
		MipBaseFade			mMipBiasFade;
		bool				bGreyScaleFormat{ false };
		bool				bIgnoreGammaConversions{ false };
		bool				bSRGB{ false };
		virtual ~Texture() {}

		virtual uint32 getWidth() const
		{
			return 0;
		}
		virtual uint32 getHeight() const
		{
			return 0;
		}

		virtual void releaseRHI() override
		{
			mTextureRHI.safeRelease();
			mSamplerStateRHI.safeRelease();
			mDeferredPassSamplerStateRHI.safeRelease();
		}

		virtual wstring getFriendlyName() const override { return TEXT("Texture"); }
	};

	class RENDER_CORE_API TextureReference : public RenderResource
	{
	public:
		void beginInit_RenderThread();

		void beginRelease_GameThread();

		virtual void initRHI() override;

		virtual void releaseRHI() override;
	public:
		TextureReferenceRHIRef mTextureReferenceRHI;
		
	private:
		bool bInitialized_GameThread{ false };

		LastRenderTimeContainer mLastRenderTimeRHI;
	};


	extern RENDER_CORE_API void beginInitResource(RenderResource* resource);

	extern RENDER_CORE_API void beginReleaseResource(RenderResource* resource);

	extern RENDER_CORE_API void releaseResourceAndFlush(RenderResource* resource);



}