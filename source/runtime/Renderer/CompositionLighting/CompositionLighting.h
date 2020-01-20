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

		bool canProcessAsyncSSAO(TArray<ViewInfo>& views);

		void processAsyncSSAO(RHICommandListImmediate& RHICmdList, TArray<ViewInfo>& views);

		void gfxWaitForAsyncSSAO(RHICommandListImmediate& RHICmdList);

	private:
		ComputeFenceRHIRef mAsyncSSAOFence;
	};

	extern CompositionLighting GCompositionLighting;
}