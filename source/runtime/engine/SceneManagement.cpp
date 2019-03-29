#include "SceneManagement.h"
#include "SceneView.h"
#include "Texture2D.h"
#include "RHIStaticStates.h"
namespace Air
{
	ENGINE_API bool GDrawListsLocked = false;

	ViewConstantShaderParameters::ViewConstantShaderParameters()
	{
		Memory::memzero(*this);
		SkyBoxTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::getRHI();
	}

	SharedSamplerState* Wrap_WorldGroupSettings = nullptr;
	SharedSamplerState* Clamp_WorldGroupSettings = nullptr;

	void initializeSharedSamplerStates()
	{
		Wrap_WorldGroupSettings = new SharedSamplerState(true);
		Clamp_WorldGroupSettings = new SharedSamplerState(false);
		beginInitResource(Wrap_WorldGroupSettings);
		beginInitResource(Clamp_WorldGroupSettings);
	}

	void SharedSamplerState::initRHI()
	{
		const float mipmapBias = RTexture2D::getGlobalMipMapLODBias();
		SamplerStateInitializerRHI samplerStateInitializer(
			ESamplerFilter::SF_Bilinear, bWrap ? AM_Wrap : AM_Clamp,
			bWrap ? AM_Wrap : AM_Clamp, bWrap ? AM_Wrap : AM_Clamp, mipmapBias
		);
		mSamplerStateRHI = RHICreateSamplerState(samplerStateInitializer);
	}

}