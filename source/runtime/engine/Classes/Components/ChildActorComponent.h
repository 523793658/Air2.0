#pragma once
#include "Classes/Components/SceneComponent.h"
namespace Air
{
	class ENGINE_API ChildActorComponent : public SceneComponent
	{
		GENERATED_RCLASS_BODY(ChildActorComponent, SceneComponent)
	public:
	

		AActor* getChildActor()const { return mChildActor; }
	private:
		AActor* mChildActor{ nullptr };

	protected:
		uint32 bAllowReregistration : 1;
	};
}