#pragma once
#include "EngineMininal.h"
#include "UObject/Object.h"
namespace Air
{
	class Brush;

	class Model : public Object
	{
		GENERATED_RCLASS_BODY(Model, Object)
	public:
		

		ENGINE_API void initialize(Brush* owner, bool inRootOutside = true);
	};
}