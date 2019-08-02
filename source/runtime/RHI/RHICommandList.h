#pragma once
#include "RHIConfig.h"
#include "RHIResource.h"
#include "Async/TaskGraphInterfaces.h"
#include "boost/noncopyable.hpp"
#include "Template/AlignOf.h"
#include "Misc/MemStack.h"
#include "DynamicRHI.h"
#include "Math/Float16Color.h"
#include "Containers/StaticArray.h"
namespace Air
{
	class IRHICommandContext;
	class IRHIComputeContext;
	struct RHICommandBase;

	enum class ECmdList
	{
		EGfx,
		ECompute,
	};



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

		template<typename T>
		FORCEINLINE_DEBUGGABLE void* alloc()
		{
			return alloc(sizeof(T), ALIGNOF(T));
		}

		template<typename TCmd>
		FORCEINLINE_DEBUGGABLE void* allocCommand()
		{
			BOOST_ASSERT(!isExecuting());
			TCmd* result = (TCmd*)alloc<TCmd>();
			++mNumCommands;
			*mCommandLink = result;
			mCommandLink = &result->mNext;
			return result;
		}

		void cacheActiveRenderTargets(uint32 newNumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargetsRHI, const RHIDepthRenderTargetView* newDepthStencilTargetRHI)
		{
			mCachedNumSimultanousRenderTargets = newNumSimultaneousRenderTargets;
			for (uint32 rtIdx = 0; rtIdx < mCachedNumSimultanousRenderTargets; ++rtIdx)
			{
				mCachedRenderTargets[rtIdx] = newRenderTargetsRHI[rtIdx];
			}
			mCachedDepthStencilTarget = (newDepthStencilTargetRHI) ? *newDepthStencilTargetRHI : RHIDepthRenderTargetView();
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

	protected:
		uint32 mNumCommands{ 0 };
		RHICommandBase* mRoot;
		RHICommandBase** mCommandLink;

		uint32 mUID;
		uint32 mCachedNumSimultanousRenderTargets;
		GraphEventArray mRTTasks;

		MemStackBase mMemManager;

		IRHICommandContext* mContext{ nullptr };
		IRHIComputeContext* mComputeContext{ nullptr };
		TStaticArray<RHIRenderTargetView, MaxSimultaneousRenderTargets> mCachedRenderTargets;
		RHIDepthRenderTargetView mCachedDepthStencilTarget;

		bool mExecuting{ false };

	

		friend class RHICommandListExecutor;
		friend class RHICommandListIterator;


	protected:
		static int32 mStateCacheEnabled;

		struct RHICommandSetDepthStencilState* mCachedDepthStencilState;
		struct RHICommandSetRasterizerState* mCachedRasterizerState;
		DrawUpData mDrawUPData;
	};

	struct RHICommandBase
	{
		RHICommandBase* mNext;

		void(*ExecuteAndDestructPtr)(RHICommandListBase& cmdList, RHICommandBase* cmd);

		void CallExecuteAndDestruct(RHICommandListBase& cmdList)
		{
			ExecuteAndDestructPtr(cmdList, this);
		}

		FORCEINLINE RHICommandBase(void(*inExecuteAndDestructPtr)(RHICommandListBase& cmdList, RHICommandBase* cmd))
			:mNext(nullptr)
			, ExecuteAndDestructPtr(inExecuteAndDestructPtr)
		{}
	};


	template <typename TCmd>
	struct RHICommand : public RHICommandBase
	{
		FORCEINLINE RHICommand()
			: RHICommandBase(&executeAndDestruct)
		{}

		static FORCEINLINE void executeAndDestruct(RHICommandListBase& cmdList, RHICommandBase* cmd)
		{
			TCmd* thisCmd = (TCmd*)cmd;
			thisCmd->execute(cmdList);
			thisCmd->~TCmd();
		}
	};

	struct RHICommandSetRasterizerState : public RHICommand<RHICommandSetRasterizerState>
	{
		RasterizerStateRHIParamRef mState;
		FORCEINLINE_DEBUGGABLE RHICommandSetRasterizerState(RasterizerStateRHIParamRef inState)
			:mState(inState)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetDepthStencilState : public RHICommand<RHICommandSetDepthStencilState>
	{
		DepthStencilStateRHIParamRef mState;
		uint32	mStencilRef;
		FORCEINLINE_DEBUGGABLE RHICommandSetDepthStencilState(DepthStencilStateRHIParamRef inState, uint32 stencilRef = 0)
			:mState(inState)
			,mStencilRef(stencilRef)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
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

	struct RHICommandTransitionTexturesArray : public RHICommand<RHICommandTransitionTexturesArray>
	{
		TArray<TextureRHIParamRef>& mTextures;
		EResourceTransitionAccess mTransitionType;
		FORCEINLINE_DEBUGGABLE RHICommandTransitionTexturesArray(EResourceTransitionAccess inTransitionType, TArray<TextureRHIParamRef>& inTextures)
			:mTextures(inTextures)
			, mTransitionType(inTransitionType)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandTransitionTextures : public RHICommand<RHICommandTransitionTextures>
	{
		static const int32 maxTexturesToTransition = 16;
		int32 numTextures;
		TextureRHIParamRef textures[maxTexturesToTransition];
		EResourceTransitionAccess transitionType;
		FORCEINLINE_DEBUGGABLE RHICommandTransitionTextures(EResourceTransitionAccess inTranstionType, TextureRHIParamRef* inTextures, int32 inNumTextures)
			:numTextures(inNumTextures),
			transitionType(inTranstionType)
		{
			for (int32 i = 0; i < numTextures; ++i)
			{
				textures[i] = inTextures[i];
			}
		}
		RHI_API void execute(RHICommandListBase& cmdList);
	};



	struct RHICommandBeginDrawingViewport : public RHICommand<RHICommandBeginDrawingViewport>
	{
		ViewportRHIParamRef mViewport;
		TextureRHIParamRef	mRenderTargetRHI;
		FORCEINLINE_DEBUGGABLE RHICommandBeginDrawingViewport(ViewportRHIParamRef inViewport, TextureRHIParamRef inRenderTargetRHI)
			:mViewport(inViewport),
			mRenderTargetRHI(inRenderTargetRHI)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandEndDrawingViewport : public RHICommand<RHICommandEndDrawingViewport>
	{
		ViewportRHIParamRef viewport;
		bool bPresent;
		bool bLockToVsync;

		FORCEINLINE_DEBUGGABLE RHICommandEndDrawingViewport(ViewportRHIParamRef inViewport, bool inPresent, bool inbLockToVsync)
			:viewport(inViewport),
			bPresent(inPresent),
			bLockToVsync(inbLockToVsync)
		{}

		RHI_API void execute(RHICommandListBase& CmdList);
	};
	struct RHICommandBeginScene : public RHICommand<RHICommandBeginScene>
	{
		FORCEINLINE_DEBUGGABLE RHICommandBeginScene()
		{}
		RHI_API void execute(RHICommandListBase& CmdList);
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
		UnorderedAccessViewRHIParamRef mUAVs[MaxSimultaneousUAVs];

		FORCEINLINE_DEBUGGABLE RHICommandSetRenderTargets(uint32 inNumSimultaneousRenderTargets, const RHIRenderTargetView* inNewRenderTargetsRHI, const RHIDepthRenderTargetView * inNewDepthStencilTarget, uint32 inNumUVAs, const UnorderedAccessViewRHIParamRef* inUAVs)
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

	struct RHICommandClearColorTextures : public RHICommand<RHICommandClearColorTextures>
	{
		LinearColor mColorArray[MaxSimultaneousRenderTargets];
		TextureRHIParamRef mTextures[MaxSimultaneousRenderTargets];
		IntRect mExcludeRect;
		int32 mNumClearColors;

		FORCEINLINE_DEBUGGABLE RHICommandClearColorTextures(
			int32 inNumClearColors,
			TextureRHIParamRef* inTextures,
			const LinearColor* inColorArray,
			IntRect inExcludeRect
		)
			:mExcludeRect(inExcludeRect)
			, mNumClearColors(inNumClearColors)
		{
			BOOST_ASSERT(inNumClearColors <= MaxSimultaneousRenderTargets);
			for (int32 index = 0; index < inNumClearColors; index++)
			{
				mTextures[index] = inTextures[index];
				mColorArray[index] = inColorArray[index];
			}
		}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandClearDepthStencilTexture : public RHICommand<RHICommandClearDepthStencilTexture>
	{
		TextureRHIParamRef mTexture;
		IntRect mExcludeRect;
		float mDepth;
		uint32 mStencil;
		EClearDepthStencil mClearDepthStencil;
		FORCEINLINE_DEBUGGABLE RHICommandClearDepthStencilTexture(
			TextureRHIParamRef inTexture,
			EClearDepthStencil inClearFlags,
			float depth,
			uint32 stencil,
			IntRect inExcludeRect
		)
			:mTexture(inTexture)
			, mClearDepthStencil(inClearFlags)
			, mDepth(depth)
			, mStencil(stencil)
			, mExcludeRect(inExcludeRect)
		{

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

	struct RHICommandSetBlendState : public RHICommand<RHICommandSetBlendState>
	{
		BlendStateRHIParamRef mState;
		LinearColor mBlendFactor;
		FORCEINLINE_DEBUGGABLE RHICommandSetBlendState(BlendStateRHIParamRef inState, const LinearColor& inBlendFactory)
			:mState(inState)
			,mBlendFactor(inBlendFactory)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetStreamSource : public RHICommand<RHICommandSetStreamSource> 
	{
		uint32 mStreamIndex;
		VertexBufferRHIParamRef mVertexBuffer;
		uint32 mStride;
		uint32 mOffset;
		FORCEINLINE_DEBUGGABLE RHICommandSetStreamSource(
			uint32 inStreamIndex,
			VertexBufferRHIParamRef inVertexBuffer,
			uint32 inStride,
			uint32 inOffset)
			:mStreamIndex(inStreamIndex),
			mVertexBuffer(inVertexBuffer),
			mStride(inStride),
			mOffset(inOffset)
		{

		}
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

	struct LocalBoundShaderStateWorkArea
	{
		BoundShaderStateInput mArgs;
		ComputedBSS* mComputedBSS;
#if DO_CHECK
		RHICommandListBase* mCheckCmdList;
		int32 UID;
#endif
		FORCEINLINE_DEBUGGABLE LocalBoundShaderStateWorkArea(
			RHICommandListBase* inCheckCmdList,
			VertexDeclarationRHIParamRef vertexDecalarationRHI,
			VertexShaderRHIParamRef vertexShaderRHI,
			HullShaderRHIParamRef hullShaderRHI,
			DomainShaderRHIParamRef domainShaderRHI,
			PixelShaderRHIParamRef pixelShaderRHI,
			GeometryShaderRHIParamRef geometryShaderRHI
		)
			:mArgs(vertexDecalarationRHI, vertexShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI, pixelShaderRHI)
#if DO_CHECK
			, mCheckCmdList(inCheckCmdList)
			, UID(0)
#endif
		{
			mComputedBSS = new (inCheckCmdList->alloc<ComputedBSS>())ComputedBSS;
		}
	};

	struct LocalBoundShaderState 
	{
		LocalBoundShaderStateWorkArea * mWorkArea;
		BoundShaderStateRHIRef mBypassBSS;
		LocalBoundShaderState()
			:mWorkArea(nullptr)
		{}
		LocalBoundShaderState(const LocalBoundShaderState& other)
			:mWorkArea(other.mWorkArea)
			,mBypassBSS(other.mBypassBSS)
		{}
	};

	struct LocalGraphicsPipelineStateWorkArea
	{

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

	struct RHICommandBuildLocalBoundShaderState : public RHICommand<RHICommandBuildLocalBoundShaderState>
	{
		LocalBoundShaderStateWorkArea mWorkArea;
		FORCEINLINE_DEBUGGABLE RHICommandBuildLocalBoundShaderState(RHICommandListBase* checkCmdList, VertexDeclarationRHIParamRef vertexDeclarationRHI, VertexShaderRHIParamRef vertexShaderRHI,
			HullShaderRHIParamRef hullShaderRHI,
			DomainShaderRHIParamRef domainShaderRHI,
			GeometryShaderRHIParamRef geometryShaderRHI,
			PixelShaderRHIParamRef pixelShaderRHI)
			:mWorkArea(checkCmdList, vertexDeclarationRHI, vertexShaderRHI, hullShaderRHI, domainShaderRHI, pixelShaderRHI, geometryShaderRHI)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};
	

	struct RHICommandSetLocalBoundShaderState : public RHICommand<RHICommandSetLocalBoundShaderState>
	{
		LocalBoundShaderState mLocalBoundShaderState;
		FORCEINLINE_DEBUGGABLE RHICommandSetLocalBoundShaderState(RHICommandListBase* checkCmdList, LocalBoundShaderState& inLocalBoundShaderState)
			:mLocalBoundShaderState(inLocalBoundShaderState)
		{
			mLocalBoundShaderState.mWorkArea->mComputedBSS->mUseCount++;
		}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandSetBoundShaderState : public RHICommand<RHICommandSetBoundShaderState>
	{
		BoundShaderStateRHIParamRef mBoundShaderState;
		FORCEINLINE_DEBUGGABLE RHICommandSetBoundShaderState(BoundShaderStateRHIParamRef inBoundShaderState)
			:mBoundShaderState(inBoundShaderState)
		{}
		RHI_API void execute(RHICommandListBase & cmdList);
	};

	template<typename TShaderRHIParamRef, ECmdList cmdListType>
	struct RHICommandSetShaderConstantBuffer : public RHICommand<RHICommandSetShaderConstantBuffer<TShaderRHIParamRef, cmdListType>>
	{
		TShaderRHIParamRef mShader;
		uint32 mBaseIndex;
		ConstantBufferRHIParamRef mConstantBuffer;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderConstantBuffer(TShaderRHIParamRef inShader, uint32 inBaseIndex, ConstantBufferRHIParamRef inConstantBuffer)
			:mShader(inShader),
			mBaseIndex(inBaseIndex),
			mConstantBuffer(inConstantBuffer)
		{
		}
		RHI_API void execute(RHICommandListBase & cmdList);
	};


	template<typename TShaderRHIParamRef>
	struct RHICommandSetLocalConstantBuffer : public RHICommand<RHICommandSetLocalConstantBuffer<TShaderRHIParamRef>>
	{
		TShaderRHIParamRef mShader;
		uint32 mBaseIndex;
		LocalConstantBuffer mLocalConstantBuffer;
		FORCEINLINE_DEBUGGABLE RHICommandSetLocalConstantBuffer(RHICommandListBase* checkCmdList, TShaderRHIParamRef inShader, uint32 inBaseIndex, const LocalConstantBuffer& inLocalConstantBuffer)
			:mShader(inShader),
			mBaseIndex(inBaseIndex),
			mLocalConstantBuffer(inLocalConstantBuffer)
		{
			mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->mUseCount++;
		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandEndDrawIndexedPrimitiveUP : public RHICommand<RHICommandEndDrawIndexedPrimitiveUP>
	{
		uint32 mPrimitiveType;
		uint32 mNumPrimitive;
		uint32 mNumVertices;
		uint32 mVertexDataStride;
		void* mOutVertexData;
		uint32 mMinVertexIndex;
		uint32 mNumIndices;
		uint32 mIndexDataStride;
		void* mOutIndexData;
		FORCEINLINE_DEBUGGABLE RHICommandEndDrawIndexedPrimitiveUP(uint32 inPrimitiveType, uint32 numPrimitive, uint32 numVertex, uint32 vertexDataStride, void* inOutVertexData, uint32 inMinVertexInddex, uint32 inNumIndices, uint32 inIndexDataStride, void* inOutIndexData)
			:mPrimitiveType(inPrimitiveType)
			,mNumPrimitive(numPrimitive)
			,mNumVertices(numVertex)
			,mVertexDataStride(vertexDataStride)
			,mOutVertexData(inOutVertexData)
			,mMinVertexIndex(inMinVertexInddex)
			,mNumIndices(inNumIndices)
			,mIndexDataStride(inIndexDataStride)
			,mOutIndexData(inOutIndexData)
		{

		}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandEndDrawPrimitiveUP : public RHICommand<RHICommandEndDrawPrimitiveUP>
	{
		uint32 mPrimitiveType;
		uint32 mNumPrimitives;
		uint32 mNumVertices;
		uint32 mVertexDataStride;
		void* mOutVertexData;

		RHICommandEndDrawPrimitiveUP(uint32 inPrimitiveType, uint32 inNumPrimitives, uint32 inNumVertices, uint32 inVertexDataStride, void* inOutVertexData)
			:mPrimitiveType(inPrimitiveType)
			,mNumPrimitives(inNumPrimitives)
			,mNumVertices(inNumVertices)
			,mVertexDataStride(inVertexDataStride)
			,mOutVertexData(inOutVertexData)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandDrawIndexedPrimitive : public RHICommand<RHICommandDrawIndexedPrimitive>
	{
		IndexBufferRHIParamRef mIndexBuffer;
		uint32	mPrimitiveType;
		uint32	mBaseVertexIndex;
		uint32	mFirstInstance;
		uint32	mNumVertex;
		uint32	mStartIndex;
		uint32	mNumPrimitives;
		uint32	mNumInstances;
		RHICommandDrawIndexedPrimitive(IndexBufferRHIParamRef inIndexBuffer, uint32 inPrimitiveType, uint32 inBaseVertexIndex, uint32 inFirstInstance, uint32 inNumVertex, uint32 inStartIndex, uint32 inNumPrimitive, uint32 inNumInstances)
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
		uint32 mPrimitiveType;
		uint32 mBaseVertexIndex;
		uint32 mNumPrimitives;
		uint32 mNumInstances;
		RHICommandDrawPrimitive(uint32 inPrimitiveType, uint32 inBaseVertexIndex, uint32 inNumPrimitives, uint32 inNumInstances)
			:mPrimitiveType(inPrimitiveType)
			,mBaseVertexIndex(inBaseVertexIndex)
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
		TextureRHIParamRef mSourceTexture;
		TextureRHIParamRef mDestTexture;
		bool bKeepOriginalSurface;

		FORCEINLINE_DEBUGGABLE RHICommandCopyToResolveTarget(TextureRHIParamRef sourceTexture, TextureRHIParamRef destTexture, bool bInKeepOriginalSurface, const ResolveParams& inResolveParams)
			:mResolveParams(inResolveParams)
			,mSourceTexture(sourceTexture)
			,mDestTexture(destTexture)
			,bKeepOriginalSurface(bInKeepOriginalSurface)
		{
			BOOST_ASSERT(sourceTexture);
			BOOST_ASSERT(destTexture);
			BOOST_ASSERT(sourceTexture->getTexture2D() || sourceTexture->getTexture3D() || sourceTexture->getTextureCube());
			BOOST_ASSERT(destTexture->getTexture2D() || destTexture->getTexture3D() || destTexture->getTextureCube());
		}

		RHI_API void execute(RHICommandListBase& CmdList);
	};

	

	template<typename TShaderRHIParamRef, ECmdList cmdListType>
	struct RHICommandSetShaderSampler : public RHICommand<RHICommandSetShaderSampler<TShaderRHIParamRef, cmdListType>>
	{
		TShaderRHIParamRef mShader;
		uint32 mSamplerIndex;
		SamplerStateRHIParamRef mState;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderSampler(TShaderRHIParamRef inShader, uint32 inSamplerIndex, SamplerStateRHIParamRef inState)
			:mShader(inShader)
			, mSamplerIndex(inSamplerIndex)
			, mState(inState)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TShaderRHIParamRef, ECmdList cmdListType>
	struct RHICommandSetShaderParameter : public RHICommand<RHICommandSetShaderParameter<TShaderRHIParamRef, cmdListType>>
	{
		TShaderRHIParamRef mShader;
		const void * newValue;
		uint32 mBufferIndex;
		uint32 mBaseIndex;
		uint32 mNumBytes;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderParameter(TShaderRHIParamRef inShader, uint32 inBufferIndex, uint32 inBaseIndex, uint32 inNumBytes, const void* inNewValue)
			:mShader(inShader)
			,mBufferIndex(inBufferIndex)
			,mBaseIndex(inBaseIndex)
			,mNumBytes(inNumBytes)
			,newValue(inNewValue)
		{}

		RHI_API void execute(RHICommandListBase& cmdList);
	};

	template<typename TShaderRHIParamRef, ECmdList cmdListType>
	struct RHICommandSetShaderTexture : public RHICommand<RHICommandSetShaderTexture<TShaderRHIParamRef, cmdListType>>
	{
		TShaderRHIParamRef mShader;
		uint32 mTextureIndex;
		TextureRHIParamRef mTexture;
		FORCEINLINE_DEBUGGABLE RHICommandSetShaderTexture(TShaderRHIParamRef inShader, uint32 inTextureIndex, TextureRHIParamRef inTexture)
			:mShader(inShader)
			,mTextureIndex(inTextureIndex)
			,mTexture(inTexture)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

	struct RHICommandUpdateTextureReference : public RHICommand<RHICommandUpdateTextureReference>
	{
		TextureReferenceRHIParamRef mTextureRef;
		TextureRHIParamRef mNewTexture;
		FORCEINLINE_DEBUGGABLE RHICommandUpdateTextureReference(
			TextureReferenceRHIParamRef inTextureRef, TextureRHIParamRef inNewTexture)
			:mTextureRef(inTextureRef)
			,mNewTexture(inNewTexture)
		{}
		RHI_API void execute(RHICommandListBase& cmdList);
	};

#define CMD_CONTEXT(Method) getContext().Method
#define COMPUTE_CONTEXT(Method) getComputeContext().Method
	

	class RHI_API RHICommandList : public RHICommandListBase
	{
	public:	
		void* operator new(size_t size);
		void operator delete(void* rawMemory);

		void beginDrawingViewport(ViewportRHIParamRef viewport, TextureRHIParamRef renderTargetRHI);

		void endDrawingViewport(ViewportRHIParamRef viewport, bool bPreset, bool bLockToVsync);

		FORCEINLINE_DEBUGGABLE void beginDrawIndexedPrimitiveUP(uint32 primitiveType, uint32 numPrimitive, uint32 numVertices, uint32 vertexDataStride, void*& outVertexData, uint32 minVertexIndex, uint32 numIndices, uint32 indexDataStride, void*& outIndexData)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIBeginDrawIndexedPrimitiveUP)(primitiveType, numPrimitive, numVertices, vertexDataStride, outVertexData, minVertexIndex, numIndices, indexDataStride, outIndexData);
				return;
			}
			BOOST_ASSERT(!mDrawUPData.mOutIndexData && !mDrawUPData.mOutIndexData && numVertices * vertexDataStride > 0 && numIndices * indexDataStride > 0);
			outVertexData = alloc(numVertices * vertexDataStride, 16);
			outIndexData = alloc(numIndices * indexDataStride, 16);
			mDrawUPData.mPrimitiveType = primitiveType;
			mDrawUPData.mNumPrimitives = numPrimitive;
			mDrawUPData.mNumVertices = numVertices;
			mDrawUPData.mVertexDataStride = vertexDataStride;
			mDrawUPData.mOutVertexData = outVertexData;
			mDrawUPData.mMinVertexIndex = minVertexIndex;
			mDrawUPData.mNumIndices = numIndices;
			mDrawUPData.mIndexDataStride = indexDataStride;
			mDrawUPData.mOutIndexData = outIndexData;
		}

		FORCEINLINE_DEBUGGABLE void endDrawIndexedPrimitiveUP()
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIEndDrawIndexedPrimitiveUP)();
				return;
			}
			BOOST_ASSERT(mDrawUPData.mOutVertexData && mDrawUPData.mOutIndexData && mDrawUPData.mNumIndices && mDrawUPData.mNumVertices);
			new (allocCommand<RHICommandEndDrawIndexedPrimitiveUP>())RHICommandEndDrawIndexedPrimitiveUP(mDrawUPData.mPrimitiveType, mDrawUPData.mNumPrimitives, mDrawUPData.mNumVertices, mDrawUPData.mVertexDataStride, mDrawUPData.mOutVertexData, mDrawUPData.mMinVertexIndex, mDrawUPData.mNumIndices, mDrawUPData.mIndexDataStride, mDrawUPData.mOutIndexData);
			mDrawUPData.mOutVertexData = nullptr;
			mDrawUPData.mOutIndexData = nullptr;
			mDrawUPData.mNumIndices = 0;
			mDrawUPData.mNumVertices = 0;
		}

		FORCEINLINE_DEBUGGABLE void beginDrawPrimitiveUP(uint32 primitiveType, uint32 numPrimitive, uint32 numVertex, uint32 vertexDataStride, void* & outVertexData)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIBeginDrawPrimitiveUP)(primitiveType, numPrimitive, numVertex, vertexDataStride, outVertexData);
			}
			BOOST_ASSERT(!mDrawUPData.mOutVertexData && numVertex * vertexDataStride > 0);
			outVertexData = alloc(numVertex * vertexDataStride, 16);
			mDrawUPData.mPrimitiveType = primitiveType;
			mDrawUPData.mNumPrimitives = numPrimitive;
			mDrawUPData.mNumVertices = numVertex;
			mDrawUPData.mVertexDataStride = vertexDataStride;
			mDrawUPData.mOutVertexData = outVertexData;
		}

		FORCEINLINE_DEBUGGABLE void endDrawPrimitiveUP()
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIEndDrawPrimitiveUP)();
				return;
			}
			BOOST_ASSERT(mDrawUPData.mOutVertexData && mDrawUPData.mNumVertices);
			new (allocCommand<RHICommandEndDrawPrimitiveUP>())RHICommandEndDrawPrimitiveUP(mDrawUPData.mPrimitiveType, mDrawUPData.mNumPrimitives, mDrawUPData.mNumVertices, mDrawUPData.mVertexDataStride, mDrawUPData.mOutVertexData);
			mDrawUPData.mOutVertexData = nullptr;
			mDrawUPData.mNumVertices = 0;
		}

		FORCEINLINE_DEBUGGABLE void drawIndexedPrimitive(IndexBufferRHIParamRef indexBuffer, uint32 primitiveType, int32 baseVertexIndex, int32 firstInstance, uint32 numVertices, uint32 startIndex, uint32 numPrimitive, uint32 numInstances)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIDrawIndexedPrimitive)(indexBuffer, primitiveType, baseVertexIndex, firstInstance, numVertices, startIndex, numPrimitive, numInstances);
				return;
			}
			new (allocCommand<RHICommandDrawIndexedPrimitive>())RHICommandDrawIndexedPrimitive(indexBuffer, primitiveType, baseVertexIndex, firstInstance, numVertices, startIndex, numPrimitive, numInstances);
		}

		FORCEINLINE_DEBUGGABLE void drawPrimitive(uint32 primitiveType, uint32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIDrawPrimitive)(primitiveType, baseVertexIndex, numPrimitives, numInstances);
				return;
			}
			new (allocCommand<RHICommandDrawPrimitive>())RHICommandDrawPrimitive(primitiveType, baseVertexIndex, numPrimitives, numInstances);
		}

		FORCEINLINE_DEBUGGABLE void setRasterizerState(RasterizerStateRHIParamRef state)
		{
			BOOST_ASSERT(StrictGraphicsPipelineStateUse == 0);
			if (bypass())
			{
				CMD_CONTEXT(RHISetRasterizerState)(state);
				return;
			}
			if (mStateCacheEnabled && mCachedRasterizerState && mCachedRasterizerState->mState == state)
			{
				return;
			}
			mCachedRasterizerState = new (allocCommand<RHICommandSetRasterizerState>())RHICommandSetRasterizerState(state);
		}

		FORCEINLINE_DEBUGGABLE void transitionResourceArrayNoCopy(EResourceTransitionAccess transitionType, TArray<TextureRHIParamRef>& inTextures)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHITransitionResources)(transitionType, &inTextures[0], inTextures.size());
				return;
			}
			new (allocCommand<RHICommandTransitionTexturesArray>()) RHICommandTransitionTexturesArray(transitionType, inTextures);
		}

		FORCEINLINE_DEBUGGABLE void transitionResource(EResourceTransitionAccess transitionType, TextureRHIParamRef inTexture)
		{
			TextureRHIParamRef texture = inTexture;
			if (bypass())
			{
				CMD_CONTEXT(RHITransitionResources)(transitionType, &texture, 1);
				return;
			}
			new (allocCommand<RHICommandTransitionTextures>())RHICommandTransitionTextures(transitionType, &texture, 1);
		}

		FORCEINLINE_DEBUGGABLE void setStreamSource(uint32 streadIndex, VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint32 offset)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetStreamSource)(streadIndex, vertexBuffer, stride, offset);
				return;
			}
			new (allocCommand<RHICommandSetStreamSource>())RHICommandSetStreamSource(streadIndex, vertexBuffer, stride, offset);
		}

		FORCEINLINE_DEBUGGABLE void transitionResources(EResourceTransitionAccess transitionType, TextureRHIParamRef* inTextures, int32 numTextures)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHITransitionResources)(transitionType, inTextures, numTextures);
				return;
			}
			new (allocCommand<RHICommandTransitionTextures>())RHICommandTransitionTextures(transitionType, inTextures, numTextures);
		}
	

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

		FORCEINLINE_DEBUGGABLE void setRenderTargets(uint32 newNumSimultanieusRenderTargets, const RHIRenderTargetView* newRenderTargetsRHI, const RHIDepthRenderTargetView* newDepthStencilTargetRHI, uint32 newNumUAVs, const UnorderedAccessViewRHIParamRef* UAVs)
		{
			cacheActiveRenderTargets(newNumSimultanieusRenderTargets, newRenderTargetsRHI, newDepthStencilTargetRHI);
			if (bypass())
			{
				CMD_CONTEXT(RHISetRenderTargets)(newNumSimultanieusRenderTargets, newRenderTargetsRHI, newDepthStencilTargetRHI, newNumUAVs, UAVs);
				return;
			}
			new (allocCommand<RHICommandSetRenderTargets>())RHICommandSetRenderTargets(newNumSimultanieusRenderTargets, newRenderTargetsRHI, newDepthStencilTargetRHI, newNumUAVs, UAVs);
		}

		FORCEINLINE_DEBUGGABLE void clearColorTexture(TextureRHIParamRef texture, const LinearColor& clearColor, IntRect excludeRect)
		{
			clearColorTextures(1, &texture, &clearColor, excludeRect);
		}

		FORCEINLINE_DEBUGGABLE void clearColorTextures(int32 numTextures, TextureRHIParamRef* textures, const LinearColor* clearColorArray, IntRect excludeRect)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIClearColorTextures)(numTextures, textures, clearColorArray, excludeRect);
				return;
			}
			new (allocCommand<RHICommandClearColorTextures>())RHICommandClearColorTextures(numTextures, textures, clearColorArray, excludeRect);
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

		FORCEINLINE_DEBUGGABLE void setBlendState(BlendStateRHIParamRef state, const LinearColor& blendFactor = LinearColor::White)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetBlendState)(state, blendFactor);
				return;
			}
			new (allocCommand<RHICommandSetBlendState>())RHICommandSetBlendState(state, blendFactor);
		}

		FORCEINLINE_DEBUGGABLE void setDepthStencilState(DepthStencilStateRHIParamRef state, uint32 stecilRef = 0)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetDepthStencilState)(state, stecilRef);
				return;
			}
			if (mStateCacheEnabled && mCachedDepthStencilState && mCachedDepthStencilState->mState == state && mCachedDepthStencilState->mStencilRef == stecilRef)
			{
				return;
			}
			mCachedDepthStencilState = new (allocCommand<RHICommandSetDepthStencilState>())RHICommandSetDepthStencilState(state, stecilRef);
		}

		FORCEINLINE_DEBUGGABLE void setStencilRef(uint32 stencilRef)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetStencilRef)(stencilRef);
				return;
			}
			if (mStateCacheEnabled && mCachedDepthStencilState && mCachedDepthStencilState->mStencilRef == stencilRef)
			{
				return;
			}
			new (allocCommand<RHICommandSetStencilRef>())RHICommandSetStencilRef(stencilRef);
		}
		FORCEINLINE_DEBUGGABLE void setViewport(uint32 x, uint32 y, float z, uint32 width, uint32 height, float depth)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetViewport)(x, y, z, width, height, depth);
				return;
			}
			new (allocCommand<RHICommandSetViewport>())RHICommandSetViewport(x, y, z, width, height, depth);
		}

		FORCEINLINE_DEBUGGABLE void setStereoViewport(uint32 leftX, uint32 leftY, float z, uint32 width, uint32 height, float depth)
		{}

		FORCEINLINE_DEBUGGABLE LocalBoundShaderState buildLocalBoundShaderState(
			VertexDeclarationRHIParamRef vertexDecalarationRHI,
			VertexShaderRHIParamRef vertexShaderRHI,
			HullShaderRHIParamRef hullShaderRHI,
			DomainShaderRHIParamRef domainShaderRHI,
			GeometryShaderRHIParamRef geometryShaderRHI,
			PixelShaderRHIParamRef pixelShaderRHI
		)
		{
			LocalBoundShaderState result;
			if (bypass())
			{
				result.mBypassBSS = RHICreateBoundShaderState(vertexDecalarationRHI, vertexShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI, pixelShaderRHI);
			}
			else
			{
				auto* cmd = new (allocCommand<RHICommandBuildLocalBoundShaderState>())RHICommandBuildLocalBoundShaderState(this, vertexDecalarationRHI, vertexShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI, pixelShaderRHI);
				result.mWorkArea = &cmd->mWorkArea;
			}
			return result;
		}

		FORCEINLINE_DEBUGGABLE void setLocalBoundShaderState(LocalBoundShaderState localBoundShaderState)
		{
			BOOST_ASSERT(StrictGraphicsPipelineStateUse == 0);
			if (bypass())
			{
				CMD_CONTEXT(RHISetBoundShaderState)(localBoundShaderState.mBypassBSS);
				return;
			}
			new (allocCommand<RHICommandSetLocalBoundShaderState>())RHICommandSetLocalBoundShaderState(this, localBoundShaderState);
		}

		FORCEINLINE_DEBUGGABLE void buildAndSetLocalBoundShaderState(const BoundShaderStateInput& boundShaderStateInput)
		{
			setLocalBoundShaderState(buildLocalBoundShaderState(boundShaderStateInput.mVertexDeclarationRHI, boundShaderStateInput.mVertexShaderRHI, boundShaderStateInput.mHullShaderRHI, boundShaderStateInput.mDomainShaderRHI, boundShaderStateInput.mGeometryShaderRHI, boundShaderStateInput.mPixelShaderRHI));
		}

		void applyCachedRenderTargets(GraphicsPipelineStateInitializer & GraphicsPSOInit)
		{
			GraphicsPSOInit.mRenderTargetsEnabled = mCachedNumSimultanousRenderTargets;
			for (uint32 i = 0; i < GraphicsPSOInit.mRenderTargetsEnabled; ++i)
			{
				if (mCachedRenderTargets[i].mTexture)
				{
					GraphicsPSOInit.mRenderTargetFormats[i] = mCachedRenderTargets[i].mTexture->getFormat();
					GraphicsPSOInit.mRenderTargetFlags[i] = mCachedRenderTargets[i].mTexture->getFlags();
				}
				else
				{
					GraphicsPSOInit.mRenderTargetFormats[i] = PF_Unknown;
				}
				GraphicsPSOInit.mRenderTargetLoadActions[i] = mCachedRenderTargets[i].mLoadAction;
				GraphicsPSOInit.mRenderTargetStoreActions[i] = mCachedRenderTargets[i].mStoreAction;
				if (GraphicsPSOInit.mRenderTargetFormats[i] != PF_Unknown)
				{
					GraphicsPSOInit.mNumSamples = mCachedRenderTargets[i].mTexture->getNumSamples();
				}
			}
			if (mCachedDepthStencilTarget.mTexture)
			{
				GraphicsPSOInit.mDepthStencilTargetFormat = mCachedDepthStencilTarget.mTexture->getFormat();
				GraphicsPSOInit.mDepthStencilTargetFlags = mCachedDepthStencilTarget.mTexture->getFlags();
			}
			else
			{
				GraphicsPSOInit.mDepthStencilTargetFormat = PF_Unknown;
			}
			GraphicsPSOInit.mDepthStencilTargetLoadAction = mCachedDepthStencilTarget.mDepthLoadAction;
			GraphicsPSOInit.mDepthStencilTargetStoreAction = mCachedDepthStencilTarget.mDepthStoreAction;

			if (GraphicsPSOInit.mDepthStencilTargetFormat != PF_Unknown)
			{
				GraphicsPSOInit.mNumSamples = mCachedDepthStencilTarget.mTexture->getNumSamples();
			}
		}

		template<typename TShaderRHIParamRef>
		FORCEINLINE_DEBUGGABLE void setLocalShaderConstantBuffer(TShaderRHIParamRef shader, uint32 baseIndex, const LocalConstantBuffer& constantBuffer)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderConstantBuffer)(shader, baseIndex, constantBuffer.mBypassConstant);
				return;
			}
			new (allocCommand<RHICommandSetLocalConstantBuffer<TShaderRHIParamRef>>())RHICommandSetLocalConstantBuffer<TShaderRHIParamRef>(this, shader, baseIndex, constantBuffer);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setLocalShaderConstantBuffer(TRefCountPtr<TShaderRHI>& shader, uint32 baseIndex, const LocalConstantBuffer& constantBuffer)
		{
			setLocalShaderConstantBuffer(shader.getInitReference(), baseIndex, constantBuffer);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderConstantBuffer(TShaderRHI* shader, uint32 baseIndex, ConstantBufferRHIParamRef constantBuffer)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderConstantBuffer)(shader, baseIndex, constantBuffer);
				return;
			}
			new (allocCommand<RHICommandSetShaderConstantBuffer<TShaderRHI*, ECmdList::EGfx>>())RHICommandSetShaderConstantBuffer<TShaderRHI*, ECmdList::EGfx>(shader, baseIndex, constantBuffer);
		}
		template<typename TShaderRHI>
		FORCEINLINE void setShaderConstantBuffer(TRefCountPtr<TShaderRHI>& shader, uint32 baseIndex, ConstantBufferRHIParamRef constantBuffer)
		{
			setShaderConstantBuffer(shader.getReference(), baseIndex, constantBuffer);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderParameter(TShaderRHI * shader, uint32 BufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderParameter)(shader, BufferIndex, baseIndex, numBytes, newValue);
				return;
			}
			void * useValue = alloc(numBytes, 16);
			Memory::memcpy(useValue, newValue, numBytes);
			new (allocCommand<RHICommandSetShaderParameter<TShaderRHI*, ECmdList::EGfx>>())RHICommandSetShaderParameter<TShaderRHI*, ECmdList::EGfx>(shader, BufferIndex, baseIndex, numBytes, useValue);
		}

		template<typename TShaderRHI>
		FORCEINLINE void setShaderParameter(TRefCountPtr<TShaderRHI>& shader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* value)
		{
			setShaderParameter(shader.getReference(), bufferIndex, baseIndex, numBytes, value);
		}


		FORCEINLINE_DEBUGGABLE void setBoundShaderState(BoundShaderStateRHIParamRef boundShaderState)
		{
			BOOST_ASSERT(StrictGraphicsPipelineStateUse == 0);
			if (bypass())
			{
				CMD_CONTEXT(RHISetBoundShaderState)(boundShaderState);
				return;
			}
			new (allocCommand < RHICommandSetLocalBoundShaderState>())RHICommandSetBoundShaderState(boundShaderState);
		}

		template<typename TShaderRHIParamRef>
		FORCEINLINE_DEBUGGABLE void setShaderTexture(TShaderRHIParamRef shader, uint32 textureIndex, TextureRHIParamRef texture)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderTexture)(shader, textureIndex, texture);
				return;
			}
			new (allocCommand<RHICommandSetShaderTexture<TShaderRHIParamRef, ECmdList::EGfx>>())RHICommandSetShaderTexture<TShaderRHIParamRef, ECmdList::EGfx>(shader, textureIndex, texture);
		}
		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderTexture(TRefCountPtr<TShaderRHI>& shader, uint32 textureIndex, TextureRHIParamRef texture)
		{
			setShaderTexture(shader.getReference(), textureIndex, texture);
		}

		template<typename TShaderRHIParamRef>
		FORCEINLINE_DEBUGGABLE void setShaderSampler(TShaderRHIParamRef shader, uint32 samplerIndex, SamplerStateRHIParamRef state)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHISetShaderSampler)(shader, samplerIndex, state);
				return;
			}

			new (allocCommand<RHICommandSetShaderSampler<TShaderRHIParamRef, ECmdList::EGfx>>())RHICommandSetShaderSampler<TShaderRHIParamRef, ECmdList::EGfx>(shader, samplerIndex, state);
		}

		template<typename TShaderRHI>
		FORCEINLINE_DEBUGGABLE void setShaderSampler(TRefCountPtr<TShaderRHI> shader, uint32 samplerIndex, SamplerStateRHIParamRef state)
		{
			setShaderSampler(shader.getReference(), samplerIndex, state);
		}

		FORCEINLINE_DEBUGGABLE void copyToResolveTarget(TextureRHIParamRef sourteTExtureRHI, TextureRHIParamRef destTextureRHI, bool bKeepOriginalSurface, const ResolveParams& resolveParams)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHICopyToResolveTarget)(sourteTExtureRHI, destTextureRHI, bKeepOriginalSurface, resolveParams);
				return;
			}

			new (allocCommand<RHICommandCopyToResolveTarget>())RHICommandCopyToResolveTarget(sourteTExtureRHI, destTextureRHI, bKeepOriginalSurface, resolveParams);
		}

		FORCEINLINE_DEBUGGABLE void clearDepthStencilTexture(TextureRHIParamRef texture, EClearDepthStencil clearDepthStencil, float depth, uint32 stencil, IntRect excludeRect)
		{
			if (bypass())
			{
				CMD_CONTEXT(RHIClearDepthStencilTexture)(texture, clearDepthStencil, depth, stencil, excludeRect);
				return;
			}
			new(allocCommand<RHICommandClearDepthStencilTexture>())RHICommandClearDepthStencilTexture(texture, clearDepthStencil, depth, stencil, excludeRect);
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

		void beginFrame();
		void endFrame();
		void beginScene();
		void endScene();
	};

	

	class RHI_API RHICommandListImmediate : public RHICommandList
	{
	public:
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
			return GDynamicRHI->RHICreateTexture2D_RenderThread(*this, width, height, format, numMips, numSamplers, flags, createInfo);
		}

		FORCEINLINE TextureCubeRHIRef createTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)
		{
			return GDynamicRHI->RHICreateTextureCube_RenderThread(*this, size, format, numMips, flags, createInfo);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint8 mipLevel)
		{
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, texture2DRHI, mipLevel);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint8 mipLevel, uint8 numMipLevel, uint8 format)
		{
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, texture2DRHI, mipLevel, numMipLevel, format);
		}

		FORCEINLINE ShaderResourceViewRHIRef createShaderResourceView(VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint8 format)
		{
			return GDynamicRHI->RHICreateShaderResourceView_RenderThread(*this, vertexBuffer, stride, format);
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

		FORCEINLINE GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream)
		{
			return GDynamicRHI->RHICreateGeometryShaderWithStreamOutput(code, elementList, numStrides, strides, rasterizedStream);
		}

		FORCEINLINE void* lockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, EResourceLockMode lockMode, uint32 & destStride, bool bLockWithinMiptail, bool bFlushRHIThread = true)
		{
			return GDynamicRHI->lockTexture2D_RenderThread(*this, texture, mipIndex, lockMode, destStride, bLockWithinMiptail, bFlushRHIThread);
		}

		FORCEINLINE void unLockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, bool bLockWithinMiptail, bool bFlushRHIThread = false)
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

		FORCEINLINE void unlockVertexBuffer(VertexBufferRHIParamRef vertexBuffer)
		{
			GDynamicRHI->unlockVertexBuffer_RenderThread(*this, vertexBuffer);
		}

		FORCEINLINE IndexBufferRHIRef createAndLockIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void* & outDataBuffer)
		{
			return GDynamicRHI->createAndLockIndexBuffer_RenderThread(*this, stride, size, inUsage, createInfo, outDataBuffer);
		}

		FORCEINLINE void* lockIndexBuffer(IndexBufferRHIParamRef indexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
		{
			return GDynamicRHI->lockIndexBuffer_RenderThread(*this, indexBuffer, offset, sizeRHI, lockMode);
		}

		FORCEINLINE void unlockIndexBuffer(IndexBufferRHIParamRef indexBuffer)
		{
			GDynamicRHI->unlockIndexBuffer_RenderThread(*this, indexBuffer);
		}

		FORCEINLINE VertexDeclarationRHIRef createVertexDeclaration(const VertexDeclarationElementList& elements)
		{
			return GDynamicRHI->createVertexDeclaration_RenderThread(*this, elements);
		}

		FORCEINLINE void* lockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)
		{
			immediateFlush(EImmediateFlushType::FlushRHIThread);
			return GDynamicRHI->RHILockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, lockMode, destStride, bLockWithinMiptail);
		}

		FORCEINLINE void unlockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail)
		{
			immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIUnlockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, bLockWithinMiptail);
		}

		FORCEINLINE TextureReferenceRHIRef createTextureReference(LastRenderTimeContainer* lastRenderTime)
		{
			return GDynamicRHI->RHICreateTextureReference(lastRenderTime);
		}

		FORCEINLINE void readSurfaceFloatData(TextureRHIParamRef texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex)
		{
			immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIReadSurfaceFloatData(texture, rect, outData, cubeface, arrayIndex, mipIndex);
		}

		void updateTextureReference(TextureReferenceRHIParamRef textureRef, TextureRHIParamRef newTexture);
	};

	class RHI_API RHIAsyncComputeCommandList : public RHICommandListBase
	{
	public:
		FORCEINLINE_DEBUGGABLE void setShaderSampler(ComputeShaderRHIParamRef shader, uint32 samplerIndex, SamplerStateRHIParamRef state)
		{
			if (bypass())
			{
				COMPUTE_CONTEXT(RHISetShaderSampler)(shader, samplerIndex, state);
				return;
			}
			new(allocCommand<RHICommandSetShaderSampler<ComputeShaderRHIParamRef, ECmdList::ECompute>>())RHICommandSetShaderSampler<ComputeShaderRHIParamRef, ECmdList::ECompute>(shader, samplerIndex, state);
		}

		FORCEINLINE_DEBUGGABLE void setShaderParameter(ComputeShaderRHIParamRef shader, uint32 BufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
		{
			if (bypass())
			{
				COMPUTE_CONTEXT(RHISetShaderParameter)(shader, BufferIndex, baseIndex, numBytes, newValue);
				return;
			}
			void * useValue = alloc(numBytes, 16);
			Memory::memcpy(useValue, newValue, numBytes);
			new (allocCommand<RHICommandSetShaderSampler<ComputeShaderRHIParamRef, ECmdList::ECompute>>())RHICommandSetShaderParameter<ComputeShaderRHIParamRef, ECmdList::ECompute>(shader, BufferIndex, baseIndex, numBytes, useValue);
		}

		FORCEINLINE_DEBUGGABLE void setShaderTexture(ComputeShaderRHIParamRef shader, uint32 textureIndex, TextureRHIParamRef tex)
		{
			if (bypass())
			{
				COMPUTE_CONTEXT(RHISetShaderTexture)(shader, textureIndex, tex);
				return;
			}
			new (allocCommand<RHICommandSetShaderTexture<ComputeShaderRHIParamRef, ECmdList::ECompute>>())RHICommandSetShaderTexture<ComputeShaderRHIParamRef, ECmdList::ECompute>(shader, textureIndex, tex);
		}
	};

	class RHI_API RHIAsyncComputeCommandListImmediate : public RHIAsyncComputeCommandList
	{

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

		static void waitOnRHIThreadFence(GraphEventRef& fence);

		static void checkNoOutstandingCmdLists();

	private:
		static void executeInner_DoExecute(RHICommandListBase& cmlList);
	private:
		friend class RHICommandListBase;
		friend class ExecuteRHIThreadTask;
		bool bLatchedBypass;
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

	
	


	

	extern RHI_API RHICommandListExecutor GRHICommandList;

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
	
	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint8 mipLevel)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(texture2DRHI, mipLevel);
	}

	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint8 mipLevel, uint8 numMipLevel, uint8 format)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(texture2DRHI, mipLevel, numMipLevel, format);
	}

	FORCEINLINE ShaderResourceViewRHIRef RHICreateShaderResourceView(VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint8 format)
	{
		return RHICommandListExecutor::getImmediateCommandList().createShaderResourceView(vertexBuffer, stride, format);
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

	FORCEINLINE void* RHILockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail, bool bFulshRHIThread = true)
	{
		return RHICommandListExecutor::getImmediateCommandList().lockTexture2D(texture, mipIndex, lockMode, destStride, bLockWithinMiptail, bFulshRHIThread);
	}

	FORCEINLINE void RHIUnlockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, bool bLockWithinMiptail, bool bFlushRHIThread = true)
	{
		RHICommandListExecutor::getImmediateCommandList().unLockTexture2D(texture, mipIndex, bLockWithinMiptail, bFlushRHIThread);
	}
	FORCEINLINE VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createVertexBuffer(size, inUsage, createInfo);
	}

	FORCEINLINE IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return RHICommandListExecutor::getImmediateCommandList().createIndexBuffer(stride, size, inUsage, createInfo);
	}

	FORCEINLINE VertexBufferRHIRef RHICreateAndLockVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer)
	{
		return RHICommandListExecutor::getImmediateCommandList().createAndLockVertexBuffer(size, inUsage, createInfo, outDataBuffer);
	}
	FORCEINLINE void RHIUnlockVertexBuffer(VertexBufferRHIParamRef vertexBuffer)
	{
		RHICommandListExecutor::getImmediateCommandList().unlockVertexBuffer(vertexBuffer);
	}

	FORCEINLINE IndexBufferRHIRef RHICreateAndLockIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer)
	{
		return RHICommandListExecutor::getImmediateCommandList().createAndLockIndexBuffer(stride, size, inUsage, createInfo, outDataBuffer);
	}

	FORCEINLINE void RHIUnlockIndexBuffer(IndexBufferRHIParamRef indexBuffer)
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

	FORCEINLINE void RHIUpdateTextureReference(TextureReferenceRHIParamRef textureRHI, TextureRHIParamRef newTexture)
	{
		RHICommandListExecutor::getImmediateCommandList().updateTextureReference(textureRHI, newTexture);
	}

	FORCEINLINE void* RHILockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)
	{
		return RHICommandListExecutor::getImmediateCommandList().lockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, lockMode, destStride, bLockWithinMiptail);
	}


	FORCEINLINE void RHIUnlockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail)
	{
		RHICommandListExecutor::getImmediateCommandList().unlockTextureCubeFace(texture, faceIndex, arrayIndex, mipIndex, bLockWithinMiptail);
	}

	FORCEINLINE TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime)
	{
		return RHICommandListExecutor::getImmediateCommandList().createTextureReference(lastRenderTime);
	}
}


#include "RHICommandList.inl"