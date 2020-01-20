#pragma once
#include "CoreMinimal.h"
#include "RHIResource.h"
#include "SceneViewBuffer.h"
#include "ShaderParameterMacros.h"
#include "SceneView.h"
#include "VertexFactory.h"
#include "MeshDrawShaderBindings.h"
#include "Containers/ChunkedArray.h"
#include "RendererInterface.h"
namespace Air
{
	namespace EMeshPass
	{
		enum Type
		{
			DepthPass,
			BasePass,
			CSMShadowDepth,
			Distortion,
			Velocity,
			TranslucencyStandard,
			TranslucencyAfterDOF,
			TranslucencyAll,
			LightmapDensity,
			DebugViewMode,
			CustomDepth,
			MobileBasePassCSM,
			MobileInverseOpacity,
			VirtualTexture,

			Num,
			NumBits = 5,
		};
	}

	class ShaderBindingState;

	class MeshPassMask
	{
	public:
		MeshPassMask()
			:mData(0)
		{}

		void set(EMeshPass::Type pass) {
			mData |= (1 << pass);
		}

		bool get(EMeshPass::Type pass) const
		{
			return !!(mData & (1 << pass));
		}

		void appendTo(MeshPassMask& mask) const { mask.mData |= mData; }

		void reset() { mData = 0; }

		bool isEmpty() const { return mData == 0; }

		uint32 mData;
	};

	struct MeshPassProcessorRenderState
	{
		MeshPassProcessorRenderState(const SceneView& sceneView, RHIConstantBuffer* inPassConstantBuffer = nullptr)
			:mBlendState(nullptr)
			,mDepthStencilState(nullptr)
			,mDepthStencilAccess(FExclusiveDepthStencil::DepthRead_StencilRead)
			,mViewConstantBuffer(sceneView.mViewConstantBuffer)
			,mInstancedViewConstantBuffer()
			,mReflectionCaptureConstantBuffer()
			,mPassConstantBuffer(inPassConstantBuffer)
			,mStencilRef(0)
		{}

		MeshPassProcessorRenderState(const TConstantBufferRef<ViewConstantShaderParameters>& inViewConstantBuffer, RHIConstantBuffer* inPassConstantBuffer)
			:mBlendState(nullptr)
			,mDepthStencilState(nullptr)
			,mDepthStencilAccess(FExclusiveDepthStencil::DepthRead_StencilRead)
			,mViewConstantBuffer(inViewConstantBuffer)
			,mInstancedViewConstantBuffer()
			,mReflectionCaptureConstantBuffer()
			,mPassConstantBuffer(inPassConstantBuffer)
			,mStencilRef(0)
		{

		}

		MeshPassProcessorRenderState()
			:mBlendState(nullptr)
			,mDepthStencilState(nullptr)
			,mViewConstantBuffer()
			,mInstancedViewConstantBuffer()
			,mReflectionCaptureConstantBuffer()
			,mPassConstantBuffer(nullptr)
			,mStencilRef(0)
		{}

		FORCEINLINE_DEBUGGABLE MeshPassProcessorRenderState(const MeshPassProcessorRenderState& drawRenderState)
			:mBlendState(drawRenderState.mBlendState)
			,mDepthStencilState(drawRenderState.mDepthStencilState)
			,mDepthStencilAccess(drawRenderState.mDepthStencilAccess)
			,mViewConstantBuffer(drawRenderState.mViewConstantBuffer)
			,mInstancedViewConstantBuffer(drawRenderState.mInstancedViewConstantBuffer)
			,mReflectionCaptureConstantBuffer(drawRenderState.mReflectionCaptureConstantBuffer)
			,mPassConstantBuffer(drawRenderState.mPassConstantBuffer)
			,mStencilRef(drawRenderState.mStencilRef)
		{

		}

		~MeshPassProcessorRenderState()
		{}

	public:
		FORCEINLINE_DEBUGGABLE void setBlendState(RHIBlendState* inBlendState)
		{
			mBlendState = inBlendState;
		}

		FORCEINLINE_DEBUGGABLE RHIBlendState* getBlendState() const
		{
			return mBlendState;
		}

		FORCEINLINE_DEBUGGABLE void setDepthStencilState(RHIDepthStencilState* inDepthStencilState)
		{
			mDepthStencilState = inDepthStencilState;
			mStencilRef = 0;
		}

		FORCEINLINE_DEBUGGABLE void setStencilRef(uint32 inStencilRef)
		{
			mStencilRef = inStencilRef;
		}

		FORCEINLINE_DEBUGGABLE RHIDepthStencilState* getDepthStencilState() const
		{
			return mDepthStencilState;
		}

		FORCEINLINE_DEBUGGABLE void setDepthStencilAccess(FExclusiveDepthStencil::Type inDepthStencilAccess)
		{
			mDepthStencilAccess = inDepthStencilAccess;
		}

		FORCEINLINE_DEBUGGABLE FExclusiveDepthStencil::Type getDepthStencilAccess() const
		{
			return mDepthStencilAccess;
		}

		FORCEINLINE_DEBUGGABLE void setViewConstantBuffer(const TConstantBufferRef<ViewConstantShaderParameters>& inViewConstantBuffer)
		{
			mViewConstantBuffer = inViewConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE const TConstantBufferRef<ViewConstantShaderParameters>& getViewConstantBuffer() const
		{
			return mViewConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE void setInstancedViewConstantBuffer(const TConstantBufferRef<InstancedViewConstantShaderParameters>& inViewConstantBuffer)
		{
			mInstancedViewConstantBuffer = inViewConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE const TConstantBufferRef<InstancedViewConstantShaderParameters>& getInstancedViewConstantBuffer() const
		{
			return mInstancedViewConstantBuffer.isValid() ? mInstancedViewConstantBuffer : reinterpret_cast<const TConstantBufferRef<InstancedViewConstantShaderParameters>&>(mViewConstantBuffer);
		}

		FORCEINLINE_DEBUGGABLE void setReflectionCaptureConstantBuffer(RHIConstantBuffer* inConstantBuffer)
		{
			mReflectionCaptureConstantBuffer = inConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE const ConstantBufferRHIRef& getReflectionCaptureConstantBuffer() const
		{
			return mReflectionCaptureConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE void setPassConstantBuffer(const ConstantBufferRHIRef& inPassConstantBuffer)
		{
			mPassConstantBuffer = inPassConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE RHIConstantBuffer* getPassConstantBuffer() const
		{
			return mPassConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE uint32 getStencilRef() const
		{
			return mStencilRef;
		}

		FORCEINLINE_DEBUGGABLE void applyToPSO(GraphicsPipelineStateInitializer& graphicsPOSInit) const
		{
			graphicsPOSInit.mBlendState = mBlendState;
			graphicsPOSInit.mDepthStencilState = mDepthStencilState;
		}

	private:
		RHIBlendState* mBlendState;
		RHIDepthStencilState* mDepthStencilState;
		FExclusiveDepthStencil::Type mDepthStencilAccess;

		TConstantBufferRef<ViewConstantShaderParameters> mViewConstantBuffer;
		TConstantBufferRef<InstancedViewConstantShaderParameters> mInstancedViewConstantBuffer;

		ConstantBufferRHIRef	mReflectionCaptureConstantBuffer;
		RHIConstantBuffer* mPassConstantBuffer;
		uint32				mStencilRef;
	};
	
	const int32 NumInlineShaderBindings = 10;

	class MeshDrawShaderBindings
	{
	public:
		MeshDrawShaderBindings() {}
		MeshDrawShaderBindings(const MeshDrawShaderBindings& other)
		{
			
		}

	private:
		TArray<MeshDrawShaderBindingsLayout, TInlineAllocator<2>> mShaderLayouts;

		union
		{
			uint8 mInlineStorage[NumInlineShaderBindings * sizeof(void*)];
			uint8* mHeapData = nullptr;
		};

		uint16 mSize = 0;

		RENDERER_API void copyFrom(const MeshDrawShaderBindings& other);

		ENGINE_API void release();

		void allocate(uint16 inSize)
		{
			BOOST_ASSERT(mSize == 0 && mHeapData == nullptr);
			mSize = inSize;
			if (inSize > ARRAY_COUNT(mInlineStorage))
			{
				mHeapData = new uint8[inSize];
			}
		}

		void allocateZeroed(uint32 inSize)
		{
			allocate(inSize);
			BOOST_ASSERT(mSize == inSize);
			PlatformMemory::Memzero(getData(), inSize);
		}


		const uint8* getData() const 
		{
			return mSize <= ARRAY_COUNT(mInlineStorage) ? &mInlineStorage[0] : mHeapData;
		}

		uint8* getData() 
		{
			return mSize <= ARRAY_COUNT(mInlineStorage) ? &mInlineStorage[0] : mHeapData;
		}
		template<class RHIShaderType>
		static void setShaderBindings(
			RHICommandList& RHICmdList,
			RHIShaderType shader,
			const class ReadOnlyMeshDrawSingleShaderBindings& RESTRICT singleShaderBindings,
			ShaderBindingState& RESTRICT shaderBindingState);

		template<class RHIShaderType>
		static void setShaderBindings(RHICommandList& RHICmdList, RHIShaderType shader, const class ReadOnlyMeshDrawSingleShaderBindings& RESTRICT singleShaderBings);
	};

	class GraphicsMinimalPipelineStateId
	{

	};

	typedef TSet<GraphicsMinimalPipelineStateInitializer> GraphicsMinimalPipelineStateSet;

	class MeshDrawCommand
	{
	public:
		MeshDrawShaderBindings mShaderBindings;

		VertexInputStreamArray mVertexStreams;

		RHIIndexBuffer* mIndexBuffer;

		GraphicsMinimalPipelineStateId mCachePipelineId;

		uint32 mFirstIndex;

		uint32 mNumPrimitives;

		uint32 mNumInstances;

		union
		{
			struct
			{
				uint32 mBaseVertexIndex;
				uint32 mNumVertices;
			}VertexParams;

			struct
			{
				RHIVertexBuffer* mBuffer;
				uint32 mOffset;
			}IndirectArgs;
		};

		uint8 mPrimitiveIdStreamIndex;

		uint8 mStencilRef;


	};

	class RENDERER_API MeshDrawCommandSortKey
	{
	public:
		union
		{
			uint64 mPackedData;
			struct
			{
				uint64 mVertexShaderHash : 16;
				uint64 mPixelShaderHash : 32;
				uint64 mMasked : 16;
			}mBasePass;

			struct
			{
				uint64 mMeshIdInPrimitive : 16;
				uint64 mDistance : 32;
				uint64 mPriority : 16;
			}Translucent;

			struct
			{
				uint64 mVertexShaderHash : 32;
				uint64 mPixelShaderHash : 32;
			}Generic;
		};

		FORCEINLINE bool operator != (MeshDrawCommandSortKey b) const
		{
			return mPackedData != b.mPackedData;
		}

		FORCEINLINE bool operator < (MeshDrawCommandSortKey b) const
		{
			return mPackedData < b.mPackedData;
		}

		static const MeshDrawCommandSortKey mDefault;
	};

	class MeshMaterialShader;

	struct MeshProcessorShaders
	{
		mutable MeshMaterialShader* mVertexShader;
		mutable MeshMaterialShader* mNullShader;
		mutable MeshMaterialShader* mDomainShader;
		mutable MeshMaterialShader* mGeometryShader;
		mutable MeshMaterialShader* mPixelShader;
		mutable MeshMaterialShader* mComputeShader;
#if RHI_RAYTRACING
		mutable MeshMaterialShader* mRayHitGroupShader;
#endif

		MeshMaterialShader* getShader(EShaderFrequency frequency) const
		{
			if (frequency == SF_Vertex)
			{
				return mVertexShader;
			}
			else if (frequency == SF_Hull)
			{
				return mNullShader;
			}
			else if (frequency == SF_Domain)
			{
				return mDomainShader;
			}
			else if (frequency == SF_Geometry)
			{
				return mGeometryShader;
			}
			else if (frequency == SF_Pixel)
			{
				return mPixelShader;
			}
			else if (frequency == SF_Compute)
			{
				return mComputeShader;
			}
#if RHI_RAYTRACING
			else if (frequency == SF_RayHitGroup)
			{
				return mRayHitGroupShader;
			}
#endif
			BOOST_ASSERT(false);
			return nullptr;
		}
	};

	class MeshPassDrawListContext
	{
	public:
		virtual ~MeshPassDrawListContext() {}

		virtual MeshDrawCommand& addCommand(const MeshDrawCommand& initializer) = 0;

		virtual void finalizeCommand(
			const MeshBatch& meshBatch,
			int32 batchElementIndex,
			int32 drawPrimitiveId,
			int32 scenePrimitiveId,
			ERasterizerFillMode meshFillMode,
			ERasterizerCullMode meshCullMode,
			MeshDrawCommandSortKey sortKey,
			const GraphicsMinimalPipelineStateInitializer& pipelineState,
			const MeshProcessorShaders* shadersForDebugging,
			MeshDrawCommand& meshDrawCommand) = 0;
	};

	class MeshPassProcessor
	{
	public:
		virtual void addMeshBatch(const MeshBatch& RESTRICT meshBatch, uint64 batchElementMask, const PrimitiveSceneProxy* RESTRICT primitiveSceneProxy, int32 staticMeshId = -1) = 0;

	public:
		const Scene* RESTRICT mScene;
		ERHIFeatureLevel::Type mFeatureLevel;
		const SceneView* mViewIfDynamicMeshCommand;
		MeshPassDrawListContext* mDrawListContext;



	};

	class CachedMeshDrawCommandInfo
	{
	public:
		explicit CachedMeshDrawCommandInfo():
			mSortKey(MeshDrawCommandSortKey::mDefault),
			mCommandIndex(-1),
			mStateBucketId(-1),
			mMeshPass(EMeshPass::Num),
			mMeshFillMode(ERasterizerFillMode_Num),
			mMeshCullMode(ERasterizerCullMode_Num)
		{}
		MeshDrawCommandSortKey mSortKey;

		int32 mCommandIndex;

		int32 mStateBucketId;

		EMeshPass::Type mMeshPass : EMeshPass::NumBits + 1;

		ERasterizerFillMode mMeshFillMode : ERasterizerFillMode_NumBits + 1;
		ERasterizerCullMode mMeshCullMode : ERasterizerCullMode_NumBits + 1;
	};


	typedef MeshPassProcessor* (*PassProcessorCreateFunction)(const Scene* scene, const SceneView* inViewIfDynamicMeshCommand, MeshPassDrawListContext* inDrawListContext);

	enum class EMeshPassFlags
	{
		None = 0,
		CachedMeshCommands = 1 << 0,
		MainView = 1 << 1
	};
	ENUM_CLASS_FLAGS(EMeshPassFlags);


	class PassProcessorManager
	{
	public:
		static PassProcessorCreateFunction getCreateFunction(EShadingPath shadingPath, EMeshPass::Type passType)
		{
			BOOST_ASSERT(shadingPath < EShadingPath::Num && passType < EMeshPass::Num);
			uint32 shadingPathIdx = (uint32)shadingPath;
			BOOST_ASSERT(mJumpTable[shadingPathIdx][passType]);
			return mJumpTable[shadingPathIdx][passType];
		}

		static EMeshPassFlags getPassFlags(EShadingPath shadingPath, EMeshPass::Type passType)
		{
			BOOST_ASSERT(shadingPath < EShadingPath::Num && passType < EMeshPass::Num);
			uint32 shadingPathIdx = (uint32)shadingPath;
			return mFlags[shadingPathIdx][passType];
		}


	private:
		ENGINE_API static PassProcessorCreateFunction mJumpTable[(uint32)EShadingPath::Num][EMeshPass::Num];
		ENGINE_API static EMeshPassFlags mFlags[(uint32)EShadingPath::Num][EMeshPass::Num];
		friend class RegisterPassProcessorCreateFunction;
	};

	class RegisterPassProcessorCreateFunction
	{
	public:
		RegisterPassProcessorCreateFunction(PassProcessorCreateFunction createFunction, EShadingPath inShadingPath, EMeshPass::Type inPassType, EMeshPassFlags inPassFlags)
			:mShadingPath(inShadingPath)
			, mPassType(inPassType)
		{
			uint32 shadingPathIdx = (uint32)mShadingPath;
			PassProcessorManager::mJumpTable[shadingPathIdx][mPassType] = createFunction;
			PassProcessorManager::mFlags[shadingPathIdx][mPassType] = inPassFlags;
		}


		~RegisterPassProcessorCreateFunction()
		{
			uint32 shadingPathIdx = (uint32)mShadingPath;
			PassProcessorManager::mJumpTable[shadingPathIdx][mPassType] = nullptr;
			PassProcessorManager::mFlags[shadingPathIdx][mPassType] = EMeshPassFlags::None;
		}
	private:
		EShadingPath mShadingPath;
		EMeshPass::Type mPassType;
	};

	class CachedPassMeshDrawList
	{
	public:
		CachedPassMeshDrawList():
			mLowestFreeIndexSearchStart(0)
		{}

		TSparseArray<MeshDrawCommand> mMeshDrawCommands;
		int32 mLowestFreeIndexSearchStart;
	};


	class DynamicMeshDrawCommandStorage
	{
	public:
		TChunkedArray<MeshDrawCommand> mMeshDrawCommands;
	};

	class CachedPassMeshDrawListContext : public MeshPassDrawListContext
	{
	public:
		CachedPassMeshDrawListContext(CachedMeshDrawCommandInfo& inCommandInfo, CachedPassMeshDrawList& inDrawList, Scene& inScene);

		virtual MeshDrawCommand& addCommand(const MeshDrawCommand& initializer) override final;

		virtual void finalizeCommand(const MeshBatch& meshBatch, int32 batchElementIndex, int32 drawPrimitiveId, int32 scenePrimitiveId, ERasterizerFillMode meshFillMode, ERasterizerCullMode meshCullMode, MeshDrawCommandSortKey sortKey, const GraphicsMinimalPipelineStateInitializer& pipelineState, const MeshProcessorShaders* shadersForDebugging, MeshDrawCommand& meshDrawCommand) override final;

	private:
		MeshDrawCommand mMeshDrawCommandForStateBucketing;
		CachedMeshDrawCommandInfo& mCommandInfo;
		CachedPassMeshDrawList& mDrawList;
		Scene& mScene;
		bool bUseStateBuckets;
	};


	class VisibleMeshDrawCommand
	{
	public:
		FORCEINLINE_DEBUGGABLE void setup(
			const MeshDrawCommand* inMeshDrawCommand,
			int32 inDrawPrimitiveId,
			int32 inScenePrimitiveId,
			int32 inStateBucketId,
			ERasterizerFillMode inMeshFillMode,
			ERasterizerCullMode inMeshCullMode,
			MeshDrawCommandSortKey inSortKey
		)
		{
			mMeshDrawCommand = inMeshDrawCommand;
			mDrawPrimitiveId = inDrawPrimitiveId;
			mScenePrimitiveId = inScenePrimitiveId;
			mPrimitiveIdBufferOffset = -1;
			mStateBucketId = inStateBucketId;
			mMeshFillMode = inMeshFillMode;
			mMeshCullMode = inMeshCullMode;
			mSortKey = inSortKey;
		}

		const MeshDrawCommand* mMeshDrawCommand;

		MeshDrawCommandSortKey mSortKey;

		int32 mDrawPrimitiveId;

		int32 mScenePrimitiveId;

		int32 mPrimitiveIdBufferOffset;

		int32 mStateBucketId;

		ERasterizerFillMode mMeshFillMode : ERasterizerFillMode_NumBits + 1;

		ERasterizerCullMode mMeshCullMode : ERasterizerFillMode_NumBits + 1;
	};

	typedef TArray<VisibleMeshDrawCommand, SceneRenderingAllocator> MeshCommandOneFrameArray;

	extern void submitMeshDrawCommandsRange(
		const MeshCommandOneFrameArray& visibleMeshDrawCommand,
		const GraphicsMinimalPipelineStateSet& graphicsMinimalPipelineStateSet,
		RHIVertexBuffer* primitiveIdsBuffer,
		int32 basePrimitiveIdsOffset,
		bool bDynamicInstancing,
		int32 startIndex,
		int32 numMeshDrawCommands,
		uint32 instanceFactor,
		RHICommandList& RHICmdList);
}