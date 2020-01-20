#pragma once
#include "CoreMinimal.h"
#include "RHICommandList.h"
#include "UtilityShaderConfig.h"
namespace Air
{
	struct ClearQuadCallbacks
	{
		std::function<void(GraphicsPipelineStateInitializer&)> PSOModifier = nullptr;
		std::function<void(RHICommandList&)> PreClear = nullptr;
		std::function<void(RHICommandList&)> PostClear = nullptr;
	};

	extern UTILITY_SHADER_API void drawClearQuadMRT(RHICommandList& RHICmdList, bool bClearColor, int32 numClearColors, const LinearColor* clearColorArray, bool bClearDepth, float depth, bool bClearStencil, uint32 stencil);
	extern UTILITY_SHADER_API void drawClearQuadMRT(RHICommandList& RHICmdList, bool bClearColor, int32 numClearColors, const LinearColor* clearColorArray, bool bClearDepth, float depth, bool bClearStencil, uint32 stencil, ClearQuadCallbacks clearQuadCallbacks);
	extern UTILITY_SHADER_API void drawClearQuadMRT(RHICommandList& RHICmdList, bool bClearColor, int32 numClearColors, const LinearColor* clearColorArray, bool bClearDepth, float depth, bool bClearStencil, uint32 stencil, int2 viewSize, IntRect excludeRect)
		
		
		
		
		
		
		
		
		;
}