#pragma once
#include "Texture.h"
#include "ResImporter/TextureImporter.h"
namespace Air
{
	class RTextureCube : public RTexture
	{
		GENERATED_RCLASS_BODY(RTextureCube, RTexture);
		

	public:

		virtual EMaterialValueType getMaterialType() override { return MCT_TextureCube; }


#include "TextureInterface.inl"

		TextureResource * createResource() override;
	};
}