#pragma once
#include "CoreMinimal.h"
#include "SceneView.h"
#include "RHICommandList.h"
#include "Classes/Camera/CameraTypes.h"
namespace Air
{
	class APlayerController;

	class ISceneViewExtension
	{
	public:
		virtual ~ISceneViewExtension() {}

		virtual void beginRenderViewFamily(SceneViewFamily& inViewFamily) = 0;
		virtual void preRenderViewFamily_RenderThread(RHICommandListImmediate& RHICmdList, SceneViewFamily& inViewFamily) = 0;

		virtual void preRenderView_RenderThread(RHICommandListImmediate& RHICmdList, SceneView& inView) = 0;

		virtual void setupViewFamily(SceneViewFamily& inSceneViewFamily) {}

		virtual void setViewPoint(std::shared_ptr<APlayerController> player, MinimalViewInfo& inViewInfo) {}

		virtual void setupView(SceneViewFamily& inViewFamily, SceneView& inView) {};

		virtual void postInitViewFamily_RenderThread(RHICommandListImmediate& RHICmdList, SceneViewFamily& inViewFamily) {};

		virtual void postInitView_RenderThread(RHICommandListImmediate& RHICmdList, SceneView& inView) {}
	};
}