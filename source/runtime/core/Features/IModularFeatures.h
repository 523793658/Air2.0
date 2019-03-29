#pragma once
#include "CoreType.h"
#include <map>
#include "Containers/Array.h"
#include "Containers/String.h"
namespace Air
{
	class IModularFeatures
	{
	public:
		static CORE_API IModularFeatures& get();

		virtual ~IModularFeatures()
		{

		}

		

		template<typename TModularFeature>
		inline TArray<TModularFeature*> getModularFeatureImplementations(const wstring type)
		{
			TArray<TModularFeature*> featureImplementations;
			const int32 numImplementations = getModularFeatureImplementationCount(type);
			for (int32 curImplementation = 0; curImplementation < numImplementations; ++curImplementation)
			{
				featureImplementations.push_back(static_cast<TModularFeature*>(getModularFeatureImplementation(type, curImplementation)));
			}
			return featureImplementations;
		}
	

		inline bool isModularFeatureAvailable(const wstring type)
		{
			return getModularFeatureImplementationCount(type) > 0;
		}

	public:
		virtual class IModularFeature* getModularFeatureImplementation(const wstring type, const int32 index) = 0;

		virtual int32 getModularFeatureImplementationCount(const wstring type) = 0;
	private:
		
	};
}