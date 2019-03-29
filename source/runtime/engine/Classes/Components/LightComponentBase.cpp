#include "Classes/Components/LightComponentBase.h"

#include "SimpleReflection.h"

namespace Air
{
	LightComponentBase::LightComponentBase(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
		,bAffectsWorld(true)
		,mIntensity(PI)
	{

	}

	bool LightComponentBase::hasStaticLighting() const
	{
		AActor* owner = getOwner();
		return owner && (mMobility == EComponentMobility::Static);
	}

	bool LightComponentBase::hasStaticShadow() const
	{
		AActor* owner = getOwner();
		return owner && (mMobility != EComponentMobility::Movable);
	}

	DECLARE_SIMPLER_REFLECTION(LightComponentBase);
}