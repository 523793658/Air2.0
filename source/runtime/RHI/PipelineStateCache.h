#pragma once
#include "CoreMinimal.h"
#include "RHI.h"
#include "Misc/EnumClassFlags.h"
#include "Async/TaskGraphInterfaces.h"
namespace Air
{
	class GraphicsPipelineState;
	class RHIGraphicsPipelineState;
	class RHICommandList;
	class GraphicsPipelineStateInitializer;

	extern RHI_API RHIGraphicsPipelineState* executeSetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState);

	enum class EApplyRendertargetOption : int
	{
		DoNothing = 0,
		ForceApply = 1 << 0,
		CheckApply = 1 << 1,
	};
	ENUM_CLASS_FLAGS(EApplyRendertargetOption);


	extern RHI_API void setGraphicsPipelineState(RHICommandList& RHICmdList, const GraphicsPipelineStateInitializer& initializer, EApplyRendertargetOption applyFlags = EApplyRendertargetOption::CheckApply);

	namespace PipelineStateCache
	{
		extern RHI_API GraphicsPipelineState* getAndOrCreateGraphicsPipelineState(RHICommandList& RHICmdList, const GraphicsPipelineStateInitializer& originalInitializer, EApplyRendertargetOption applyFlags);


	}


	struct RHI_API PipelineStateStats
	{
		int64 mFirstFrameUsed{ -1 };
		int64 mLastFrameUsed{ -1 };
		uint64 mCreateCount{ 0 };
		int64 mTotalBindCount{ 0 };
		uint32 mPSOHash{ 0 };

		~PipelineStateStats()
		{}

		static void updateStats(PipelineStateStats* stats);

		friend RHI_API Archive& operator <<(Archive& ar, PipelineStateStats& info);
	};

	
	


}