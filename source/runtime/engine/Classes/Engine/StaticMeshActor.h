#pragma once
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/StaticMeshComponent.h"
namespace Air
{
	class ENGINE_API AStaticMeshActor : public AActor
	{
		GENERATED_RCLASS_BODY(AStaticMeshActor, AActor)
	public:
		

		class StaticMeshComponent* getStaticMeshComponent()const;

	public:

		std::shared_ptr<class StaticMeshComponent> mStaticMeshComponennt;
	};
}