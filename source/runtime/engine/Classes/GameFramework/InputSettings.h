#pragma once
#include "EngineMininal.h"
#include "UObject/Object.h"
namespace Air
{
	class ENGINE_API InputSettings : public Object
	{
		GENERATED_RCLASS_BODY(InputSettings, Object)
	public:
	

		float DoubleClickTime{ 0.2f };
	};
}