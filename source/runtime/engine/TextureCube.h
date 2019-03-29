#pragma once
#include "Texture.h"
#include "ResImporter/TextureImporter.h"
namespace Air
{
	class RTextureCube : public RTexture
	{
		GENERATED_RCLASS_BODY(RTextureCube, RTexture);
		
	public:

#include "TextureInterface.inl"

		TextureResource * createResource() override;
	};
}