#pragma once
#include "CoreMinimal.h"
#include "PostProcess/RenderingCompositionGraph.h"
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

	class PostProcessVS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(PostProcessVS, Global);

		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		PostProcessVS() {}

		void setParameters(RenderingCompositePassContext& context)
		{
			GlobalShader::setParameters(context.mRHICmdList, getVertexShader(), context.mView);
		}

		void setParameters(RHICommandList& RHICmdList, const SceneView& view)
		{
			GlobalShader::setParameters(RHICmdList, getVertexShader(), view);
		}
	public:
		PostProcessVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
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