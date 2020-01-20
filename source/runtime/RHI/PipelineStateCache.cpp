#include "PipelineStateCache.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
#include "RHIResource.h"
#include "RHICommandList.h"
namespace Air
{
#define PIPELINESTATECACHE_VERIFYTHREADSAFE 1


	static inline uint32 getTypeHash(const BoundShaderStateInput& input)
	{
		return getTypeHash(input.mVertexDeclarationRHI)
			^ getTypeHash(input.mVertexShaderRHI)
			^ getTypeHash(input.mPixelShaderRHI)
#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
			^ getTypeHash(input.mHullShaderRHI)
			^ getTypeHash(input.mDomainShaderRHI)
#endif
#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
			^ getTypeHash(input.mGeometryShaderRHI)
#endif
			;
	}

	static inline uint32 getTypeHash(const GraphicsPipelineStateInitializer& initializer)
	{
		return (getTypeHash(initializer.mBoundShaderState) | (initializer.mNumSamples << 28)) ^ ((uint32)initializer.mPrimitiveType << 24) ^ getTypeHash(initializer.mBlendState) ^ initializer.mRenderTargetsEnabled ^ getTypeHash(initializer.mRasterizerState) ^ getTypeHash(initializer.mDepthStencilState);
	}
	


	class PipelineState
	{
	public:
		PipelineState()
			:mStats(nullptr)
		{
			initStats();
		}

		virtual ~PipelineState()
		{

		}

		virtual bool isCompute() const = 0;

		GraphEventRef mCompletionEvent;

		void waitCompletion()
		{
			if (mCompletionEvent.isValid() && !mCompletionEvent->isComplete())
			{
				TaskGraphInterface::get().waitUntilTaskCompletes(mCompletionEvent);
				mCompletionEvent = nullptr;
			}
		}

		inline void addUse()
		{
			PipelineStateStats::updateStats(mStats);
		}

		void initStats() {}
		void addHit() {}

		PipelineStateStats* mStats;
	};


	class GraphicsPipelineState : public PipelineState
	{
	public:
		GraphicsPipelineState()
		{}

		virtual bool isCompute() const
		{
			return false;
		}

		TRefCountPtr<RHIGraphicsPipelineState> mRHIPipeline;

#if PIPELINESTATECACHE_VERIFYTHREADSAFE
		ThreadSafeCounter mInUseCount;
#endif

	};


	void setGraphicsPipelineState(RHICommandList& RHICmdList, const GraphicsPipelineStateInitializer& initializer, EApplyRendertargetOption applyFlags /* = EApplyRendertargetOption::CheckApply */)
	{
		GraphicsPipelineState* pipelineState = PipelineStateCache::getAndOrCreateGraphicsPipelineState(RHICmdList, initializer, applyFlags);
		if (pipelineState && (pipelineState->mRHIPipeline || !initializer.bFromPSOFileCache))
		{
#if PIPELINESTATECACHE_VERIFYTHREADSAFE
			int32 result = pipelineState->mInUseCount.increment();
			BOOST_ASSERT(result >= 1);
#endif
			BOOST_ASSERT(isInRenderingThread() || isInParallelRenderingThread());
			RHICmdList.setGraphicsPipelineState(pipelineState);
		}
	}

	static bool isAsyncCompilationAllowed(RHICommandList& RHICmdList)
	{
		return false;
	}

	template<class TMyKey, class TMyValue>
	class TSharedPipelineStateCache
	{
	private:
		TMap<TMyKey, TMyValue>& getLocalCache()
		{
			void* tlsValue = PlatformTLS::getTLSValue(mTLSSlot);
			if (tlsValue == nullptr)
			{
				PipelineStateCacheType* piplineStateCache = new PipelineStateCacheType;
				PlatformTLS::setTlsValue(mTLSSlot, (void*)(piplineStateCache));
				
				ScopeLock s(&mAllThreadsLock);
				mAllThreadsPiplineStateCache.add(piplineStateCache);
				return *piplineStateCache;
			}
			return *((PipelineStateCacheType*)tlsValue);
		}

#if PIPELINESTATECACHE_VERIFYTHREADSAFE
		struct ScopeVerifyIncrement
		{
			volatile int32& mVerifyMutex;

			ScopeVerifyIncrement(volatile int32& inVerifyMutex)
				:mVerifyMutex(inVerifyMutex)
			{
				int32 result = PlatformAtomics::interlockedIncrement(&mVerifyMutex);
				if (result <= 0)
				{
					AIR_LOG(logRHI, Fatal, TEXT("find was hit while consolidate was running"));
				}
			}

			~ScopeVerifyIncrement()
			{
				int32 result = PlatformAtomics::interLockedDecrement(&mVerifyMutex);
				if (result < 0)
				{
					AIR_LOG(LogRHI, Fatal, TEXT("find was hit while consolidate running"));
				}
			}
		};

		struct ScopeVerifyDecrement
		{
			volatile int32& mVerifyMutex;
			ScopeVerifyDecrement(volatile int32& inVerifyMutex)
				:mVerifyMutex(inVerifyMutex)
			{
				int32 result = PlatformAtomics::interLockedDecrement(&mVerifyMutex);
				if (result > 0)
				{
					AIR_LOG(logRHI, Fatal, TEXT("consolidate was hit while get/getPSO was running"));
				}
			}

			~ScopeVerifyDecrement()
			{
				int32 result = PlatformAtomics::interlockedIncrement(&mVerifyMutex);
				if (result != 0)
				{
					AIR_LOG(LogRHI, Fatal, TEXT("consolidate was hit while get/setPSO was running"));
				}
			}
		};
#endif


	public:
		typedef TMap<TMyKey, TMyValue> PipelineStateCacheType;

		bool find(const TMyKey& inKey, TMyValue& outResult)
		{
#if PIPELINESTATECACHE_VERIFYTHREADSAFE
			ScopeVerifyIncrement S(mVerifyMutex);
#endif
			auto it = mCurrentMap->find(inKey);
			if (it != mCurrentMap->end())
			{
				outResult = it->second;
				return true;
			}

			TMap<TMyKey, TMyValue>& localCache = getLocalCache();
			
			it = localCache.find(inKey);

			if (it != localCache.end())
			{
				outResult = it->second;
				return true;
			}

			it = mBackfillMap->find(inKey);
			if (it != mBackfillMap->end())
			{
				localCache.emplace(inKey, it->second);
				outResult = it->second;
				return true;
			}
			return false;
		}

		bool add(const TMyKey& inKey, const TMyValue& inValue)
		{
#if PIPELINESTATECACHE_VERIFYTHREADSAFE
			ScopeVerifyIncrement s(mVerifyMutex);
#endif
			TMap<TMyKey, TMyValue>& localCache = getLocalCache();

			BOOST_ASSERT(localCache.contains(inKey) == false);
			localCache.emplace(inKey, inValue);
			BOOST_ASSERT(localCache.contains(inKey));
			return true;
		}

	private:
		uint32 mTLSSlot;
		PipelineStateCacheType* mCurrentMap;
		PipelineStateCacheType* mBackfillMap;
		PipelineStateCacheType* mMap1;
		PipelineStateCacheType* mMap2;

		TArray<TMyValue> mDeleteArray;

		CriticalSection mAllThreadsLock;

		TArray<PipelineStateCacheType*> mAllThreadsPiplineStateCache;

		uint32 mDuplicateStateGenerated;

#if PIPELINESTATECACHE_VERIFYTHREADSAFE
		volatile int32 mVerifyMutex;
#endif
	};


	typedef TSharedPipelineStateCache<GraphicsPipelineStateInitializer, GraphicsPipelineState*> GraphicsPipelineCache;

	GraphicsPipelineCache GGraphicsPipelineCache;

	GraphicsPipelineState* PipelineStateCache::getAndOrCreateGraphicsPipelineState(RHICommandList& RHICmdList, const GraphicsPipelineStateInitializer& originalInitializer, EApplyRendertargetOption applyFlags)
	{
		{
			GraphicsPipelineStateInitializer& hashableInitializer = const_cast<GraphicsPipelineStateInitializer&>(originalInitializer);
			
			hashableInitializer.mVertexShaderHash = hashableInitializer.mBoundShaderState.mVertexShaderRHI ? hashableInitializer.mBoundShaderState.mVertexShaderRHI->getHash() : SHAHash();

#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
			hashableInitializer.mGeometryShaderHash = hashableInitializer.mBoundShaderState.mGeometryShaderRHI ? hashableInitializer.mBoundShaderState.mGeometryShaderRHI->getHash() : SHAHash();
#endif

#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
			hashableInitializer.mHullShaderHash = hashableInitializer.mBoundShaderState.mHullShaderRHI ? hashableInitializer.mBoundShaderState.mHullShaderRHI->getHash() : SHAHash();

			hashableInitializer.mDomainShaderHash = hashableInitializer.mBoundShaderState.mDomainShaderRHI ? hashableInitializer.mBoundShaderState.mDomainShaderRHI->getHash() : SHAHash();
#endif

			hashableInitializer.mPixelShaderHash = hashableInitializer.mBoundShaderState.mPixelShaderRHI ? hashableInitializer.mBoundShaderState.mPixelShaderRHI->getHash() : SHAHash();

		}

		GraphicsPipelineStateInitializer newInitializer;
		const GraphicsPipelineStateInitializer* initializer = &originalInitializer;

		BOOST_ASSERT(originalInitializer.mDepthStencilState && originalInitializer.mBlendState && originalInitializer.mRasterizerState);

		if (!!(applyFlags & EApplyRendertargetOption::ForceApply))
		{
			newInitializer = originalInitializer;
			RHICmdList.applyCachedRenderTargets(newInitializer);
			initializer = &newInitializer;
		}

		bool doAsyncCompile = isAsyncCompilationAllowed(RHICmdList);

		GraphicsPipelineState* outCachedState = nullptr;

		bool bWasFound = GGraphicsPipelineCache.find(*initializer, outCachedState);
		if (bWasFound == false)
		{
			//PipelineStateCache::cachegr
			outCachedState = new GraphicsPipelineState();
			GGraphicsPipelineCache.add(*initializer, outCachedState);
		}
		else
		{
			if (doAsyncCompile)
			{
				GraphEventRef& completionEvent = outCachedState->mCompletionEvent;
				if (completionEvent.isValid() && !completionEvent->isComplete())
				{
					RHICmdList.addDispatchPrerequisite(completionEvent);
				}
			}
		}
		return outCachedState;
	}

	RHIGraphicsPipelineState* executeSetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState)
	{
		RHIGraphicsPipelineState* rhiPipeline = graphicsPipelineState->mRHIPipeline;
		graphicsPipelineState->addUse();

#if PIPELINESTATECACHE_VERIFYTHREADSAFE
		int32 result = graphicsPipelineState->mInUseCount.decrement();
		BOOST_ASSERT(result >= 0);
#endif
		return rhiPipeline;
	}

	void PipelineStateStats::updateStats(PipelineStateStats* stats)
	{
		if (stats)
		{
			PlatformAtomics::interlockedExchange(&stats->mLastFrameUsed, GFrameCounter);
			PlatformAtomics::interlockedIncrement(&stats->mTotalBindCount);
			PlatformAtomics::interlockedCompareExchange(&stats->mFirstFrameUsed, GFrameCounter, -1);
		}
	}
}