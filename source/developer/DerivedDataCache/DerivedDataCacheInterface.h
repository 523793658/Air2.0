#pragma once
#include "CoreMinimal.h"
#include "Misc/Char.h"
#include "Modules/ModuleInterface.h"
#include "boost/algorithm/algorithm.hpp"
namespace Air
{
	class DerivedDataCacheInterface
	{
	public:
		virtual ~DerivedDataCacheInterface() {}

		virtual void put(const TCHAR* cacheKey, TArray<uint8>& data, bool bPutEvenIfExists = false) = 0;

		virtual bool getSynchronous(class DerivedDataCacheInterface* dataDeriver, TArray<uint8>& outData, bool* bDataWasBuilt = nullptr) = 0;

		static wstring sanitizeCacheKey(const TCHAR* cacheKey)
		{
			wstring output;
			wstring input(cacheKey);
			int32 startValid = 0;
			int32 numValid = 0;
			for (int32 i = 0; i < input.size(); i++)
			{
				if (Char::isAlnum(input[i]) || Char::isUnderScore(input[i]))
				{
					numValid++;
				}
				else
				{
					output += input.substr(startValid, numValid);
					startValid = i + 1;
					numValid = 0;
					output += printf(TEXT("$%x"), uint32(input[i]));
				}
			}
			if (startValid == 0 && numValid == input.size())
			{
				return input;
			}
			else if (numValid > 0)
			{
				output += input.substr(startValid, numValid);
			}
			return output;
		}

		static wstring buildCacheKey(const TCHAR* pluginName, const TCHAR* versionString, const TCHAR* pluginSpecificCacheKeySuffix)
		{
			return sanitizeCacheKey(String::printf(TEXT("%s_%s_%s"), pluginName, versionString, pluginSpecificCacheKeySuffix).c_str());
		}
	};

	class IDerivedDataCacheModule : public IModuleInterface
	{
	public:
		virtual DerivedDataCacheInterface& getDDC() = 0;
	};
}