#include "DrawingPolicy.h"
#include "RHIUtilities.h"
namespace Air
{
	DrawingPolicyRenderState::DrawingPolicyRenderState(RHICommandList* inDebugResetRHICmdList, const SceneView& sceneView)
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

	void MeshDrawingPolicy::drawMesh(RHICommandList& rhiCmdList, const MeshBatch& mesh, int32 batchElementIndex, const bool bIsInstancedStereo /* = false */) const 
	{
		const MeshBatchElement& batchElement = mesh.mElements[batchElementIndex];
		if (mesh.bUseDynamicData)
		{
			BOOST_ASSERT(mesh.mDynamicVertexData);
			if (batchElement.mDynamicIndexData)
			{
				drawIndexedPrimitiveUP(
					rhiCmdList,
					mesh.mType,
					batchElement.mMinVertexIndex,
					batchElement.mMaxVertexIndex - batchElement.mMinVertexIndex + 1,
					batchElement.mNumPrimitives,
					batchElement.mDynamicIndexData,
					batchElement.mDynamicIndexStride,
					mesh.mDynamicVertexData,
					mesh.mDynamicVertexStride
				);
			}
			else
			{
				drawPrimitiveUP(rhiCmdList, mesh.mType, batchElement.mNumPrimitives, mesh.mDynamicVertexData, mesh.mDynamicVertexStride);
			}
		}
		else
		{
			if (batchElement.mIndexBuffer)
			{
				BOOST_ASSERT(batchElement.mIndexBuffer->isInitialized());
				if (batchElement.bIsInstanceRuns)
				{
					BOOST_ASSERT(batchElement.bIsInstanceRuns);
					if (!GRHISupportsFirstInstance)
					{

					}
					else
					{
						for (uint32 run = 0; run < batchElement.mNumInstances; run++)
						{
							rhiCmdList.drawIndexedPrimitive(
								batchElement.mIndexBuffer->mIndexBufferRHI
								, mesh.mType
								, 0
								, batchElement.mInstantceRuns[run * 2]
								, batchElement.mMaxVertexIndex - batchElement.mMinVertexIndex + 1
								, batchElement.mFirstIndex
								, batchElement.mNumPrimitives
								, 1 + batchElement.mInstantceRuns[run * 2 + 1] - batchElement.mInstantceRuns[run * 2]);
						}
					}
				}
				else
				{
					const uint32 instanceCount = (bIsInstancedStereo && !batchElement.bIsInstancedMesh) ? 2 : batchElement.mNumInstances;
					rhiCmdList.drawIndexedPrimitive(batchElement.mIndexBuffer->mIndexBufferRHI, mesh.mType, 
						0, 0,
						batchElement.mMaxVertexIndex - batchElement.mMinVertexIndex + 1,
						batchElement.mFirstIndex,
						batchElement.mNumPrimitives,
						instanceCount);
				}
			}
			else
			{
				rhiCmdList.drawPrimitive(mesh.mType, batchElement.mFirstIndex, batchElement.mNumPrimitives, batchElement.mNumInstances);
			}
		}
	}

	const VertexDeclarationRHIRef& MeshDrawingPolicy::getVertexDeclaration() const
	{
		BOOST_ASSERT(mVertexFactory && mVertexFactory->isInitialized());
		const VertexDeclarationRHIRef& vertexDeclaration = mVertexFactory->getDeclaration();
		BOOST_ASSERT(isValidRef(vertexDeclaration));
		return vertexDeclaration;
	}

	MeshDrawingPolicy::MeshDrawingPolicy(const VertexFactory* inVertexFactory, const MaterialRenderProxy* inMaterialRenderProxy, const FMaterial& inMaterialResource, const MeshDrawingPolicyOverrideSettings & inOverrideSettings, EDebugViewShaderMode inDebugViewShaderMode)
		:mVertexFactory(inVertexFactory)
		,mMaterialRenderProxy(inMaterialRenderProxy)
		,mMaterialResource(&inMaterialResource)
		,mMeshPrimitvieType(inOverrideSettings.mMeshPrimitiveType),
		bIsDitheredLODTransitionMaterial(false),
		mDebugViewShaderMode((uint32)inDebugViewShaderMode)
	{
		bool bMaterialResourceIsTwoSided = inMaterialResource.isTwoSided();
		const bool bIsWireframeMaterial = inMaterialResource.isWireFrame() || !!(inOverrideSettings.mMeshOverrideFlags & EDrawingPolicyOverrideFlags::Wireframe);
		mMeshFillMode = bIsWireframeMaterial ? FM_Wireframe : FM_Solid;
		const bool bInTwoSideedOverride = !!(inOverrideSettings.mMeshOverrideFlags & EDrawingPolicyOverrideFlags::TwoSided);
		const bool bInReverseCullModeOverride = !!(inOverrideSettings.mMeshOverrideFlags & EDrawingPolicyOverrideFlags::ReverseCullMode);
		const bool bIsTwoSided = (bMaterialResourceIsTwoSided || bInTwoSideedOverride);
		const bool bMeshRenderTwoSided = bIsTwoSided || bInTwoSideedOverride;
		mMeshCullMode = (bMeshRenderTwoSided) ? CM_None : (bInReverseCullModeOverride ? CM_CCW : CM_CW);

		bUsePositionOnlyVS = false;
	}
}