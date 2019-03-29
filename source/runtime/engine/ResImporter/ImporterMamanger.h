#pragma once
#include "EngineMininal.h"
#include "Misc/Paths.h"

namespace Air
{
	struct EngineResourceData
	{
		bool bLoaded{ false };

		virtual ~EngineResourceData() {}
	};

	class Importer
	{
	public:
		Importer(TCHAR* extension);
		virtual ~Importer() {}

		virtual bool decode(filesystem::path path, std::shared_ptr<EngineResourceData>& outResourceData) = 0;

	private:
		wstring mExtension;
	};

	class ENGINE_API ImporterMamanger
	{
	public:
		static ImporterMamanger* get();

		bool registerImporter(const TCHAR* extension, Importer* importer);

		Importer* getImporter(const TCHAR* extension);

	private:
		ImporterMamanger();

		TMap<wstring, Importer*> mImporters;

		static ImporterMamanger* mInstance;
	};
}