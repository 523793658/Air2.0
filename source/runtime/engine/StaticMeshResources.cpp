#include "StaticMeshResources.h"
#include "Classes/Components/StaticMeshComponent.h"
namespace Air
{
	StaticMeshSceneProxy::StaticMeshSceneProxy(StaticMeshComponent* component, bool bCanLODsShareStaticLighting)
		:PrimitiveSceneProxy(component, component->getStaticMesh()->getName())
		,mOwner(component->getOwner())
		,mStaticMesh(component->getStaticMesh())
		,mRenderData(component->getStaticMesh()->mRenderData.get())
		,bCastShadow(component->bCastShadow)
	{

	}

	StaticMeshLODResources::~StaticMeshLODResources()
	{
		
	}

	void StaticMeshRenderData::initResource(RStaticMesh* owner)
	{
		for (int32 lodIndex = 0; lodIndex < mLODResources.size(); ++lodIndex)
		{
			mLODResources[lodIndex].initResource(owner);
		}
	}
}