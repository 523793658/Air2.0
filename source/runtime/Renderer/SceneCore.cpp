#include "SceneCore.h"
#include "ScenePrivate.h"
#include "PrimitiveSceneInfo.h"
namespace Air
{
	void StaticMesh::unlinkDrawList(StaticMesh::DrawListElementLink* link)
	{
		BOOST_ASSERT(isInRenderingThread());
		BOOST_ASSERT(mDrawListLinks.removeSingleSwap(link) == 1);
	}

	void StaticMesh::linkDrawList(DrawListElementLink* link)
	{
		BOOST_ASSERT(isInRenderingThread());
		BOOST_ASSERT(!mDrawListLinks.contains(link));
		mDrawListLinks.push_back(link);
	}

	void StaticMesh::addToDrawList(RHICommandListImmediate& RHICmdList, Scene* scene)
	{
		const auto featureLevel = scene->getFeatureLevel();
		if (bCastShadow)
		{
			
		}
		if (!mPrimitiveSceneInfo->mProxy->shouldRenderInMainPass())
		{
			return;
		}

		if (isTranslucent(featureLevel))
		{
			return;
		}

		if (scene->getShadingPath() == EShadingPath::Deferred)
		{
			if (bUseAsOccluder)
			{

			}
			if (bUseForMaterial)
			{
				BasePassOpaqueDrawingPolicyFactory::addStaticMesh(RHICmdList, scene, this);

			}
		}
	}


	void StaticMesh::removeFromDrawList()
	{
		while (mDrawListLinks.size())
		{
			StaticMesh::DrawListElementLink*link = mDrawListLinks[0];
			const int32 originalNumLinks = mDrawListLinks.size();
			link->remove(true);
			if (mDrawListLinks.size())
			{
				BOOST_ASSERT(mDrawListLinks[0] != link);
			}
		}
	}
}