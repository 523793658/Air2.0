#include "Classes/GameFramework/Info.h"
#include "SimpleReflection.h"
namespace Air
{
	AInfo::AInfo(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimaryActorTick.bCanEverTick = false;
		bAllowTickBeforeBeginPlay = true;
		bHidden = true;
		
	}

	DECLARE_SIMPLER_REFLECTION(AInfo);
}