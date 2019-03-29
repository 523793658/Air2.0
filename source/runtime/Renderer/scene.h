#pragma once
#include "RendererMininal.h"
#include "SceneInterface.h"
#include "RenderResource.h"
namespace Air
{
	class LightSceneInfo;

	enum EAntiAliasingMethod
	{
		AAM_None,
		AAM_FXAA,
		AAM_TemporalAA,
		AAM_MSAA,
		AAM_MAX
	};

	struct PostProcessSettings 
	{
	};

	

	class AScene
	{
#define SDPG_NumBits	3
	};

	class IndirectLightingCache : public RenderResource
	{

	};
}