#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Engine/StaticMeshActor.h"
#include "SimpleReflection.h"
namespace Air
{
	AStaticMeshActor::AStaticMeshActor(const ObjectInitializer & objectInitializer):AActor(objectInitializer)
	{
		mStaticMeshComponennt = createDefaultSubObject<StaticMeshComponent>(wstring(TEXT("staticMeshComponent")));
		mRootComponent = mStaticMeshComponennt;
	}

	StaticMeshComponent* AStaticMeshActor::getStaticMeshComponent()const
	{
		return mStaticMeshComponennt.get();
	}

	DECLARE_SIMPLER_REFLECTION(AStaticMeshActor);
}