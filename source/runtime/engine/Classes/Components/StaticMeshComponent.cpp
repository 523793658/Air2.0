#include "Classes/Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "SimpleReflection.h"
#include "Classes/Engine/World.h"
namespace Air
{
	StaticMeshComponent::StaticMeshComponent(const ObjectInitializer& objectInitializer)
		:MeshComponent(objectInitializer)
	{

	}

	

	bool StaticMeshComponent::setStaticMesh(class RStaticMesh* newMesh)
	{
		if (newMesh == getStaticMesh())
		{
			return false;
		}


		AActor* owner = getOwner();
		if (World* world = getWorld())
		{
			if (world->hasBegunPlay() && !areDynamicDataChangeAllowed() && owner != nullptr)
			{
				return false;
			}
		}

		mStaticMesh = std::dynamic_pointer_cast<RStaticMesh>(newMesh->shared_from_this());

		markRenderStateDirty();
		markCachedMaterialParameterNameIndicesDirty();
		return true;
	}

	DECLARE_SIMPLER_REFLECTION(StaticMeshComponent);
}