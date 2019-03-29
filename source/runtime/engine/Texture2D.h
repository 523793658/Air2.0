#pragma once
#include "Texture.h"
#include "ResImporter/TextureImporter.h"
namespace Air
{
	class RTexture2D : public RTexture
	{
		GENERATED_RCLASS_BODY(RTexture2D, RTexture)

	public:
		ENGINE_API static float getGlobalMipMapLODBias();

		virtual TextureResource* createResource() override;

#include "TextureInterface.inl"

		void getMipData(int32 firstMipToLoad, void** outMipData);

		static void calcAllowedMips(int32 mipCount, int32 numNonStreamingMips, int32 LODBias, int32& outMinAllowedMips, int32& outMaxAllowedMips);

	public:
		Texture2DResourceMem * mResourceMem;
		
		int32 mRequestedMips;
		int32 mResidentMips;
		uint32 bIsStreamable : 1;
	};
}