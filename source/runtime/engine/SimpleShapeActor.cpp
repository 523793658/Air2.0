#include "SimpleShapeActor.h"
#include "Classes/Components/SphereComponent.h"
#include "Classes/Components/CubeComponent.h"
#include "SimpleReflection.h"
namespace Air
{

	SimpleShapeActor::SimpleShapeActor(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	SphereActor::SphereActor(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimitiveComponent = createDefaultSubObject<SphereComponent>(TEXT("Sphere"));
		mRootComponent = mPrimitiveComponent;
	}

	CubeActor::CubeActor(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimitiveComponent = createDefaultSubObject<CubeComponent>(TEXT("Cube"));
		mRootComponent = mPrimitiveComponent;
	}

	DECLARE_SIMPLER_REFLECTION(SimpleShapeActor);
	
	DECLARE_SIMPLER_REFLECTION(SphereActor);

	DECLARE_SIMPLER_REFLECTION(CubeActor);
}