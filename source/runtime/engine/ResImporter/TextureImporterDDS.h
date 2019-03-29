#pragma once
#include "ResImporter/TextureImporter.h"
namespace Air
{
	class TextureImporterDDS : public TextureImporter
	{
	public:
		TextureImporterDDS();

		virtual bool decode(filesystem::path path, std::shared_ptr<EngineResourceData>& outResourceData) override;

		virtual TextureInfo getTextureInfo(Archive& file) override;
	private:
		static TextureImporterDDS* mInstance;
	};
}