#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/World.h"
#include "RHIDefinitions.h"
#include "Math/Math.h"
namespace Air
{
	class RenderTarget;

	class Canvas
	{
	public:

		enum EElementType
		{
			ET_Line,
			ET_Triangle,
			ET_MAX
		};

		enum ECanvasAllowModes
		{
			Allow_Flush		= 1 << 0,
			Allow_DeleteOnRender = 1 << 1
		};

		enum ECanvasDrawMode
		{
			CDM_DeferDrawing,
			CMD_ImmediateDrawing
		};

		ENGINE_API Canvas(RenderTarget* inRenderTarget, World* inWorld, ERHIFeatureLevel::Type inFeatureLevel, ECanvasDrawMode drawMode = CDM_DeferDrawing);

		void setRenderTargetRect(const IntRect& inViewRect)
		{
			mViewRect = inViewRect;
		}

		ENGINE_API void flush_GameThread(bool bForce = false);
	private:
		IntRect mViewRect;
	};
}