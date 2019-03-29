#pragma once
#include "EngineMininal.h"
#include "Classes/Components/ActorComponent.h"
namespace Air
{
	class ComponentRecreateRenderStateContext
	{
	private:
		ActorComponent* mComponent;
	public:
		ComponentRecreateRenderStateContext(ActorComponent* inComponent)
		{
			BOOST_ASSERT(inComponent);
			if (inComponent->isRegistered() && inComponent->isRenderStateCreated())
			{
				inComponent->destroyRenderState_Concurrent();
				mComponent = inComponent;
			}
			else
			{
				mComponent = nullptr;
			}
		}
		~ComponentRecreateRenderStateContext()
		{
			if (mComponent && !mComponent->isRenderStateCreated() && mComponent->isRegistered())
			{
				mComponent->recreateRenderState_Concurrent();
			}
		}
	};
}