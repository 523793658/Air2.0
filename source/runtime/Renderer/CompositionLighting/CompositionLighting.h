#pragma once
#include "CoreMinimal.h"
#include "sceneRendering.h"
namespace Air
{
	class CompositionLighting
	{
	public:
		void processBeforeBasePass(RHICommandListImmediate& RHICmdList, ViewInfo& view);

		void processAfterBasePass(RHICommandListImmediate& RHICmdList, ViewInfo& view);
	};

	extern CompositionLighting GCompositionLighting;
}