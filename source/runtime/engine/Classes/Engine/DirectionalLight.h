#pragma once
#include "Classes/Engine/Light.h"
namespace Air
{
	class ENGINE_API ADirectionalLight : public ALight
	{
		GENERATED_RCLASS_BODY(ADirectionalLight, ALight)
	public:
		virtual void postLoad() override;

		
	};
}