#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
namespace Air
{

	PrimitiveSceneProxy::PrimitiveSceneProxy(const PrimitiveComponent* inComponent, wstring inResourceName)
		:bRenderInMainPass(inComponent->bRenderInMainPass)
	{
		beginInitResource(&mConstantBuffer);

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
		const PrimitiveConstantShaderParameters primitiveConstantShaderParameters = getPrimitiveConstantShaderParameters(mLocalToWorld, mActorPosition, mBounds, mLocalBounds, false);
		mConstantBuffer.setContents(primitiveConstantShaderParameters);
		if (mPrimitiveSceneInfo)
		{
			mPrimitiveSceneInfo->setNeedsConstantBufferUpdate(false);
		}
	}

	void PrimitiveSceneProxy::verifyUsedMaterial(const class MaterialRenderProxy* materialRenderProxy) const
	{

	}
}