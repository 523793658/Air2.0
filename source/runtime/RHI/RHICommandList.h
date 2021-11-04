#pragma once
#include "RHIConfig.h"
#include "Async/TaskGraphInterfaces.h"
#include "boost/noncopyable.hpp"
#include "Template/AlignOf.h"
#include "Misc/MemStack.h"
#include "Math/Float16Color.h"
#include "Containers/StaticArray.h"
#include "Containers/ArrayView.h"
#include "Template/RefCounting.h"
#include "RHIResource.h"
#include "RHIContext.h"
#include "DynamicRHI.h"
namespace Air
{
	class IRHICommandContext;
	class IRHIComputeContext;
	class GraphicsPipelineState;
	class RHIGraphicsPipelineState;
	struct RHICommandBase;

	enum class ECmdList
	{
		EGfx,
		ECompute,
	};
	class RHICommandListBase;

	struct RHICommandListDebugContext
	{};

	struct RHICommandBase
	{
		RHICommandBase* mNext = nullptr;

		virtual void executeAndDestruct(RHICommandListBase& cmdList, RHICommandListDebugContext& debugContext) = 0;

	};
	extern RHI_API RHIGraphicsPipelineState* executeSetGraphicsPipelineState(GraphicsPipelineState*);

	StructuredBufferRHIRef RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo);

	FORCEINLINE Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo);


	class RHI_API RHICommandListBase : public boost::noncopyable
	{
	public:
		void* operator new(size_t size);
		void operator delete(void* rawMemory);

		mutable uint32 StrictGraphicsPipelineStateUse;
		RHICommandListBase();
		~RHICommandListBase();

		struct DrawUpData 
		{
			uint32 mPrimitiveType;
			uint32 mNumPrimitives;
			uint32 mNumVertices;
			uint32 mVertexDataStride;
			void* mOutVertexData;
			uint32 mMinVertexIndex;
			uint32 mNumIndices;
			uint32 mIndexDataStride;
			void* mOutIndexData;
			DrawUpData()
				:mPrimitiveType(PT_Num)
				, mOutVertexData(nullptr)
				, mOutIndexData(nullptr)
			{}
		};

		inline void flush();

		inline bool isImmediate();

		inline bool isImmediateAsyncCompute();

		inline bool bypass();

		inline bool isInsideRenderPass() const
		{
			return mData.bInsideRenderPass;
		}

		inline bool isOutsideRenderPass() const
		{
			return !mData.bInsideRenderPass;
		}

		inline bool isInsideComputePass() const
		{
			return mData.bInsideComputePass;
		}

		void resetSubpass(ESubpassHint subpassHint)
		{
			mPSOContext.mSubpassHint = subpassHint;
			mPSOContext.mSubpassIndex = 0;
		}

		void reset();

		void waitForDispatch();

		void waitForRHIThreadTasks();

		void waitForTasks(bool bKnownToBeComplete = false);

		const int32 getUsedMemory() const;

		FORCEINLINE bool isExecuting() const
		{
			return mExecuting;
		}

		void setContext(IRHICommandContext* inContext)
		{
			mContext = inContext;
		}

		void setComputeContext(IRHIComputeContext* inContext)
		{
			mComputeContext = inContext;
		}

		FORCEINLINE bool hasCommands() const
		{
			return (mNumCommands > 0);
		}

		FORCEINLINE IRHICommandContext& getContext()
		{
			return *mContext;
		}

		IRHIComputeContext& getComputeContext()
		{
			return *mComputeContext;
		}

		FORCEINLINE_DEBUGGABLE void* alloc(int32 allocSize, int32 alignment)
		{
			return mMemManager.alloc(allocSize, alignment);
		}

		void addDispatchPrerequisite(const GraphEventRef& prereq);

		template<typename T>
		FORCEINLINE_DEBUGGABLE void* alloc()
		{
			return alloc(sizeof(T), ALIGNOF(T));
		}

		FORCEINLINE_DEBUGGABLE TCHAR* allocString(const TCHAR* name)
		{
			int32 len = CString::strlen(name) + 1;
			TCHAR* nameCopy = (TCHAR*)alloc(len * (int32)sizeof(TCHAR), (int32)sizeof(TCHAR));
			CString::strcpy(nameCopy, len, name);
			return nameCopy;
		}

		FORCEINLINE_DEBUGGABLE void* allocCommand(int32 allocSize, int32 alignment)
		{
			BOOST_ASSERT(!isExecuting());
			RHICommandBase* result = (RHICommandBase*)mMemManager.alloc(allocSize, alignment);
			++mNumCommands;
			*mCommandLink = result;
			mCommandLink = &result->mNext;
			return result;
		}

		template<typename TCmd>
		FORCEINLINE_DEBUGGABLE void* allocCommand()
		{
			return allocCommand(sizeof(TCmd), alignof(TCmd));
		}

#define ALLOC_COMMAND(...) new(allocCommand(sizeof(__VA_ARGS__), alignof(__VA_ARGS__)) ) __VA_ARGS__


		struct PSOContext
		{
			uint32 mCachedNumSimultanousRenderTargets = 0;
			TStaticArray<RHIRenderTargetView, MaxSimultaneousRenderTargets> mCachedRenderTargets;
			RHIDepthRenderTargetView mCachedDepthStencilTarget;
			ESubpassHint mSubpassHint = ESubpassHint::None;
			uint8 mSubpassIndex = 0;
		}mPSOContext;

		void cacheActiveRenderTargets(uint32 newNumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargetsRHI, const RHIDepthRenderTargetView* newDepthStencilTargetRHI)
		{
			mPSOContext.mCachedNumSimultanousRenderTargets = newNumSimultaneousRenderTargets;
			for (uint32 RTIdx = 0; RTIdx < mPSOContext.mCachedNumSimultanousRenderTargets; ++RTIdx)
			{
				mPSOContext.mCachedRenderTargets[RTIdx] = newRenderTargetsRHI[RTIdx];
			}
			mPSOContext.mCachedDepthStencilTarget = (newDepthStencilTargetRHI) ? *newDepthStencilTargetRHI : RHIDepthRenderTargetView();
		}

		void cacheActiveRenderTargets(const RHIRenderPassInfo& info)
		{
			RHISetRenderTargetsInfo RTInfo;
			info.convertToRenderTargetsInfo(RTInfo);
			cacheActiveRenderTargets(RTInfo.mNumColorRenderTargets, RTInfo.mColorRenderTarget, &RTInfo.mDepthStencilRenderTarget);
		}

		FORCEINLINE_DEBUGGABLE void exchangeCmdList(RHICommandListBase& other)
		{
			BOOST_ASSERT(!mRTTasks.size() && !other.mRTTasks.size());
			Memory::memswap(this, &other, sizeof(RHICommandListBase));
			if (mCommandLink == &other.mRoot)
			{
				mCommandLink = &mRoot;
			}
			if (other.mCommandLink == &mRoot)
			{
				other.mCommandLink = &other.mRoot;
			}
		}

		void copyRenderThreadContexts(const RHICommandListBase& parentCommandList)
		{
			for (int32 index = 0; ERenderThreadContext(index) < ERenderThreadContext::Num; index++)
			{
				mRenderThreadContext[index] = parentCommandList.mRenderThreadContext[index];
			}
		}

		void queueRenderThreadCommandListSubmit(GraphEventRef& renderThreadCompletionEvent, class RHICommandList* cmdList);

		void FORCEINLINE flushStateCache()
		{
			mCachedRasterizerState = nullptr;
			mCachedDepthStencilState = nullptr;
		}

		void handleRTThreadTaskCompletion(const GraphEventRef& myComplitionGraphEvent);

	public:
		enum class ERenderThreadContext
		{
			SceneRenderTargets,
			Num
		};


		FORCEINLINE void* getRenderThreadContext(ERenderThreadContext slot)
		{
			return mRenderThreadContext[int32(slot)];
		}

		void copyContext(RHICommandListBase& parentCommandList)
		{
			mContext = parentCommandList.mContext;
			mComputeContext = parentCommandList.mComputeContext;
		}

		void *mRenderThreadContext[(int32)ERenderThreadContext::Num];


		FORCEINLINE bool isAsyncCompute() const
		{
			return mContext == nullptr && mComputeContext != nullptr;
		}

		FORCEINLINE ERHIPipeline getPipeline() const
		{
			return isAsyncCompute() ? ERHIPipeline::AsyncCompute : ERHIPipeline::Graphics;
		}
	protected:
		uint32 mNumCommands{ 0 };
		RHICommandBase* mRoot;
		RHICommandBase** mCommandLink;

		uint32 mUID;
		GraphEventArray mRTTasks;

		MemStackBase mMemManager;

		IRHICommandContext* mContext{ nullptr };
		IRHIComputeContext* mComputeContext{ nullptr };
		
		bool mExecuting{ false };

	

		friend class RHICommandListExecutor;
		friend class RHICommandListIterator;


	protected:
		static int32 mStateCacheEnabled;

		struct RHICommandSetDepthStencilState* mCachedDepthStencilState;
		struct RHICommandSetRasterizerState* mCachedRasterizerState;
		DrawUpData mDrawUPData;

	public:

		struct CommonData
		{
			class RHICommandListBase* mParent = nullptr;

			enum class ECmdListType
			{
				Immediate = 1,
				Regular,
			};

			ECmdListType mType = ECmdListType::Regular;
			bool bInsideRenderPass = false;
			bool bInsideComputePass = false;
		};

		CommonData mData;
	};

	struct UnnameedRHICommand
	{
		static const TCHAR* TStr() { return TEXT("UnnameedRHICommand"); }
	};

	template <typename TCmd, typename NameType = UnnameedRHICommand>
	struct RHICommand : public RHICommandBase
	{
		void executeAndDestruct(RHICommandListBase& cmdList, RHICommandListDebugContext& context) override final
		{
			TCmd* thisCmd = static_cast<TCmd*>(this);
			thisCmd->execute(cmdList);
			thisCmd->~TCmd();
		}
	};

	

	struct RHICommandSetStencilRef : public RHICommand<RHICommandSetStencilRef>
	{
		uint32 mStencilRef;
		FORCEINLINE_DEBUGGABLE RHICommandSetStencilRef(uint32 inStencilRef)
			:mStencilRef(inStencilRef)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetViewport : public RHICommand<RHICommandSetViewport>
	{
		uint32 mX;
		uint32 mY;
		float mZ;
		uint32 mWidth;
		uint32 mHeight;
		float mDepth;
		FORCEINLINE_DEBUGGABLE RHICommandSetViewport(uint32 x, uint32 y, float z, uint32 width, uint32 height, float depth)
			:mX(x), mY(y),mZ(z), mWidth(width),mHeight(height),mDepth(depth)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetStereoViewport final : public RHICommand< RHICommandSetStereoViewport>
	{
		uint32 mLeftMinX;
		uint32 mRightMinX;
		uint32 mLeftMinY;
		uint32 mRightMinY;

		float mMinZ;
		uint32 mLeftMaxX;
		uint32 mRightMaxX;
		uint32 mLeftMaxY;
		uint32 mRightMaxY;
		float mMaxZ;

		FORCEINLINE_DEBUGGABLE RHICommandSetStereoViewport(uint32 inLeftMinX, uint32 inRightMinX, uint32 inLeftMinY, uint32 inRightMinY, float inMinZ, uint32 inLeftMaxX, uint32 inRightMaxX, uint32 inLeftMaxY, uint32 inRightMaxY, float inMaxZ)
			:mLeftMinX(inLeftMinX)
			,mRightMinX(inRightMinX)
			,mLeftMinY(inLeftMinY)
			,mRightMinY(inRightMinY)
			,mMinZ(inMinZ)
			,mLeftMaxX(inLeftMaxX)
			,mRightMaxX(inRightMaxX)
			,mLeftMaxY(inLeftMaxY)
			,mRightMaxY(inRightMaxY)
			,mMaxZ(inMaxZ)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};
	

	struct RHICommandBeginDrawingViewport : public RHICommand<RHICommandBeginDrawingViewport>
	{
		RHIViewport* mViewport;
		RHITexture*	mRenderTargetRHI;
		FORCEINLINE_DEBUGGABLE RHICommandBeginDrawingViewport(RHIViewport* inViewport, RHITexture* inRenderTargetRHI)
			:mViewport(inViewport),
			mRenderTargetRHI(inRenderTargetRHI)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandEndDrawingViewport : public RHICommand<RHICommandEndDrawingViewport>
	{
		RHIViewport* viewport;
		bool bPresent;
		bool bLockToVsync;

		FORCEINLINE_DEBUGGABLE RHICommandEndDrawingViewport(RHIViewport* inViewport, bool inPresent, bool inbLockToVsync)
			:viewport(inViewport),
			bPresent(inPresent),
			bLockToVsync(inbLockToVsync)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};
	struct RHICommandBeginScene : public RHICommand<RHICommandBeginScene>
	{
		FORCEINLINE_DEBUGGABLE RHICommandBeginScene()
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandEndScene : public RHICommand<RHICommandEndScene>
	{
		FORCEINLINE_DEBUGGABLE RHICommandEndScene()
		{}
		RHI_API void execute(RHICommandListBase& CmdList);
	};

	struct RHICommandBeginFrame : public RHICommand<RHICommandBeginFrame>
	{
		FORCEINLINE_DEBUGGABLE RHICommandBeginFrame()
		{

		}
		RHI_API void execute(RHICommandListBase& CmdList);
	};

	struct RHICommandEndFrame : public RHICommand<RHICommandEndFrame>
	{
		FORCEINLINE_DEBUGGABLE RHICommandEndFrame()
		{

		}
		RHI_API void execute(RHICommandListBase& CmdList);
	};



	struct ComputedConstantBuffer
	{
		ConstantBufferRHIRef mConstantBuffer;
		mutable int32 mUseCount;
		ComputedConstantBuffer()
			:mUseCount(0)
		{}
	};


	struct LocalConstantBufferWorkArea
	{
		void* mContents;
		const RHIConstantBufferLayout* mLayout;
		ComputedConstantBuffer* mComputedConstantBuffer;
		LocalConstantBufferWorkArea(RHICommandListBase* inCheckCmdList, const void* inContents, uint32 contentsSize, const RHIConstantBufferLayout* inLayout)
			:mLayout(inLayout)
		{
			BOOST_ASSERT(contentsSize);
			mContents = inCheckCmdList->alloc(contentsSize, CONSTANT_BUFFER_STRUCT_ALIGNMENT);
			Memory::memcpy(mContents, inContents, contentsSize);
			mComputedConstantBuffer = new (inCheckCmdList->alloc<ComputedConstantBuffer>()) ComputedConstantBuffer;
		}
	};

	struct RHICommandBuildLocalConstantBuffer : public RHICommand<RHICommandBuildLocalConstantBuffer>
	{
		LocalConstantBufferWorkArea mWorkArea;
		FORCEINLINE_DEBUGGABLE RHICommandBuildLocalConstantBuffer(RHICommandListBase* checkCmdList,
			const void* contents,
			uint32 contentSize,
			const RHIConstantBufferLayout& layout)
			: mWorkArea(checkCmdList, contents, contentSize, &layout)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);

	};

	struct RHICommandSetRenderTargetsAndClear : public RHICommand<RHICommandSetRenderTargetsAndClear>
	{
		RHISetRenderTargetsInfo mRenderTargetInfo;
		FORCEINLINE_DEBUGGABLE RHICommandSetRenderTargetsAndClear(const RHISetRenderTargetsInfo& inRenderTargetInfo)
			:mRenderTargetInfo(inRenderTargetInfo)
		{}
		RHI_API void execute(RHICommandListBase& CmdList);
	};

	struct RHICommandSetRenderTargets : public RHICommand<RHICommandSetRenderTargets>
	{
		uint32 mNewNumSimultaneousRenderTargets;
		RHIRenderTargetView mNewRenderTargetsRHI[MaxSimultaneousRenderTargets];
		RHIDepthRenderTargetView mNewDepthStencilTarget;
		uint32 mNumUAVs;
		RHIUnorderedAccessView* mUAVs[MaxSimultaneousUAVs];

		FORCEINLINE_DEBUGGABLE RHICommandSetRenderTargets(uint32 inNumSimultaneousRenderTargets, const RHIRenderTargetView* inNewRenderTargetsRHI, const RHIDepthRenderTargetView * inNewDepthStencilTarget, uint32 inNumUVAs, RHIUnorderedAccessView* const* inUAVs)
			:mNewNumSimultaneousRenderTargets(inNumSimultaneousRenderTargets)
			,mNumUAVs(inNumUVAs)
		{
			BOOST_ASSERT(inNumSimultaneousRenderTargets <= MaxSimultaneousRenderTargets && inNumUVAs <= MaxSimultaneousUAVs);
			for (uint32 index = 0; index < mNewNumSimultaneousRenderTargets; index++)
			{
				mNewRenderTargetsRHI[index] = inNewRenderTargetsRHI[index];
			}
			for (uint32 index = 0; index < mNumUAVs; index++)
			{
				mUAVs[index] = inUAVs[index];
			}
			if (inNewDepthStencilTarget)
			{
				mNewDepthStencilTarget = *inNewDepthStencilTarget;
			}
		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	

	struct RHICommandSetScissorRect : public RHICommand<RHICommandSetScissorRect>
	{
		bool bEnable;
		uint32 mMinX;
		uint32 mMinY;
		uint32 mMaxX;
		uint32 mMaxY;

		FORCEINLINE_DEBUGGABLE RHICommandSetScissorRect(bool inbEnable, uint32 inMinX, uint32 inMinY, uint32 inMaxX, uint32 inMaxY)
			:bEnable(inbEnable)
			,mMinX(inMinX)
			,mMinY(inMinY)
			,mMaxX(inMaxX)
			,mMaxY(inMaxY)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<ECmdList CmdListType>
	struct RHICommandSetComputeShader final : RHICommand<RHICommandSetComputeShader<CmdListType>>
	{
		RHIComputeShader* mShader;
		FORCEINLINE_DEBUGGABLE RHICommandSetComputeShader(RHIComputeShader* shader)
			:mShader(shader)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<ECmdList CmdListType>
	struct RHICommandDispatchComputeShader final : public RHICommand<RHICommandDispatchComputeShader<CmdListType>>
	{
		uint32 mThreadGroupCountX;
		uint32 mThreadGroupCountY;
		uint32 mThreadGroupCountZ;

		FORCEINLINE_DEBUGGABLE RHICommandDispatchComputeShader(uint32 inThreadGroupCountX, uint32 inThreadGroupCountY, uint32 inThreadGroupCountZ)
			:mThreadGroupCountX(inThreadGroupCountX)
			,mThreadGroupCountY(inThreadGroupCountY)
			,mThreadGroupCountZ(inThreadGroupCountZ)

		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TRHIShader, ECmdList CmdListType>
	struct RHICommandSetShaderResourceViewParameter final : public RHICommand<RHICommandSetShaderResourceViewParameter<TRHIShader, CmdListType>>
	{
		TRHIShader* mShader;
		uint32 mSamplerIndex;
		RHIShaderResourceView* mSRV;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderResourceViewParameter(TRHIShader* inShader, uint32 inSamplerIndex, RHIShaderResourceView* inSRV)
			:mShader(inShader)
			, mSamplerIndex(inSamplerIndex)
			, mSRV(inSRV)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandBindClearMRTValues : public RHICommand<RHICommandBindClearMRTValues>
	{
		bool bClearColor;
		bool bClearDepth;
		bool bClearStencil;

		FORCEINLINE_DEBUGGABLE RHICommandBindClearMRTValues(bool inbClearColor, bool inbClearDepth, bool inbClearStencil)
			:bClearColor(inbClearColor),
			bClearDepth(inbClearDepth),
			bClearStencil(inbClearStencil)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	

	struct RHICommandSetStreamSource : public RHICommand<RHICommandSetStreamSource> 
	{
		uint32 mStreamIndex;
		RHIVertexBuffer* mVertexBuffer;
		uint32 mOffset;
		FORCEINLINE_DEBUGGABLE RHICommandSetStreamSource(
			uint32 inStreamIndex,
			RHIVertexBuffer* inVertexBuffer,
			uint32 inOffset)
			:mStreamIndex(inStreamIndex),
			mVertexBuffer(inVertexBuffer),
			mOffset(inOffset)
		{

		}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandAutomaticCacheFlushAfterComputeShader final : public RHICommand< RHICommandAutomaticCacheFlushAfterComputeShader>
	{
		bool bEnable;
		FORCEINLINE_DEBUGGABLE RHICommandAutomaticCacheFlushAfterComputeShader(bool inbEnable)
			:bEnable(inbEnable)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandFlushComputeShaderCache final : public RHICommand< RHICommandFlushComputeShaderCache>
	{
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	namespace EImmediateFlushType
	{
		enum Type
		{
			WaitForOutstandingTasksOnly = 0,
			DispatchToRHIThread,
			WaitForDispatchToRHIThread,
			FlushRHIThread,
			FlushRHIThreadFlushResources
		};
	}
	struct RHICommandBase;

	struct LocalConstantBuffer
	{
		LocalConstantBufferWorkArea* mWorkArea;
		ConstantBufferRHIRef mBypassConstant;
		LocalConstantBuffer()
			:mWorkArea(nullptr)
		{}
		LocalConstantBuffer(const LocalConstantBuffer& other)
			:mWorkArea(other.mWorkArea)
			, mBypassConstant(other.mBypassConstant)
		{

		}
		FORCEINLINE_DEBUGGABLE bool isValid() const
		{
			return mWorkArea || isValidRef(mBypassConstant);
		}
	};

	struct ComputedBSS
	{
		BoundShaderStateRHIRef mBSS;
		int32 mUseCount;
		ComputedBSS()
			:mUseCount(0)
		{}
	};




	struct LocalGraphicsPipelineState
	{
		GraphicsPipelineStateInitializer mArgs;
		LocalGraphicsPipelineState(
			RHICommandListBase* inCheckCmdList, const GraphicsPipelineStateInitializer& initializer
		) :mArgs(initializer)
		{

		}
	};

	
	
	struct RHICommandSetBoundShaderState : public RHICommand<RHICommandSetBoundShaderState>
	{
		RHIBoundShaderState* mBoundShaderState;
		FORCEINLINE_DEBUGGABLE RHICommandSetBoundShaderState(RHIBoundShaderState* inBoundShaderState)
			:mBoundShaderState(inBoundShaderState)
		{}
		RHI_API void execute(RHICommandListBase & cmdList);
	};

	template<typename TRHIShader, ECmdList cmdListType>
	struct RHICommandSetShaderConstantBuffer : public RHICommand<RHICommandSetShaderConstantBuffer<TRHIShader, cmdListType>>
	{
		TRHIShader* mShader;
		uint32 mBaseIndex;
		RHIConstantBuffer* mConstantBuffer;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderConstantBuffer(TRHIShader* inShader, uint32 inBaseIndex, RHIConstantBuffer* inConstantBuffer)
			:mShader(inShader),
			mBaseIndex(inBaseIndex),
			mConstantBuffer(inConstantBuffer)
		{
		}
		RHI_API void execute(RHICommandListBase & cmdList);
	};


	template<typename TRHIShader>
	struct RHICommandSetLocalConstantBuffer : public RHICommand<RHICommandSetLocalConstantBuffer<TRHIShader>>
	{
		TRHIShader* mShader;
		uint32 mBaseIndex;
		LocalConstantBuffer mLocalConstantBuffer;
		FORCEINLINE_DEBUGGABLE RHICommandSetLocalConstantBuffer(RHICommandListBase* checkCmdList, TRHIShader* inShader, uint32 inBaseIndex, const LocalConstantBuffer& inLocalConstantBuffer)
			:mShader(inShader),
			mBaseIndex(inBaseIndex),
			mLocalConstantBuffer(inLocalConstantBuffer)
		{
			mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->mUseCount++;
		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	

	struct RHICommandDrawIndexedPrimitive : public RHICommand<RHICommandDrawIndexedPrimitive>
	{
		RHIIndexBuffer* mIndexBuffer;
		uint32	mPrimitiveType;
		uint32	mBaseVertexIndex;
		uint32	mFirstInstance;
		uint32	mNumVertex;
		uint32	mStartIndex;
		uint32	mNumPrimitives;
		uint32	mNumInstances;
		RHICommandDrawIndexedPrimitive(RHIIndexBuffer* inIndexBuffer, uint32 inPrimitiveType, uint32 inBaseVertexIndex, uint32 inFirstInstance, uint32 inNumVertex, uint32 inStartIndex, uint32 inNumPrimitive, uint32 inNumInstances)
			:mIndexBuffer(inIndexBuffer)
			,mPrimitiveType(inPrimitiveType)
			,mBaseVertexIndex(inBaseVertexIndex)
			,mFirstInstance(inFirstInstance)
			,mNumVertex(inNumVertex)
			,mStartIndex(inStartIndex)
			,mNumPrimitives(inNumPrimitive)
			,mNumInstances(inNumInstances)
		{

		}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandDrawPrimitive : public RHICommand<RHICommandDrawPrimitive>
	{
		uint32 mBaseVertexIndex;
		uint32 mNumPrimitives;
		uint32 mNumInstances;
		RHICommandDrawPrimitive(uint32 inBaseVertexIndex, uint32 inNumPrimitives, uint32 inNumInstances)
			:mBaseVertexIndex(inBaseVertexIndex)
			,mNumPrimitives(inNumPrimitives)
			,mNumInstances(inNumInstances)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandCopyToResolveTarget : public
		RHICommand<RHICommandCopyToResolveTarget>
	{
		ResolveParams mResolveParams;
		RHITexture* mSourceTexture;
		RHITexture* mDestTexture;

		FORCEINLINE_DEBUGGABLE RHICommandCopyToResolveTarget(RHITexture* sourceTexture, RHITexture* destTexture, const ResolveParams& inResolveParams)
			:mResolveParams(inResolveParams)
			,mSourceTexture(sourceTexture)
			,mDestTexture(destTexture)
		{
			BOOST_ASSERT(sourceTexture);
			BOOST_ASSERT(destTexture);
			BOOST_ASSERT(sourceTexture->getTexture2D() || sourceTexture->getTexture3D() || sourceTexture->getTextureCube());
			BOOST_ASSERT(destTexture->getTexture2D() || destTexture->getTexture3D() || destTexture->getTextureCube());
		}

		RHI_API void execute(RHICommandListBase& CmdList);
	};

	

	template<typename TRHIShader, ECmdList cmdListType>
	struct RHICommandSetShaderSampler : public RHICommand<RHICommandSetShaderSampler<TRHIShader, cmdListType>>
	{
		TRHIShader* mShader;
		uint32 mSamplerIndex;
		RHISamplerState* mState;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderSampler(TRHIShader* inShader, uint32 inSamplerIndex, RHISamplerState* inState)
			:mShader(inShader)
			, mSamplerIndex(inSamplerIndex)
			, mState(inState)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TRHIShader, ECmdList cmdListType>
	struct RHICommandSetShaderParameter : public RHICommand<RHICommandSetShaderParameter<TRHIShader, cmdListType>>
	{
		TRHIShader* mShader;
		const void * newValue;
		uint32 mBufferIndex;
		uint32 mBaseIndex;
		uint32 mNumBytes;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderParameter(TRHIShader* inShader, uint32 inBufferIndex, uint32 inBaseIndex, uint32 inNumBytes, const void* inNewValue)
			:mShader(inShader)
			,mBufferIndex(inBufferIndex)
			,mBaseIndex(inBaseIndex)
			,mNumBytes(inNumBytes)
			,newValue(inNewValue)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TRHIShader, ECmdList cmdListType>
	struct RHICommandSetShaderTexture : public RHICommand<RHICommandSetShaderTexture<TRHIShader, cmdListType>>
	{
		TRHIShader* mShader;
		uint32 mTextureIndex;
		RHITexture* mTexture;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderTexture(TRHIShader* inShader, uint32 inTextureIndex, RHITexture* inTexture)
			:mShader(inShader)
			,mTextureIndex(inTextureIndex)
			,mTexture(inTexture)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandUpdateTextureReference : public RHICommand<RHICommandUpdateTextureReference>
	{
		RHITextureReference* mTextureRef;
		RHITexture* mNewTexture;
		FORCEINLINE_DEBUGGABLE RHICommandUpdateTextureReference(
			RHITextureReference* inTextureRef, RHITexture* inNewTexture)
			:mTextureRef(inTextureRef)
			,mNewTexture(inNewTexture)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TRHIShader, ECmdList CmdListType>
	struct RHICommandSetUAVParameter final : public RHICommand<RHICommandSetUAVParameter<TRHIShader, CmdListType>>
	{
		TRHIShader* mShader;
		uint32 mUAVIndex;
		RHIUnorderedAccessView* mUAV;

		FORCEINLINE_DEBUGGABLE RHICommandSetUAVParameter(TRHIShader* inShader, uint32 inUAVIndex, RHIUnorderedAccessView* inUAV)
			:mShader(inShader)
			,mUAVIndex(inUAVIndex)
			,mUAV(inUAV)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TRHIShader, ECmdList CmdListType>
	struct RHICommandSetUAVParameter_InitialCount final : public RHICommand< RHICommandSetUAVParameter_InitialCount<TRHIShader, CmdListType>>
	{
		TRHIShader* mShader;
		uint32 mUAVIndex;
		RHIUnorderedAccessView* mUAV;
		uint32 mInitialCount;

		FORCEINLINE_DEBUGGABLE RHICommandSetUAVParameter_InitialCount(TRHIShader* shader, uint32 inUAVIndex, RHIUnorderedAccessView* inUAV, uint32 inInitialCount)
			:mShader(shader)
			,mUAVIndex(inUAVIndex)
			,mUAV(inUAV)
			,mInitialCount(inInitialCount)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetGraphicsPipelineState final : public RHICommand< RHICommandSetGraphicsPipelineState>
	{
		GraphicsPipelineState* mGraphicsPipelineState;
		FORCEINLINE_DEBUGGABLE RHICommandSetGraphicsPipelineState(GraphicsPipelineState* inGraphicsPipelineState)
			:mGraphicsPipelineState(inGraphicsPipelineState)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandBeginRenderPass final : public RHICommand<RHICommandBeginRenderPass>
	{
		RHIRenderPassInfo mInfo;
		const TCHAR* mName;
		RHICommandBeginRenderPass(const RHIRenderPassInfo& inInfo, const TCHAR* inName)
			:mInfo(inInfo)
			,mName(inName)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandEndRenderPass final : public RHICommand<RHICommandEndRenderPass>
	{
		RHICommandEndRenderPass()
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetBlendFactor final : public RHICommand<RHICommandSetBlendFactor>
	{
		LinearColor mBlendFactor;
		FORCEINLINE_DEBUGGABLE RHICommandSetBlendFactor(const LinearColor& inBlendFactor)
			:mBlendFactor(inBlendFactor)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<ECmdList CmdListType>
	struct RHICommandWaitComputeFence final : public RHICommand< RHICommandWaitComputeFence<CmdListType>>
	{
		RHIComputeFence* mWaitFence;

		FORCEINLINE_DEBUGGABLE RHICommandWaitComputeFence(RHIComputeFence* inWaitFence)
			:mWaitFence(inWaitFence)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSubmitCommandsHint final : public RHICommand<RHICommandSubmitCommandsHint>
	{
		FORCEINLINE_DEBUGGABLE RHICommandSubmitCommandsHint()
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

#define RHICOMMAND_MACRO(CommandName) 	\
struct PREPROCESSOR_JOIN(CommandName##String, __LINE__)			\
{																\
	static const TCHAR* TStr(){return TEXT(#CommandName);}		\
};																\
struct CommandName final : public RHICommand<CommandName, PREPROCESSOR_JOIN(CommandName##String, __LINE__)>

#define CMD_CONTEXT(Method) getContext().Method
#define COMPUTE_CONTEXT(Method) getComputeContext().Method
	

	RHICOMMAND_MACRO(RHICommandBeginTransitions)
	{
		TArrayView<const RHITransition*> mTransitions;
		RHICommandBeginTransitions(TArrayView<const RHITransition*> inTransitions)
			:mTransitions(inTransitions)
		{}

		RHI_API void execute(RHICommandListBase & cmdList);
	};

	RHICOMMAND_MACRO(RHICommandEndTransitions)
	{
		TArrayView<const RHITransition*> mTransitions;
		RHICommandEndTransitions(TArrayView<const RHITransition*> inTranstions)
			:mTransitions(inTranstions)
		{

		}

		RHI_API void execute(RHICommandListBase & cmdList);
	};

	RHICOMMAND_MACRO(RHICommandResourceTransition)
	{
		RHITransition* mTransition;
		RHICommandResourceTransition(RHITransition * inTransition)
			:mTransition(inTransition)
		{}

		RHI_API void execute(RHICommandListBase & cmdList);
	};


	class RHI_API RHIComputeCommandList : public RHICommandListBase
	{
	public:

		FORCEINLINE_DEBUGGABLE void BeginTransitions(TArrayView<const RHITransition*> transitions)
		{
			if (bypass())
			{
				getComputeContext().RHIBeginTransitions(transitions);
				for (const RHITransition* transition : transitions)
				{
					transition->markBegin(getPipeline());
				}
			}
			else
			{
				RHITransition** dstTransitionArray = (RHITransition**)alloc(sizeof(RHITransition*) * transitions.size(), alignof(RHITransition*));
				Memory::memcpy(dstTransitionArray, transitions.getData(), sizeof(RHITransition*) * transitions.size());
				ALLOC_COMMAND(RHICommandBeginTransitions)(makeArrayView((const RHITransition**)dstTransitionArray, transitions.size()));
			}
		}

		FORCEINLINE_DEBUGGABLE void endTransitions(TArrayView<const RHITransition*> transitions)
		{
			if (bypass())
			{
				getComputeContext().RHIEndTransitions(transitions);
				for (const RHITransition* transition : transitions)
				{
					transition->markEnd(getPipeline());
				}
			}
			else
			{
				RHITransition** dstTransitionArray = (RHITransition**)alloc(sizeof(RHITransition*) * transitions.size(), alignof(RHITransition*));
				Memory::memcpy(dstTransitionArray, transitions.getData(), sizeof(RHITransition*) * transitions.size());
				ALLOC_COMMAND(RHICommandBeginTransitions)(makeArrayView((const RHITransition**)dstTransitionArray, transitions.size()));
			}
		}

		FORCEINLINE_DEBUGGABLE void transitionResource(ERHIAccess transitionType, EResourceTransitionPipeline transitionPipeline, RHIUnorderedAccessView* inUAV, RHIComputeFence* writeFence)
		{
			transitionResources(transitionType, transitionPipeline, &inUAV, 1, writeFence);
		}

		FORCEINLINE_DEBUGGABLE void transitionResource(ERHIAccess transitionType, EResourceTransitionPipeline transitionPipeline, RHIUnorderedAccessView* inUAV)
		{
			transitionResource(transitionType, transitionPipeline, inUAV, nullptr);
		}

		FORCEINLINE_DEBUGGABLE void transitionResource(FExclusiveDepthStencil depthStencilMode, RHITexture* depthTexture)
		{
			BOOST_ASSERT(depthStencilMode.isUsingDepth() || depthStencilMode.isUsingStencil());
			TArray<RHITransitionInfo, TInlineAllocator<2>> infos;
			depthStencilMode.enumerateSubresource([&](ERHIAccess newAccess, uint32 planeSlice)
				{
					RHITransitionInfo info;
					info.mType = RHITransitionInfo::EType::Texture;
					info.mTexture = depthTexture;
					info.mAccessAfter = newAccess;
					info.mPlaneSlice = planeSlice;
					infos.emplace(info);
				});
			RHIComputeCommandList::transition(makeArrayView(infos));
		}

		inline void transition(TArrayView<const RHITransitionInfo> infos)
		{
			ERHIPipeline pipeline = getPipeline();
			if (bypass())
			{
				MemStack& memStack = MemStack::get();
				MemMark mark(memStack);
				RHITransition* transition = new (memStack.alloc(RHITransition::getTotalAllocationSize(), RHITransition::getAlignment()))RHITransition(pipeline, pipeline);
				GDynamicRHI->RHICreateTransition(transition, pipeline, pipeline, ERHICreateTransitionFlags::NoSplit, infos);
				getComputeContext().RHIBeginTransitions(makeArrayView((const RHITransition**)&transition, 1));
				getComputeContext().RHIEndTransitions(makeArrayView((const RHITransition**)&transition, 1));

				GDynamicRHI->RHIReleaseTransition(transition);
				transition->~RHITransition();
			}
			else
			{
				RHITransition* transition = new (alloc(RHITransition::getTotalAllocationSize(), RHITransition::getAlignment()))RHITransition(pipeline, pipeline);
				GDynamicRHI->RHICreateTransition(transition, pipeline, pipeline, ERHICreateTransitionFlags::NoSplit, infos);
				ALLOC_COMMAND(RHICommandResourceTransition)(transition);
			}
		}

		FORCEINLINE_DEBUGGABLE void transitionResources(ERHIAccess transitionType, EResourceTransitionPipeline transitionPipeline, RHIUnorderedAccessView** inUAV, int32 numUAVs, RHIComputeFence* writeFence)
		{
			MemMark mark(MemStack::get());
			TArray<RHITransitionInfo, TMemStackAllocator<>> infos;
			infos.reserve(numUAVs);
			for (int32 index = 0; index < numUAVs; ++index)
			{
				infos.add(RHITransitionInfo(inUAV[index], ERHIAccess::Unknown, transitionType));
			}
			if (writeFence)
			{
				ERHIPipeline srcPipeline = isAsyncCompute() ? ERHIPipeline::AsyncCompute : ERHIPipeline::Graphics;
				ERHIPipeline dstPipeline = isAsyncCompute() ? ERHIPipeline::Graphics : ERHIPipeline::AsyncCompute;
				writeFence->mTransition = RHICreateTransition(srcPipeline, dstPipeline, ERHICreateTransitionFlags::None, infos);
				BeginTransitions(makeArrayView(&writeFence->mTransition, 1));
			}
			else
			{
				transition(infos);
			}
		}

		FORCEINLINE_DEBUGGABLE void transitionResources(ERHIAccess transitionType, EResourceTransitionPipeline transitionPipeline, RHIUnorderedAccessView** inUAV, int32 numUAVs)
		{
			transitionResources(transitionType, transitionPipeline, inUAV, numUAVs, nullptr);
		}

		FORCEINLINE_DEBUGGABLE void submitCommandHint()
		{
			if (bypass())
			{
				getComputeContext().RHISubmitCommandsHint();
				return;
			}
			ALLOC_COMMAND(RHICommandSubmitCommandsHint)();
		}

		FORCEINLINE_DEBUGGABLE void waitComputeFence(RHIComputeFence* waitFence)
		{
			BOOST_ASSERT(waitFence->mTransition);
			endTransitions(makeArrayView(&waitFence->mTransition, 1));
			waitFence->mTransition = nullptr;
		}
	};

	class RHI_API RHICommandList : public RHIComputeCommandList
	{
	public:	
		void* operator new(size_t size);
		void operator delete(void* rawMemory);

		void beginDrawingViewport(RHIViewport* viewport, RHITexture* renderTargetRHI);

		void endDrawingViewport(RHIViewport* viewport, bool bPreset, bool bLockToVsync);

		FORCEINLINE_DEBUGGABLE void beginRenderPass(const RHIRenderPassInfo& inInfo, const TCHAR* name)
		{
			BOOST_ASSERT(!isInsideRenderPass());
			BOOST_ASSERT(!isInsideComputePass());

			if (inInfo.bTooManyUAVs)
			{
				AIR_LOG(logRHI, Warning, TEXT("RenderPass has too many UVAs"));
			}
			inInfo.validate();
			if (bypass())
			{
				getContext().RHIBeginRenderPass(inInfo, name);
			}
			else
			{
				TCHAR* nameCopy = allocString(name);
				ALLOC_COMMAND(RHICommandBeginRenderPass)(inInfo, nameCopy);
			}

			mData.bInsideRenderPass = true;
			cacheActiveRenderTargets(inInfo);
			resetSubpass(inInfo.mSubpassHint);
			mData.bInsideRenderPass = true;
		}

		void endRenderPass()
		{
			BOOST_ASSERT(isInsideRenderPass());
			BOOST_ASSERT(!isInsideComputePass());
			if (bypass())
			{
				getContext().RHIEndRenderPass();
			}
			else
			{
				ALLOC_COMMAND(RHICommandEndRenderPass)();
			}

			mData.bInsideRenderPass = false;
			resetSubpass(ESubpassHint::None);
		}

		FORCEINLINE_DEBUGGABLE void setGraphicsPipelineState(class GraphicsPipelineState* graphicsPipelineState)
		{
			if (bypass())
			{
				

				RHIGraphicsPipelineState* rhiGraphicsPipelineState = executeSetGraphicsPipelineState(graphicsPipelineState);
				getContext().RHISetGraphicsPipelineState(rhiGraphicsPipelineState);
				return;
			}
			ALLOC_COMMAND(RHICommandSetGraphicsPipelineState)(graphicsPipelineState);
		}
		
		

		FORCEINLINE_DEBUGGABLE void drawIndexedPrimitive(RHIIndexBuffer* indexBuffer, uint32 primitiveType, int32 baseVertexIndex, int32 firstInstance, uint32 numVertices, uint32 startIndex, uint32 numPrimitive, uint32 numInstances)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIDrawIndexedPrimitive)(indexBuffer, primitiveType, baseVertexIndex, firstInstance, numVertices, startIndex, numPrimitive, numInstances);
				return;
			}
			new (allocCommand<RHICommandDrawIndexedPrimitive>())RHICommandDrawIndexedPrimitive(indexBuffer, primitiveType, baseVertexIndex, firstInstance, numVertices, startIndex, numPrimitive, numInstances);
		}

		FORCEINLINE_DEBUGGABLE void drawPrimitive(uint32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIDrawPrimitive)(baseVertexIndex, numPrimitives, numInstances);
				return;
			}
			new (allocCommand<RHICommandDrawPrimitive>())RHICommandDrawPrimitive(baseVertexIndex, numPrimitives, numInstances);
		}

		FORCEINLINE_DEBUGGABLE void setStreamSource(uint32 streadIndex, RHIVertexBuffer* vertexBuffer, uint32 offset)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetStreamSource)(streadIndex, vertexBuffer, offset);
				return;
			}
			new (allocCommand<RHICommandSetStreamSource>())RHICommandSetStreamSource(streadIndex, vertexBuffer, offset);
		}

		using RHIComputeCommandList::transitionResource;
		using RHIComputeCommandList::transitionResources;
	

		FORCEINLINE_DEBUGGABLE LocalConstantBuffer buildLocalConstantBuffer(const void* contents, uint32 contentsSize, const RHIConstantBufferLayout& layout)
		{
			LocalConstantBuffer result;
			if (bypass())
			{
				result.mBypassConstant = RHICreateConstantBuffer(contents, layout, ConstantBuffer_SingleFrame);
			}
			else
			{
				BOOST_ASSERT(contents && contentsSize && (&layout != nullptr));
				auto * cmd = new (allocCommand<RHICommandBuildLocalConstantBuffer>())RHICommandBuildLocalConstantBuffer(this, contents, contentsSize, layout);
				result.mWorkArea = &cmd->mWorkArea;

			}
			return result;
		}

		FORCEINLINE_DEBUGGABLE void setRenderTargetAndClear(const RHISetRenderTargetsInfo& renderTargetInfo)
		{
			cacheActiveRenderTargets(renderTargetInfo.mNumColorRenderTargets, renderTargetInfo.mColorRenderTarget, &renderTargetInfo.mDepthStencilRenderTarget);
			if (bypass())
			{
				CMD_CONTEXT(RHISetRenderTargetsAndClear)(renderTargetInfo);
				return;
			}
			new (allocCommand<RHICommandSetRenderTargetsAndClear>())RHICommandSetRenderTargetsAndClear(renderTargetInfo);
		}

		FORCEINLINE_DEBUGGABLE void setRenderTargets(uint32 newNumSimultanieusRenderTargets, const RHIRenderTargetView* newRenderTargetsRHI, const RHIDepthRenderTargetView* newDepthStencilTargetRHI, uint32 newNumUAVs, RHIUnorderedAccessView* const* UAVs)
		{
			cacheActiveRenderTargets(newNumSimultanieusRenderTargets, newRenderTargetsRHI, newDepthStencilTargetRHI);
			if (bypass())
			{
				CMD_CONTEXT(RHISetRenderTargets)(newNumSimultanieusRenderTargets, newRenderTargetsRHI, newDepthStencilTargetRHI, newNumUAVs, UAVs);
				return;
			}
			new (allocCommand<RHICommandSetRenderTargets>())RHICommandSetRenderTargets(newNumSimultanieusRenderTargets, newRenderTargetsRHI, newDepthStencilTargetRHI, newNumUAVs, UAVs);
		}
		

		FORCEINLINE_DEBUGGABLE void automaticCacheFlushAfterComputeShader(bool bEnable)
		{
			if (bypass())
			{
				getContext().RHIAutomaticCacheFlushAfterComputeShader(bEnable);
				return;
			}
			ALLOC_COMMAND(RHICommandAutomaticCacheFlushAfterComputeShader)(bEnable);
		}

		FORCEINLINE_DEBUGGABLE void flushComputeShaderCache()
		{
			if (bypass())
			{
				getContext().RHIFlushComputeShaderCache();
				return;
			}
			ALLOC_COMMAND(RHICommandFlushComputeShaderCache)();
		}

		FORCEINLINE_DEBUGGABLE void bindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIBindClearMRTValues)(bClearColor, bClearDepth, bClearStencil);
				return;
			}

			new (allocCommand<RHICommandBindClearMRTValues>())RHICommandBindClearMRTValues(bClearColor, bClearDepth, bClearStencil);
		}

		FORCEINLINE_DEBUGGABLE void setBlendFactor(const LinearColor& blendFactor = LinearColor::White)
		{
			if (bypass())
			{
				getContext().RHISetBlendFactor(blendFactor);
				return;
			}
			ALLOC_COMMAND(RHICommandSetBlendFactor)(blendFactor);
		}

		

		FORCEINLINE_DEBUGGABLE void setStencilRef(uint32 stencilRef)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetStencilRef)(stencilRef);
				return;
			}
			
			ALLOC_COMMAND(RHICommandSetStencilRef)(stencilRef);
		}
		FORCEINLINE_DEBUGGABLE void setViewport(uint32 x, uint32 y, float z, uint32 width, uint32 height, float depth)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetViewport)(x, y, z, width, height, depth);
				return;
			}
			ALLOC_COMMAND(RHICommandSetViewport)(x, y, z, width, height, depth);
		}

		FORCEINLINE_DEBUGGABLE void setStereoViewport(uint32 leftMinX, uint32 rightMinX, uint32 leftMinY, uint32 rightMinY, float z, uint32 leftMaxX, uint32 rightMaxX, uint32 leftMaxY, uint32 rightMaxY, float maxZ)
		{
			if (bypass())
			{
				getContext().RHISetStereoViewport(leftMinX, rightMinX, leftMinY, rightMinY, z, leftMaxX, rightMaxX, leftMaxY, rightMaxY, maxZ);
			}
			ALLOC_COMMAND(RHICommandSetStereoViewport)(leftMinX, rightMinX, leftMinY, rightMaxY, z, leftMaxX, rightMaxX, leftMaxY, rightMaxY, maxZ);
		}


		
		void applyCachedRenderTargets(GraphicsPipelineStateInitializer & GraphicsPSOInit)
		{
			GraphicsPSOInit.mRenderTargetsEnabled = mPSOContext.mCachedNumSimultanousRenderTargets;
			for (int32 i = 0; i < GraphicsPSOInit.mRenderTargetsEnabled; ++i)
			{
				if (mPSOContext.mCachedRenderTargets[i].mTexture)
				{
					GraphicsPSOInit.mRenderTargetFormats[i] = mPSOContext.mCachedRenderTargets[i].mTexture->getFormat();
					GraphicsPSOInit.mRenderTargetFlags[i] = mPSOContext.mCachedRenderTargets[i].mTexture->getFlags();
					const RHITexture2DArray* textureArray = mPSOContext.mCachedRenderTargets[i].mTexture->getTextureArray();
					GraphicsPSOInit.bMultiView = textureArray && textureArray->getDepth() > 1;
				}
				else
				{
					GraphicsPSOInit.mRenderTargetFormats[i] = PF_Unknown;
				}
				if (GraphicsPSOInit.mRenderTargetFlags[i] != PF_Unknown)
				{
					GraphicsPSOInit.mNumSamples = mPSOContext.mCachedRenderTargets[i].mTexture->getNumSamples();
				}
			}

			if (mPSOContext.mCachedDepthStencilTarget.mTexture)
			{
				GraphicsPSOInit.mDepthStencilTargetFormat = mPSOContext.mCachedDepthStencilTarget.mTexture->getFormat();
				GraphicsPSOInit.mDepthStencilTargetFlags = mPSOContext.mCachedDepthStencilTarget.mTexture->getFlags();
				const RHITexture2DArray* textureArray = mPSOContext.mCachedDepthStencilTarget.mTexture->getTextureArray();
				GraphicsPSOInit.bMultiView = textureArray && textureArray->getDepth() > 1;
			}
			else
			{
				GraphicsPSOInit.mDepthStencilTargetFormat = PF_Unknown;
			}

			GraphicsPSOInit.mDepthTargetLoadAction = mPSOContext.mCachedDepthStencilTarget.mDepthLoadAction;
			GraphicsPSOInit.mDepthTargetStoreAction = mPSOContext.mCachedDepthStencilTarget.mDepthStoreAction;
			GraphicsPSOInit.mStencilTargetLoadAction = mPSOContext.mCachedDepthStencilTarget.mStencialLoadAction;
			GraphicsPSOInit.mStencilTargetStoreAction = mPSOContext.mCachedDepthStencilTarget.mStencialStoreAction;
			GraphicsPSOInit.mDepthStencilAccess = mPSOContext.mCachedDepthStencilTarget.getDepthStencilAccess();

			if (GraphicsPSOInit.mDepthStencilTargetFormat != PF_Unknown)
			{
				GraphicsPSOInit.mNumSamples = mPSOContext.mCachedDepthStencilTarget.mTexture->getNumSamples();
			}

			GraphicsPSOInit.mSubpassHint = mPSOContext.mSubpassHint;
			GraphicsPSOInit.mSubpassIndex = mPSOContext.mSubpassIndex;

		}

		template<typename TRHIShader>
		FORCEINLINE_DEBUGGABLE void setLocalShaderConstantBuffer(TRHIShader* shader, uint32 baseIndex, const LocalConstantBuffer& constantBuffer)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderConstantBuffer)(shader, baseIndex, constantBuffer.mBypassConstant);
				return;
			}
			new (allocCommand<RHICommandSetLocalConstantBuffer<TRHIShader>>())RHICommandSetLocalConstantBuffer<TRHIShader>(this, shader, baseIndex, constantBuffer);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setLocalShaderConstantBuffer(TRefCountPtr<TShaderRHI>& shader, uint32 baseIndex, const LocalConstantBuffer& constantBuffer)
		{
			setLocalShaderConstantBuffer(shader.getInitReference(), baseIndex, constantBuffer);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderConstantBuffer(TShaderRHI* shader, uint32 baseIndex, RHIConstantBuffer* constantBuffer)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderConstantBuffer)(shader, baseIndex, constantBuffer);
				return;
			}
			new (allocCommand<RHICommandSetShaderConstantBuffer<TShaderRHI, ECmdList::EGfx>>())RHICommandSetShaderConstantBuffer<TShaderRHI, ECmdList::EGfx>(shader, baseIndex, constantBuffer);
		}
		template<typename TShaderRHI>
		FORCEINLINE void setShaderConstantBuffer(TRefCountPtr<TShaderRHI>& shader, uint32 baseIndex, RHIConstantBuffer* constantBuffer)
		{
			setShaderConstantBuffer(shader.getReference(), baseIndex, constantBuffer);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderParameter(TShaderRHI * shader, uint32 BufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
		{
			if (bypass())
			{
				getContext().RHISetShaderParameter(shader, BufferIndex, baseIndex, numBytes, newValue);
				return;
			}
			void * useValue = alloc(numBytes, 16);
			Memory::memcpy(useValue, newValue, numBytes);
			new (allocCommand<RHICommandSetShaderParameter<TShaderRHI, ECmdList::EGfx>>())RHICommandSetShaderParameter<TShaderRHI, ECmdList::EGfx>(shader, BufferIndex, baseIndex, numBytes, useValue);
		}

		template<typename TShaderRHI>
		FORCEINLINE void setShaderParameter(TRefCountPtr<TShaderRHI>& shader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* value)
		{
			setShaderParameter(shader.getReference(), bufferIndex, baseIndex, numBytes, value);
		}


		

		template<typename TRHIShader>
		FORCEINLINE_DEBUGGABLE void setShaderTexture(TRHIShader* shader, uint32 textureIndex, RHITexture* texture)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderTexture)(shader, textureIndex, texture);
				return;
			}
			new (allocCommand<RHICommandSetShaderTexture<TRHIShader, ECmdList::EGfx>>())RHICommandSetShaderTexture<TRHIShader, ECmdList::EGfx>(shader, textureIndex, texture);
		}
		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderTexture(TRefCountPtr<TShaderRHI>& shader, uint32 textureIndex, RHITexture* texture)
		{
			setShaderTexture(shader.getReference(), textureIndex, texture);
		}

		template<typename TRHIShader>
		FORCEINLINE_DEBUGGABLE void setShaderSampler(TRHIShader* shader, uint32 samplerIndex, RHISamplerState* state)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderSampler)(shader, samplerIndex, state);
				return;
			}

			ALLOC_COMMAND(RHICommandSetShaderSampler<TRHIShader, ECmdList::EGfx>)(shader, samplerIndex, state);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderSampler(TRefCountPtr<TShaderRHI> shader, uint32 samplerIndex, RHISamplerState* state)
		{
			setShaderSampler(shader.getReference(), samplerIndex, state);
		}

		FORCEINLINE_DEBUGGABLE void setUAVParameter(RHIComputeShader* shader, uint32 uavIndex, RHIUnorderedAccessView* view)
		{
			if (bypass())
			{
				getComputeContext().RHISetUAVParameter(shader, uavIndex, view);
				return;
			}
			ALLOC_COMMAND(RHICommandSetUAVParameter<RHIComputeShader, ECmdList::ECompute>)(shader, uavIndex, view);
		}

		FORCEINLINE_DEBUGGABLE void setUAVParameter(RHIComputeShader* shader, uint32 uavIndex, RHIUnorderedAccessView* view, uint32 initialCount)
		{
			if (bypass())
			{
				getComputeContext().RHISetUAVParameter(shader, uavIndex, view, initialCount);
				return;
			}
			ALLOC_COMMAND(RHICommandSetUAVParameter_InitialCount<RHIComputeShader, ECmdList::ECompute>)(shader, uavIndex, view, initialCount);
		}

		FORCEINLINE_DEBUGGABLE void copyToResolveTarget(RHITexture* sourteTExtureRHI, RHITexture* destTextureRHI, const ResolveParams& resolveParams)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHICopyToResolveTarget)(sourteTExtureRHI, destTextureRHI, resolveParams);
				return;
			}

			new (allocCommand<RHICommandCopyToResolveTarget>())RHICommandCopyToResolveTarget(sourteTExtureRHI, destTextureRHI, resolveParams);
		}

		

		FORCEINLINE_DEBUGGABLE void setScissorRect(bool bEnable, uint32 minX, uint32 minY, uint32 maxX, uint32 maxY)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetScissorRect)(bEnable, minX, minY, maxX, maxY);
				return;
			}
			new (allocCommand<RHICommandSetScissorRect>())RHICommandSetScissorRect(bEnable, minX, minY, maxX, maxY);
		}

		FORCEINLINE_DEBUGGABLE void setComputeShader(RHIComputeShader* computeShader)
		{
			computeShader->updateStats();
			if (bypass())
			{
				getContext().RHISetComputeShader(computeShader);
				return;
			}
			ALLOC_COMMAND(RHICommandSetComputeShader<ECmdList::EGfx>)(computeShader);
		}

		FORCEINLINE_DEBUGGABLE void dispatchComputeShader(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ)
		{
			if (bypass())
			{
				getContext().RHIDispatchComputeShader(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
			}
			ALLOC_COMMAND(RHICommandDispatchComputeShader<ECmdList::EGfx>)(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
		}

		template<typename TRHIShader>
		FORCEINLINE_DEBUGGABLE void setShaderResourceViewParameter(TRHIShader* shader, uint32 samplerIndex, RHIShaderResourceView* srv)
		{
			if (bypass())
			{
				getContext().RHISetShaderResourceViewParameter(shader, samplerIndex, srv);
				return;
			}
			ALLOC_COMMAND(RHICommandSetShaderResourceViewParameter<TRHIShader, ECmdList::EGfx>)(shader, samplerIndex, srv);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderResourceViewParameter(TRefCountPtr<TShaderRHI>& shader, uint32 samplerIndex, RHIShaderResourceView* srv)
		{
			setShaderResourceViewParameter(shader.getReference(), samplerIndex, srv)
		}
		void beginFrame();
		void endFrame();
		void beginScene();
		void endScene();
	};

	class RHI_API RHIAsyncComputeCommandList : public RHIComputeCommandList
	{
	public:
		FORCEINLINE_DEBUGGABLE void setShaderSampler(RHIComputeShader* shader, uint32 samplerIndex, RHISamplerState* state)
		{
			if (bypass())
			{
				COMPUTE_CONTEXT(RHISetShaderSampler)(shader, samplerIndex, state);
				return;
			}
			new(allocCommand<RHICommandSetShaderSampler<RHIComputeShader, ECmdList::ECompute>>())RHICommandSetShaderSampler<RHIComputeShader, ECmdList::ECompute>(shader, samplerIndex, state);
		}

		FORCEINLINE_DEBUGGABLE void setShaderParameter(RHIComputeShader* shader, uint32 BufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
		{
			if (bypass())
			{
				COMPUTE_CONTEXT(RHISetShaderParameter)(shader, BufferIndex, baseIndex, numBytes, newValue);
				return;
			}
			void* useValue = alloc(numBytes, 16);
			Memory::memcpy(useValue, newValue, numBytes);
			new (allocCommand<RHICommandSetShaderSampler<RHIComputeShader, ECmdList::ECompute>>())RHICommandSetShaderParameter<RHIComputeShader, ECmdList::ECompute>(shader, BufferIndex, baseIndex, numBytes, useValue);
		}

		FORCEINLINE_DEBUGGABLE void setShaderTexture(RHIComputeShader* shader, uint32 textureIndex, RHITexture* tex)
		{
			if (bypass())
			{
				COMPUTE_CONTEXT(RHISetShaderTexture)(shader, textureIndex, tex);
				return;
			}
			new (allocCommand<RHICommandSetShaderTexture<RHIComputeShader, ECmdList::ECompute>>())RHICommandSetShaderTexture<RHIComputeShader, ECmdList::ECompute>(shader, textureIndex, tex);
		}

		FORCEINLINE_DEBUGGABLE void setShaderResourceViewParameter(RHIComputeShader* shader, uint32 samplerIndex, RHIShaderResourceView* srv)
		{
			if (bypass())
			{
				getComputeContext().RHISetShaderResourceViewParameter(shader, samplerIndex, srv);
				return;
			}
			ALLOC_COMMAND(RHICommandSetShaderResourceViewParameter<RHIComputeShader, ECmdList::ECompute>)(shader, samplerIndex, srv);

		}

		FORCEINLINE_DEBUGGABLE void setComputeShader(RHIComputeShader* computeShader)
		{
			computeShader->updateStats();
			if (bypass())
			{
				getComputeContext().RHISetComputeShader(computeShader);
				return;
			}
			ALLOC_COMMAND(RHICommandSetComputeShader<ECmdList::ECompute>)(computeShader);
		}

		FORCEINLINE_DEBUGGABLE void dispatchComputeShader(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ)
		{
			if (bypass())
			{
				getComputeContext().RHIDispatchComputeShader(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
			}
			ALLOC_COMMAND(RHICommandDispatchComputeShader<ECmdList::ECompute>)(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
		}
	};

	class RHI_API RHICommandListImmediate : public RHICommandList
	{
	public:
		template<typename LAMBDA>
		struct TRHILambdaCommand final : public RHICommandBase
		{
			LAMBDA mLambda;

			TRHILambdaCommand(LAMBDA&& inLambda)
				:mLambda(inLambda)
			{}

			void executeAndDestruct(RHICommandListBase& cmdList, RHICommandListDebugContext&) override final
			{
				mLambda(*static_cast<RHICommandListImmediate*>(&cmdList));
				mLambda.~LAMBDA();
			}
		};

		void queueAsyncCompute(RHIAsyncComputeCommandList& RHIComputeList);


		void immediateFlush(EImmediateFlushType::Type flushType);

		FORCEINLINE void flushResource()
		{

		}

		FORCEINLINE void acquireThreadOwnership()
		{

		}

		FORCEINLINE void releaseThreadOwnership()
		{

		}

		bool stallRHIThread();
		void unStallRHIThread();



		GraphEventRef RHIThreadFence(bool bSetLockFence = false);

		FORCEINLINE Texture2DRHIRef createTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo)
		{
			return RHICreateTexture2D(width, height, format, numMips, numSamplers, flags, createInfo);
		}

		FORCEINLINE TextureCubeRHIRef createTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)
		{
			return GDynamicRHI->RHICreateTextureCube_RenderThread(*this, size, format, numMips, flags, createInfo);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(RHITexture* texture, const RHITextureSRVCreateInfo& createInfo)
		{
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, texture, createInfo);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(RHITexture* texture, uint8 mipLevel)
		{
			const RHITextureSRVCreateInfo createInfo(mipLevel, 1, texture->getFormat());
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, texture, createInfo);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(RHIStructuredBuffer* structuredBuffer)
		{
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, structuredBuffer);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(RHITexture* texture, uint8 mipLevel, uint8 numMipLevels, uint8 format)
		{
			const RHITextureSRVCreateInfo createInfo(mipLevel, numMipLevels, format);
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, texture, createInfo);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format)
		{
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, vertexBuffer, stride, format);
		}

		FORCEINLINE UnorderedAccessViewRHIRef createUnorderedAccessView(RHIStructuredBuffer* structuredBuffer, bool bUseUAVCounter, bool bAppendBuffer)
		{
			return GDynamicRHI->RHICreateUnorderredAccessView_RenderThread(*this, structuredBuffer, bUseUAVCounter, bAppendBuffer);
		}

		FORCEINLINE UnorderedAccessViewRHIRef createUnorderedAccessView(RHITexture* texture, uint32 mipLevel)
		{
			return GDynamicRHI->RHICreateUnorderredAccessView_RenderThread(*this, texture, mipLevel);
		}

		FORCEINLINE UnorderedAccessViewRHIRef createUnorderedAccessView(RHIVertexBuffer* vertexBuffer, uint8 format)
		{
			return GDynamicRHI->RHICreateUnorderredAccessView_RenderThread(*this, vertexBuffer, format);
		}

		FORCEINLINE UnorderedAccessViewRHIRef createUnorderedAccessView(RHIIndexBuffer* indexBuffer, uint8 format)
		{
			return GDynamicRHI->RHICreateUnorderredAccessView_RenderThread(*this, indexBuffer, format);
		}


		
		FORCEINLINE VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code)
		{
			return GDynamicRHI->RHICreateVertexShader(code);
		}

		FORCEINLINE HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code)
		{
			return GDynamicRHI->RHICreateHullShader(code);
		}

		FORCEINLINE DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code)
		{
			return GDynamicRHI->RHICreateDomainShader(code);
		}

		FORCEINLINE GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code)
		{
			return GDynamicRHI->RHICreateGeometryShader(code);
		}

		FORCEINLINE PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code)
		{
			return GDynamicRHI->RHICreatePixelShader(code);
		}

		FORCEINLINE ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code)
		{
			return GDynamicRHI->RHICreateComputeShader(code);
		}

		FORCEINLINE ComputeFenceRHIRef createComputeFence(const wstring& name)
		{
			return GDynamicRHI->RHICreateComputeFence(name);
		}

		FORCEINLINE GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream)
		{
			return GDynamicRHI->RHICreateGeometryShaderWithStreamOutput(code, elementList, numStrides, strides, rasterizedStream);
		}

		FORCEINLINE void* lockTexture2D(RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32 & destStride, bool bLockWithinMiptail, bool bFlushRHIThread = true)
		{
			return GDynamicRHI->lockTexture2D_RenderThread(*this, texture, mipIndex, lockMode, destStride, bLockWithinMiptail, bFlushRHIThread);
		}

		FORCEINLINE void unLockTexture2D(RHITexture2D* texture, uint32 mipIndex, bool bLockWithinMiptail, bool bFlushRHIThread = false)
		{
			GDynamicRHI->unlockTexture2D_RenderThread(*this, texture, mipIndex, bLockWithinMiptail, bFlushRHIThread);
		}

		FORCEINLINE VertexBufferRHIRef createAndLockVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataPtr)
		{
			return GDynamicRHI->createAndLockVertexBuffer_RenderThread(*this, size, inUsage, createInfo, outDataPtr);
		}

		FORCEINLINE VertexBufferRHIRef createVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
		{
			return GDynamicRHI->createVertexBuffer_RenderThread(*this, size, inUsage, createInfo);
		}

		FORCEINLINE IndexBufferRHIRef createIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo & createInfo)
		{
			return GDynamicRHI->createIndexBuffer_RenderThread(*this, stride, size, inUsage, createInfo);
		}

		FORCEINLINE void* lockVertexBuffer(RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
		{
			return GDynamicRHI->lockVertexBuffer_RenderThread(*this, vertexBuffer, offset, sizeRHI, lockMode);
		}

		FORCEINLINE void unlockVertexBuffer(RHIVertexBuffer* vertexBuffer)
		{
			GDynamicRHI->unlockVertexBuffer_RenderThread(*this, vertexBuffer);
		}

		FORCEINLINE IndexBufferRHIRef createAndLockIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void* & outDataBuffer)
		{
			return GDynamicRHI->createAndLockIndexBuffer_RenderThread(*this, stride, size, inUsage, createInfo, outDataBuffer);
		}

		FORCEINLINE void* lockIndexBuffer(RHIIndexBuffer* indexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
		{
			return GDynamicRHI->lockIndexBuffer_RenderThread(*this, indexBuffer, offset, sizeRHI, lockMode);
		}

		FORCEINLINE void unlockIndexBuffer(RHIIndexBuffer* indexBuffer)
		{
			GDynamicRHI->unlockIndexBuffer_RenderThread(*this, indexBuffer);
		}

		FORCEINLINE VertexDeclarationRHIRef createVertexDeclaration(const VertexDeclarationElementList& elements)
		{
			return GDynamicRHI->createVertexDeclaration_RenderThread(*this, elements);
		}

		FORCEINLINE void* lockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)
		{
			immediateFlush(EImmediateFlushType::FlushRHIThread);
			return GDynamicRHI->RHILockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, lockMode, destStride, bLockWithinMiptail);
		}

		FORCEINLINE void unlockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail)
		{
			immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIUnlockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, bLockWithinMiptail);
		}

		FORCEINLINE TextureReferenceRHIRef createTextureReference(LastRenderTimeContainer* lastRenderTime)
		{
			return GDynamicRHI->RHICreateTextureReference(lastRenderTime);
		}

		FORCEINLINE StructuredBufferRHIRef createStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
		{
			return RHICreateStructuredBuffer(stride, size, inUsage, createInfo);
		}

		FORCEINLINE void discardTransientResource_RenderThread(RHIStructuredBuffer* buffer)
		{
			if (buffer->isCommitted())
			{
				if (GSupportsTransientResourceAliasing)
				{
					GDynamicRHI->RHIDiscardTransientResource_RenderThread(buffer);
				}
				buffer->setCommitted(false);
			}
		}

		FORCEINLINE void acquireTransientResource_RenderThread(RHIStructuredBuffer* buffer)
		{
			if (!buffer->isCommitted())
			{
				if (GSupportsTransientResourceAliasing)
				{
					GDynamicRHI->RHIAcquireTransientResource_RenderThread(buffer);
				}
				buffer->setCommitted(true);
			}
		}

		FORCEINLINE void discardTransientResource_RenderThread(RHIVertexBuffer* buffer)
		{
			if (buffer->isCommitted())
			{
				if (GSupportsTransientResourceAliasing)
				{
					GDynamicRHI->RHIDiscardTransientResource_RenderThread(buffer);
				}
				buffer->setCommitted(false);
			}
		}

		FORCEINLINE void acquireTransientResource_RenderThread(RHIVertexBuffer* buffer)
		{
			if (!buffer->isCommitted())
			{
				if (GSupportsTransientResourceAliasing)
				{
					GDynamicRHI->RHIAcquireTransientResource_RenderThread(buffer);
				}
				buffer->setCommitted(true);
			}
		}


		FORCEINLINE void discardTransientResource_RenderThread(RHITexture* texture)
		{
			if (texture->isCommitted())
			{
				if (GSupportsTransientResourceAliasing)
				{
					GDynamicRHI->RHIDiscardTransientResource_RenderThread(texture);
				}
				texture->setCommitted(false);
			}
		}

		FORCEINLINE void acquireTransientResource_RenderThread(RHITexture* texture)
		{
			if (!texture->isCommitted())
			{
				if (GSupportsTransientResourceAliasing)
				{
					GDynamicRHI->RHIAcquireTransientResource_RenderThread(texture);
				}
				texture->setCommitted(true);
			}
		}

		FORCEINLINE void readSurfaceFloatData(RHITexture* texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex)
		{
			immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIReadSurfaceFloatData(texture, rect, outData, cubeface, arrayIndex, mipIndex);
		}

		void updateTextureReference(RHITextureReference* textureRef, RHITexture* newTexture);

		template<typename LAMBDA>
		FORCEINLINE_DEBUGGABLE bool enqueueLambda(bool bRunOnCurrentThread, LAMBDA&& lambda)
		{
			if (bRunOnCurrentThread)
			{
				lambda(*this);
				return false;
			}
			else
			{
				ALLOC_COMMAND(TRHILambdaCommand<LAMBDA>)(std::forward<LAMBDA>(lambda));
				return true;
			}
		}

		template<typename LAMBDA>
		FORCEINLINE_DEBUGGABLE bool enqueueLambda(LAMBDA&& lambda)
		{
			return enqueueLambda(bypass(), std::forward<LAMBDA>(lambda));
		}
	};

	

	class RHI_API RHIAsyncComputeCommandListImmediate : public RHIAsyncComputeCommandList
	{
	public:
		static void immediateDispatch(RHIAsyncComputeCommandListImmediate& RHIComputeCmdList);

	};


	class RHI_API RHICommandList_RecursiveHazardous : public RHICommandList
	{
		RHICommandList_RecursiveHazardous()
		{}
	public:
		RHICommandList_RecursiveHazardous(IRHICommandContext* context)
		{
			setContext(context);
		}
	};


	class RHI_API RHICommandListExecutor
	{
	public:
		enum
		{
			//DefaultBypass = PLAT
		};

		RHICommandListExecutor();

		static inline RHICommandListImmediate& getImmediateCommandList();

		static inline RHIAsyncComputeCommandListImmediate& getImmediateAsyncComputeCommandList();

		void executeList(RHICommandListBase& cmdList);

		void executeInner(RHICommandListBase& cmdList);


		void latchBypass();

		FORCEINLINE_DEBUGGABLE bool bypass()
		{
#if 1
			return bLatchedBypass;
#endif
		}

		FORCEINLINE_DEBUGGABLE bool useParallelAlgorithms()
		{
#if 1
			return bLatchedUseParallelAlgorithms;
#endif
		}

		static void waitOnRHIThreadFence(GraphEventRef& fence);

		static void checkNoOutstandingCmdLists();

	private:
		static void executeInner_DoExecute(RHICommandListBase& cmlList);
	private:
		friend class RHICommandListBase;
		friend class ExecuteRHIThreadTask;
		bool bLatchedBypass;
		bool bLatchedUseParallelAlgorithms;
		ThreadSafeCounter mUIDCounter;
		ThreadSafeCounter mOutstandingCmdListCount;
		RHICommandListImmediate mCommandListImmediate;
		RHIAsyncComputeCommandListImmediate mAsyncComputeCmdListImmediate;
	};



	class ScopedRHIThreadStaller
	{
		class RHICommandListImmediate* mImmed;
	public:
		ScopedRHIThreadStaller(class RHICommandListImmediate& inImmed);
		~ScopedRHIThreadStaller();
	};

	
	extern RHI_API AutoConsoleTaskPriority CPrio_SceneRenderingTask;
	
	class RenderTask
	{
	public:
		FORCEINLINE static ENamedThreads::Type getDesiredThread()
		{
			return CPrio_SceneRenderingTask.get();
		}
	};

	

	extern RHI_API RHICommandListExecutor GRHICommandList;

	extern RHI_API bool GEnableAsyncCompute;

	FORCEINLINE void RHIFlushResource()
	{
		return RHICommandListExecutor::getImmediateCommandList().flushResource();
	}

	FORCEINLINE void RHIAcquireThreadOwnership()
	{
		return RHICommandListExecutor::getImmediateCommandList().acquireThreadOwnership();
	}

	FORCEINLINE void RHIReleaseThreadOwnership()
	{
		return RHICommandListExecutor::getImmediateCommandList().releaseThreadOwnership();
	}

	FORCEINLINE Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createTexture2D(width, height, format, numMips, numSamplers, flags, createInfo);
	}
	
	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(vertexBuffer, stride, format);
	}

	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(RHITexture* texture, uint8 mipLevel)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(texture, mipLevel);
	}

	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(RHITexture* texture, uint8 mipLevel, uint8 numMipLevels, uint8 format)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(texture, mipLevel, numMipLevels, format);
	}

	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(RHITexture* texture, const RHITextureSRVCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(texture, createInfo);
	}

	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIStructuredBuffer* structuredBuffer)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(structuredBuffer);
	}

	FORCEINLINE UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIStructuredBuffer* structuredBuffer, bool bUseUAVCounter, bool bAppendBuffer)
	{
		return RHICommandListExecutor::getImmediateCommandList().createUnorderedAccessView(structuredBuffer, bUseUAVCounter, bAppendBuffer);
	}

	FORCEINLINE UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHITexture* texture, uint32 mipLevel)
	{
		return RHICommandListExecutor::getImmediateCommandList().createUnorderedAccessView(texture, mipLevel);
	}

	FORCEINLINE UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIVertexBuffer* vertexBuffer, uint8 format)
	{
		return RHICommandListExecutor::getImmediateCommandList().createUnorderedAccessView(vertexBuffer, format);
	}

	FORCEINLINE UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIIndexBuffer* indexBuffer, uint8 format)
	{
		return RHICommandListExecutor::getImmediateCommandList().createUnorderedAccessView(indexBuffer, format);
	}

	FORCEINLINE VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreateVertexShader(code);
	}

	FORCEINLINE HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreateHullShader(code);
	}

	FORCEINLINE DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreateDomainShader(code);
	}

	FORCEINLINE GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreateGeometryShader(code);
	}

	FORCEINLINE PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreatePixelShader(code);
	}

	FORCEINLINE ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreateComputeShader(code);
	}

	FORCEINLINE GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream)
	{
		return RHICommandListExecutor::getImmediateCommandList().RHICreateGeometryShaderWithStreamOutput(code, elementList, numStrides, strides, rasterizedStream);
	}

	FORCEINLINE void* RHILockTexture2D(RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail, bool bFulshRHIThread = true)
	{
		return RHICommandListExecutor::getImmediateCommandList().lockTexture2D(texture, mipIndex, lockMode, destStride, bLockWithinMiptail, bFulshRHIThread);
	}

	FORCEINLINE void RHIUnlockTexture2D(RHITexture2D* texture, uint32 mipIndex, bool bLockWithinMiptail, bool bFlushRHIThread = true)
	{
		RHICommandListExecutor::getImmediateCommandList().unLockTexture2D(texture, mipIndex, bLockWithinMiptail, bFlushRHIThread);
	}
	FORCEINLINE VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createVertexBuffer(size, inUsage, createInfo);
	}

	FORCEINLINE VertexBufferRHIRef RHIAsyncCreateVertexBuffer(uint32 sie, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return GDynamicRHI->RHICreateVertexBuffer(inUsage, inUsage, createInfo);
	}

	FORCEINLINE IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createIndexBuffer(stride, size, inUsage, createInfo);
	}

	FORCEINLINE VertexBufferRHIRef RHICreateAndLockVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer)
	{
		return RHICommandListExecutor::getImmediateCommandList().createAndLockVertexBuffer(size, inUsage, createInfo, outDataBuffer);
	}
	FORCEINLINE void RHIUnlockVertexBuffer(RHIVertexBuffer* vertexBuffer)
	{
		RHICommandListExecutor::getImmediateCommandList().unlockVertexBuffer(vertexBuffer);
	}

	FORCEINLINE IndexBufferRHIRef RHICreateAndLockIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer)
	{
		return RHICommandListExecutor::getImmediateCommandList().createAndLockIndexBuffer(stride, size, inUsage, createInfo, outDataBuffer);
	}

	FORCEINLINE void RHIUnlockIndexBuffer(RHIIndexBuffer* indexBuffer)
	{
		RHICommandListExecutor::getImmediateCommandList().unlockIndexBuffer(indexBuffer);
	}

	FORCEINLINE VertexDeclarationRHIRef RHICreateVertexDeclaration(const VertexDeclarationElementList& elements)
	{
		return RHICommandListExecutor::getImmediateCommandList().createVertexDeclaration(elements);
	}

	FORCEINLINE TextureCubeRHIRef RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createTextureCube(size, format, numMips, flags, createInfo);
	}

	FORCEINLINE void RHIUpdateTextureReference(RHITextureReference* textureRHI, RHITexture* newTexture)
	{
		RHICommandListExecutor::getImmediateCommandList().updateTextureReference(textureRHI, newTexture);
	}

	FORCEINLINE void* RHILockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)
	{
		return RHICommandListExecutor::getImmediateCommandList().lockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, lockMode, destStride, bLockWithinMiptail);
	}


	FORCEINLINE void RHIUnlockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail)
	{
		RHICommandListExecutor::getImmediateCommandList().unlockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, bLockWithinMiptail);
	}

	FORCEINLINE TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime)
	{
		return RHICommandListExecutor::getImmediateCommandList().createTextureReference(lastRenderTime);
	}

	extern RHI_API ERHIAccess RHIGetDefaultResourceState(EBufferUsageFlags inUsage, bool bInHasInitialData);


	

	FORCEINLINE StructuredBufferRHIRef RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, ERHIAccess inResourceState, RHIResourceCreateInfo& createInfo)
	{
		return GDynamicRHI->RHICreateStructuredBuffer_RenderThread(RHICommandListExecutor::getImmediateCommandList(), stride, size, inUsage, inResourceState, createInfo);
	}
	FORCEINLINE StructuredBufferRHIRef RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		bool bHasInitialData = createInfo.mBulkData != nullptr;
		ERHIAccess resourceState = RHIGetDefaultResourceState((EBufferUsageFlags)inUsage | BUF_StructuredBuffer, bHasInitialData);
		return RHICreateStructuredBuffer(stride, size, inUsage, resourceState, createInfo);
	}
	FORCEINLINE void RHIAcquireTransientResource(RHIStructuredBuffer* resource)
	{
		RHICommandListExecutor::getImmediateCommandList().acquireTransientResource_RenderThread(resource);
	}

	FORCEINLINE void RHIDiscardTransientResource(RHIStructuredBuffer* resource)
	{
		RHICommandListExecutor::getImmediateCommandList().discardTransientResource_RenderThread(resource);
	}

	FORCEINLINE void RHIAcquireTransientResource(RHIVertexBuffer* resource)
	{
		RHICommandListExecutor::getImmediateCommandList().acquireTransientResource_RenderThread(resource);
	}

	FORCEINLINE void RHIDiscardTransientResource(RHIVertexBuffer* resource)
	{
		RHICommandListExecutor::getImmediateCommandList().discardTransientResource_RenderThread(resource);
	}

	FORCEINLINE void RHIAcquireTransientResource(RHITexture* resource)
	{
		RHICommandListExecutor::getImmediateCommandList().acquireTransientResource_RenderThread(resource);
	}

	FORCEINLINE void RHIDiscardTransientResource(RHITexture* resource)
	{
		RHICommandListExecutor::getImmediateCommandList().discardTransientResource_RenderThread(resource);
	}

	FORCEINLINE void* RHILockVertexBuffer(RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
	{
		return RHICommandListExecutor::getImmediateCommandList().lockVertexBuffer(vertexBuffer, offset, sizeRHI, lockMode);
	}

	FORCEINLINE void* RHILockIndexBuffer(RHIIndexBuffer* indexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode)
	{
		return RHICommandListExecutor::getImmediateCommandList().lockIndexBuffer(indexBuffer, offset, size, lockMode);
	}


}


#include "RHICommandList.inl"