#pragma once

#include "SlateRHIRendererConfig.h"
#include "Rendering/RenderingPolicy.h"
namespace Air
{

	class SlateRHIRenderingPolicy : public SlateRederingPolicy
	{
	public:
		void beginDrawingWindows();
		void endDrawingWindows();

	};

}