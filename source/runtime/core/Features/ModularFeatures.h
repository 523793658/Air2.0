#pragma once
#include "Features/IModularFeatures.h"
#include <map>
namespace Air
{
	class ModularFeatures : public IModularFeatures
	{
	public:
		virtual int32 getModularFeatureImplementationCount(const wstring type) override;

		virtual IModularFeature* getModularFeatureImplementation(const wstring Type, const int32 Index) override;

	private:
		std::multimap<wstring, class IModularFeature*> mModularFeaturesMap;
	};
}