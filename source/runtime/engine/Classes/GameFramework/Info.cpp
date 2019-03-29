#include "Classes/GameFramework/Info.h"
#include "SimpleReflection.h"
namespace Air
{
	Info::Info(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimaryActorTick.bCanEverTick = false;
		bAllowTickBeforeBeginPlay = true;
		bHidden = true;
		
	}

	DECLARE_SIMPLER_REFLECTION(Info);
}