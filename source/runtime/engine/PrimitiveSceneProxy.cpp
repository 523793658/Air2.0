#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
namespace Air
{

	PrimitiveSceneProxy::PrimitiveSceneProxy(const PrimitiveComponent* inComponent, wstring inResourceName)
		:bRenderInMainPass(inComponent->bRenderInMainPass)
		,mScene(inComponent->getScene())
		, mMobility(inComponent->mMobility)
		, bDrawInGame(inComponent->isVisible())
		, mPrimitiveSceneInfo(nullptr)
		, bDisableStaticPath(false)
		, bVFRequiresPrimitiveConstantBuffer(true)
	{

	}

	bool PrimitiveSceneProxy::isShown(const SceneView* view) const
	{
		if (!bDrawInGame)
		{
			return false;
		}
		return true;
	}
	PrimitiveViewRelevance PrimitiveSceneProxy::getViewRelevance(const SceneView* view) const
	{
		return PrimitiveViewRelevance();
	}
	void PrimitiveSceneProxy::setTransform(const Matrix& inLocalToWorld, const BoxSphereBounds& inBounds, const BoxSphereBounds& inLocalBounds, float3 inActorPosition)
	{
		BOOST_ASSERT(isInRenderingThread());
		mLocalToWorld = inLocalToWorld;
		mBounds = inBounds;
		mLocalBounds = inLocalBounds;
		mActorPosition = inActorPosition;
		updateConstantBufferMaybeLazy();
		OnTransformChanged();
	}

	void PrimitiveSceneProxy::updateConstantBufferMaybeLazy()
	{
		if(mPrimitiveSceneInfo)
		{
			mPrimitiveSceneInfo->setNeedsConstantBufferUpdate(true);
		}
		else
		{
			updateConstantBuffer();
		}
	}

	void PrimitiveSceneProxy::updateConstantBuffer()
	{
		if (doesVFRequirePrimitiveConstantBuffer())
		{
			bool bHasPrcomputedVolumetricLightmap;
			Matrix previousLocalToWorld;
			int32 singleCaptureIndex;
			bool bOutputVelocity;

			mScene->getPrimitiveConstantShaderParameters_RenderThread(mPrimitiveSceneInfo, bHasPrcomputedVolumetricLightmap, previousLocalToWorld, singleCaptureIndex, bOutputVelocity);

			BoxSphereBounds preSkinnedLocalBounds;
			//getpreskin

			const PrimitiveConstantShaderParameters primitiveConstantShaderParameters = getPrimitiveConstantShaderParameters(
				mLocalToWorld,
				previousLocalToWorld,
				mActorPosition,
				mBounds,
				mLocalBounds,
				bReceivesDecals
			);
			if (mConstantBuffer.getReference())
			{
				mConstantBuffer.updateConstantBufferImmediate(primitiveConstantShaderParameters);
			}
			else
			{
				mConstantBuffer = TConstantBufferRef<PrimitiveConstantShaderParameters>::createConstantBufferImmediate(primitiveConstantShaderParameters, ConstantBuffer_MultiFrame);
			}
		}

		if (mPrimitiveSceneInfo)
		{
			mPrimitiveSceneInfo->setNeedsConstantBufferUpdate(false);
		}
	}

	void PrimitiveSceneProxy::verifyUsedMaterial(const class MaterialRenderProxy* materialRenderProxy) const
	{

	}

	bool supportsCachingMeshDrawCommands(const PrimitiveSceneProxy* RESTRICT primtiveSceneProxy, const MeshBatch& meshBatch)
	{
		return
			(meshBatch.mElements.size() == 1) &&
			meshBatch.mVertexFactory->getType()->supportsCachingMeshDrawCommand() &&
			!primtiveSceneProxy->castsVolumetricTranslucentShadow();
	}

	bool supportsCachingMeshDrawCommands(const PrimitiveSceneProxy* RESTRICT primtiveSceneProxy, const MeshBatch& meshBatch, ERHIFeatureLevel::Type featureLevel)
	{
		if (supportsCachingMeshDrawCommands(primtiveSceneProxy, meshBatch))
		{
			const FMaterial* material = meshBatch.mMaterialRenderProxy->getMaterial(featureLevel);
			const MaterialShaderMap* shaderMap = material->getRenderingThreadShaderMap();
			if (shaderMap)
			{
				const ConstantExpressionSet& expressionSet = shaderMap->getConstantExpressionSet();
				if (expressionSet.hasExternalTextureExpressions())
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}
}