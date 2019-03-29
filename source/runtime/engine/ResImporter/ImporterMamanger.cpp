#include "ResImporter/ImporterMamanger.h"
namespace Air
{
	ImporterMamanger* ImporterMamanger::mInstance = nullptr;

	ImporterMamanger::ImporterMamanger()
	{

	}

	Importer::Importer(TCHAR* extension)
		:mExtension(extension)
	{
		ImporterMamanger::get()->registerImporter(extension, this);
	}

	ImporterMamanger* ImporterMamanger::get()
	{
		if (mInstance == nullptr)
		{
			mInstance = new ImporterMamanger();
		}
		return mInstance;
	}

	bool ImporterMamanger::registerImporter(const TCHAR* patten, Importer* importer)
	{
		mImporters.emplace(patten, importer);
		return true;
	}

	Importer* ImporterMamanger::getImporter(const TCHAR* extension)
	{
		auto it = mImporters.find(extension);
		if (it == mImporters.end())
		{
			return nullptr;
		}
		return it->second;
	}
}