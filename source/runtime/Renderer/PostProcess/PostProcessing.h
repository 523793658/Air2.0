#pragma once
#include "CoreMinimal.h"
#include "PostProcess/RenderingCompositionGraph.h"
#include "ScreenPass.h"
namespace Air
{
	class PostProcessContext
	{
	public:
		PostProcessContext(RHICommandListImmediate& inRHICmdList, RenderingCompositionGraph& inGraph, const ViewInfo& inView);

		RHICommandListImmediate& mRHICmdList;
		RenderingCompositionGraph& mGraph;
		const ViewInfo& mView;
		RenderingCompositePass* mSceneColor;
		RenderingCompositePass* mSceneDepth;
		RenderingCompositeOutputRef mFinalOutput;
	};

	class PostProcessVS : public ScreenPassVS
	{
	public:

		PostProcessVS() = default;

		PostProcessVS(const ShaderMetaType::CompiledShaderInitializerType& initialier)
			:ScreenPassVS(initialier)
		{

		}


		void setParameters(RenderingCompositePassContext& context)
		{
			
		}

		void setParameters(RHICommandList& RHICmdList, const SceneView& view)
		{
		
		}
	};

	class PostProcessing
	{
	public:
		bool allowFullPostProcessing(const ViewInfo& view, ERHIFeatureLevel::Type featureLevel);

		void process(RHICommandListImmediate& RHICmdList, const ViewInfo& view, TRefCountPtr<IPooledRenderTarget>& velocityRT);

		void processES2(RHICommandListImmediate& RHICmdList, const ViewInfo& view, bool bViewRectSource);
	
		void processPlanarReflection(RHICommandListImmediate& RHICmdList, ViewInfo& view, TRefCountPtr<IPooledRenderTarget>& velocityRT, TRefCountPtr<IPooledRenderTarget>& outFilterredSceneColor);
	};

	extern PostProcessing GPostProcessing;
}