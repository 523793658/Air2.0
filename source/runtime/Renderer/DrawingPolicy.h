#pragma once
#include "CoreMinimal.h"
#include "SceneView.h"
#include "Misc/EnumClassFlags.h"
#include "MeshBatch.h"
#include "DebugViewModeHelpers.h"
#include "RHIStaticStates.h"
#include "Containers/BitArray.h"
#include "sceneRendering.h"
namespace Air
{
	class RHICommandList;

#define COMPAREDRAWINGPOLICYMEMBERS(MemberName) \
	if(A.MemberName < B.MemberName){return -1;}	\
	else if(A.MemberName > B.MemberName){return +1;}


	enum class EDrawingPolicyOverrideFlags
	{
		None = 0,
		TwoSided = 1 << 0,
		DitheredLODTransition = 1 << 1,
		Wireframe = 1 << 2,
		ReverseCullMode = 1 << 3,
	};
	ENUM_CLASS_FLAGS(EDrawingPolicyOverrideFlags);

	class DrawingPolicyMatchResult
	{
	public:
		DrawingPolicyMatchResult()
#if		!(BUILD_SHIPPING || BUILD_TEST)
			:mMatches(0)
#else
			:bLastResult(false)
#endif
		{}

		bool append(const DrawingPolicyMatchResult& result, const TCHAR* condition)
		{
			bLastResult = result.bLastResult;
#if !(BUILD_SHIPPING || BUILD_TEST)
			TestResults = result.TestResults;
			mTestCondition = result.mTestCondition;
			mMatches = result.mMatches;
#endif
			return bLastResult;
		}

		bool append(bool result, const TCHAR* condition)
		{
			bLastResult = result;
#if !(BUILD_SHIPPING || BUILD_TEST)
			TestResults.add(result);
			mTestCondition.add(condition);
			mMatches += result;
#endif
			return bLastResult;
		}

		bool result() const
		{
			return bLastResult;
		}

		int32 matchCount()const
		{
#if !(BUILD_SHIPPING || BUILD_TEST)
			return mMatches;
#else
			return 0;
#endif
		}




		uint32 bLastResult : 1;
#if !(BUILD_SHIPPING || BUILD_TEST)
		int32 mMatches;
		TBitArray<>		TestResults;
		TArray<const TCHAR*> mTestCondition;
#endif
	};

#define DRAWING_POLICY_MATCH_BEGIN	DrawingPolicyMatchResult result;{
#define DRAWING_POLICY_MATCH(matchExp)	result.append((matchExp), TEXT(#matchExp))
#define DRAWING_POLICY_MATCH_END	} return result;

	struct DrawingPolicyRenderState
	{
		DrawingPolicyRenderState() = delete;
		DrawingPolicyRenderState(RHICommandList* inDebugResetRHICmdList, const SceneView& sceneView)
			:mBlendState(nullptr)
			, mDepthStencilState(nullptr)
			, mViewConstantBuffer(sceneView.mViewConstantBuffer)
			, mStencilRef(0)
			, mViewOverrideFlags(EDrawingPolicyOverrideFlags::None)
			, mDitheredLODTransitionAlpha(0.0f)
		{
			mViewOverrideFlags |= sceneView.bReverseCulling ? EDrawingPolicyOverrideFlags::ReverseCullMode : EDrawingPolicyOverrideFlags::None;
			mViewOverrideFlags |= sceneView.bRenderSceneTwoSided ?
				EDrawingPolicyOverrideFlags::TwoSided :
				EDrawingPolicyOverrideFlags::None;
		}
		DrawingPolicyRenderState(const DrawingPolicyRenderState& drawRenderState) = delete;

		FORCEINLINE_DEBUGGABLE DrawingPolicyRenderState(RHICommandList* inDebugResetRHICmdList, const DrawingPolicyRenderState& drawRenderState)
			:mBlendState(drawRenderState.mBlendState)
			,mDepthStencilState(drawRenderState.mDepthStencilState)
			,mViewConstantBuffer(drawRenderState.mViewConstantBuffer)
			,mStencilRef(drawRenderState.mStencilRef)
			,mViewOverrideFlags(drawRenderState.mViewOverrideFlags)
			,mDitheredLODTransitionAlpha(drawRenderState.mDitheredLODTransitionAlpha)
		{}

		~DrawingPolicyRenderState()
		{

		}

	public:

		FORCEINLINE_DEBUGGABLE void setBlendState(RHICommandList& RHICmdList, BlendStateRHIParamRef inBlendState)
		{
			mBlendState = inBlendState;
			if (!RHICmdList.StrictGraphicsPipelineStateUse)
			{
				RHICmdList.setBlendState(inBlendState);
			}
		}
		FORCEINLINE_DEBUGGABLE const BlendStateRHIParamRef getBlendState() const
		{
			return mBlendState;
		}
		FORCEINLINE_DEBUGGABLE void setDepthStencilState(RHICommandList& RHICmdList, DepthStencilStateRHIParamRef inDepthStencilState, uint32 inStencilRef = 0)
		{
			mDepthStencilState = inDepthStencilState;
			mStencilRef = inStencilRef;
			if (!RHICmdList.StrictGraphicsPipelineStateUse)
			{
				RHICmdList.setDepthStencilState(inDepthStencilState, inStencilRef);
			}
		}

		FORCEINLINE_DEBUGGABLE const DepthStencilStateRHIParamRef getDepthStencilState() const
		{
			return mDepthStencilState;
		}

		FORCEINLINE_DEBUGGABLE void setViewConstantBuffer(const TConstantBufferRef<ViewConstantShaderParameters>& inViewConstantBuffer)
		{
			mViewConstantBuffer = inViewConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE const TConstantBufferRef<ViewConstantShaderParameters>& getViewConstantBuffer() const
		{
			return mViewConstantBuffer;
		}

		FORCEINLINE_DEBUGGABLE uint32 getStencilRef() const
		{
			return mStencilRef;
		}

		FORCEINLINE_DEBUGGABLE void setDitheredLODTransitionAlpha(float inDitheredLODTransitionAlpha)
		{
			mDitheredLODTransitionAlpha = inDitheredLODTransitionAlpha;
		}
		FORCEINLINE_DEBUGGABLE EDrawingPolicyOverrideFlags & modifyViewOverrideFlags()
		{
			return mViewOverrideFlags;
		}

		FORCEINLINE_DEBUGGABLE EDrawingPolicyOverrideFlags getViewOverrideFlags() const
		{
			return mViewOverrideFlags;
		}
	private:
		BlendStateRHIParamRef			mBlendState;
		DepthStencilStateRHIParamRef	mDepthStencilState;
		TConstantBufferRef<ViewConstantShaderParameters>	mViewConstantBuffer;
		uint32 mStencilRef;
		EDrawingPolicyOverrideFlags		mViewOverrideFlags;
		float							mDitheredLODTransitionAlpha;
	};

	struct MeshDrawingPolicyOverrideSettings
	{
		EDrawingPolicyOverrideFlags mMeshOverrideFlags = EDrawingPolicyOverrideFlags::None;
		EPrimitiveType mMeshPrimitiveType = PT_TriangleList;
	};

	class RENDERER_API MeshDrawingPolicy
	{
	public:
		struct ElementDataType {};

		struct ContextDataType 
		{
			bool bIsInstancedStereo;

			ContextDataType(const bool inbIsInstancedStereo) :
				bIsInstancedStereo(inbIsInstancedStereo) {}
			ContextDataType() : bIsInstancedStereo(false) {};

		};

		MeshDrawingPolicy(
			const VertexFactory* inVertexFactory,
			const MaterialRenderProxy* inMaterialRenderProxy,
			const FMaterial& inMaterialResource,
			const MeshDrawingPolicyOverrideSettings & inOverrideSettings,
			EDebugViewShaderMode inDebugViewShaderMode);

		const VertexDeclarationRHIRef& getVertexDeclaration() const;

		friend int32 compareDrawingPolicy(const MeshDrawingPolicy& A, const MeshDrawingPolicy& B)
		{
			//COMPAREDRAWINGPOLICYMEMBERS(mVertexFactory);
			//COMPAREDRAWINGPOLICYMEMBERS(mMaterialRenderProxy);
			//COMPAREDRAWINGPOLICYMEMBERS(bIsDitheredLODTransitionMaterial);
			return 0;
		}

		void drawMesh(RHICommandList& rhiCmdList, const MeshBatch& mesh, int32 batchElementIndex, const bool bIsInstancedStereo = false) const;

		DrawingPolicyMatchResult matches(const MeshDrawingPolicy& otherDrawer) const
		{
			DRAWING_POLICY_MATCH_BEGIN
				DRAWING_POLICY_MATCH(mVertexFactory == otherDrawer.mVertexFactory) &&
				DRAWING_POLICY_MATCH(mMaterialRenderProxy == otherDrawer.mMaterialRenderProxy) &&
				DRAWING_POLICY_MATCH(bUsePositionOnlyVS == otherDrawer.bUsePositionOnlyVS) &&
				DRAWING_POLICY_MATCH(mMeshFillMode == otherDrawer.mMeshFillMode) &&
				DRAWING_POLICY_MATCH(mMeshCullMode == otherDrawer.mMeshCullMode) &&
				DRAWING_POLICY_MATCH(mMeshPrimitvieType == otherDrawer.mMeshPrimitvieType);
			DRAWING_POLICY_MATCH_END
		}

		size_t getTypeHash() const
		{
			return pointerHash(mVertexFactory, pointerHash(mMaterialRenderProxy));
		}

		static FORCEINLINE_DEBUGGABLE ERasterizerCullMode inverseCullMode(ERasterizerCullMode cullMode)
		{
			return cullMode == CM_None ? CM_None : (cullMode == CM_CCW ? CM_CW : CM_CCW);
		}

		FORCEINLINE_DEBUGGABLE RasterizerStateRHIParamRef computeRasterizeState(EDrawingPolicyOverrideFlags inOverrideFlags) const
		{
			const bool bReverseCullMode = !!(inOverrideFlags & EDrawingPolicyOverrideFlags::ReverseCullMode);
			const bool bRenderSceneTwoSided = !!(inOverrideFlags & EDrawingPolicyOverrideFlags::TwoSided);
			ERasterizerCullMode localCullMode = bRenderSceneTwoSided ? CM_None: bReverseCullMode ? inverseCullMode(mMeshCullMode) : mMeshCullMode;
			return getStaticRasterizerState<true>(mMeshFillMode, localCullMode);
		}

		FORCEINLINE_DEBUGGABLE void setMeshRenderState(RHICommandList& RHICmdList,
			const ViewInfo& view,
			const PrimitiveSceneProxy* primitiveSceneProxy,
			const MeshBatch& mesh,
			int32 batchElementIndex,
			DrawingPolicyRenderState& drawRenderState,
			const ElementDataType& elementData,
			const ContextDataType& policyContext)const
		{
			RHICmdList.setRasterizerState(computeRasterizeState(drawRenderState.getViewOverrideFlags()));
		}

#if !(BUILD_SHIPPING || BUILD_TEST)
		FORCEINLINE EDebugViewShaderMode getDebugViewShaderMode() const { return (EDebugViewShaderMode)mDebugViewShaderMode; }

		FORCEINLINE bool useDebugViewPS()const { return mDebugViewShaderMode != DVSM_None; }
#else
		FORCEINLINE EDebugViewShaderMode getDebugViewShaderMode() const { return DVSM_None; }

		FORCEINLINE bool useDebugViewPS()const { return false; }
#endif

	protected:
		const VertexFactory* mVertexFactory;
		const MaterialRenderProxy* mMaterialRenderProxy;
		const FMaterial* mMaterialResource;

		ERasterizerFillMode mMeshFillMode;
		ERasterizerCullMode mMeshCullMode;
		EPrimitiveType mMeshPrimitvieType;
		uint32 bIsDitheredLODTransitionMaterial : 1;
		uint32 bUsePositionOnlyVS : 1;
		uint32 mDebugViewShaderMode : 6;
	};

	

	FORCEINLINE_DEBUGGABLE MeshDrawingPolicyOverrideSettings computeMeshOverrideSettings(const MeshBatch& mesh)
	{
		MeshDrawingPolicyOverrideSettings overrideSettings;
		overrideSettings.mMeshPrimitiveType = (EPrimitiveType)mesh.mType;
		overrideSettings.mMeshOverrideFlags |= mesh.bDisableBackfaceCulling ? EDrawingPolicyOverrideFlags::TwoSided : EDrawingPolicyOverrideFlags::None;
		return overrideSettings;
	}
}