#pragma once
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/PrimitiveComponent.h"
namespace Air
{
	class SimpleShapeActor : public AActor
	{
		GENERATED_RCLASS_BODY(SimpleShapeActor, AActor)
	public:
		

	protected:
		std::shared_ptr<class PrimitiveComponent>	mPrimitiveComponent;
	};

	class ENGINE_API SphereActor : public SimpleShapeActor
	{
		GENERATED_RCLASS_BODY(SphereActor, SimpleShapeActor)
	public:
		
	};

	class ENGINE_API CubeActor : public SimpleShapeActor
	{
		GENERATED_RCLASS_BODY(CubeActor, SimpleShapeActor)
	public:
		
	};
}