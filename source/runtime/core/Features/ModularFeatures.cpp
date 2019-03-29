#include "Features/ModularFeatures.h"
namespace Air
{
	IModularFeatures& IModularFeatures::get()
	{
		static ModularFeatures mModularFeatures;
		return mModularFeatures;
	}


	int32 ModularFeatures::getModularFeatureImplementationCount(const wstring type)
	{
		return mModularFeaturesMap.count(type);
	}

	IModularFeature* ModularFeatures::getModularFeatureImplementation(const wstring Type, const int32 Index)
	{
		IModularFeature* modularFeature = nullptr;
		int32 currentIndex = 0;
		auto bound = mModularFeaturesMap.equal_range(Type);
		for (auto it = bound.first; it != bound.second; it++)
		{
			if (Index == currentIndex)
			{
				modularFeature = it->second;
				break;
			}
			++currentIndex;
		}
		return modularFeature;
	}
}