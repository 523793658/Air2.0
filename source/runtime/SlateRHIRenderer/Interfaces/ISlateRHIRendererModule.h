#pragma once
#include "SlateRHIRendererConfig.h"
#include "Modules/ModuleInterface.h"
#include "ISlate3DRenderer.h"
namespace Air
{

	namespace SlateRHIConstants
	{
		const int32 numBuffers = 3;
	}

	class SlateRenderer;

	class ISlateRHIRendererModule
		: public IModuleInterface
	{
	public:
		virtual std::shared_ptr<SlateRenderer> createSlateRHIRenderer() = 0;
		virtual std::shared_ptr<ISlate3DRenderer> createSlate3DRenderer(bool useGammaCorrect) = 0;


	};
}