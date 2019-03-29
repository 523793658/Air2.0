#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include "Serialization/Archive.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include <map>
namespace Air
{
	using std::multimap;

	enum class EConfigCacheType : uint8
	{
		DiskBacked,
		Temporary
	};


	template<typename KeyType, typename ValueType>
	Archive& operator << (Archive& ar, multimap<KeyType, ValueType> mulM)
	{
		if (ar.isLoading())
		{
			size_t size;
			ar << size;
			for (int i = 0; i < size; i++)
			{
				KeyType key;
				ValueType value;
				ar << key;
				ar << value;
				mulM.emplace(key, value);
			}
		}
		else
		{
			ar << mulM.size();
			for (auto& it : mulM)
			{
				ar << const_cast<KeyType&>(it.first);
				ar << it.second;
			}
		}

	}



	struct ConfigValue 
	{
		ConfigValue() {}
		
		ConfigValue(const TCHAR* inValue)
			:mSavedValue(inValue)
#if WITH_EDITOR
			,bRead(false)
#endif
		{
			expandValueInternal();
		}

		ConfigValue(wstring inValue)
			:mSavedValue(std::move(inValue))
#if 0
			,bRead(false)
#endif
		{}
		ConfigValue(const ConfigValue& inConfigValue)
			:mSavedValue(inConfigValue.mSavedValue)
			, mExpandedValue(inConfigValue.mExpandedValue)
#if WITH_EDITOR
			, bRead(inConfigValue.bRead)
#endif
		{

		}

		const wstring& getValue() const
		{
#if WITH_EDITOR
			bRead = true;
#endif
			return (mExpandedValue.length() > 0 ? mExpandedValue : mSavedValue);
		}

		friend Archive& operator << (Archive& ar, ConfigValue& configSection)
		{
			ar << configSection.mSavedValue;
			if (ar.isLoading())
			{
				configSection.expandValueInternal();
			}
			return ar;
		}

		bool operator == (const ConfigValue& other) const { return (mSavedValue.compare(other.mSavedValue) == 0); }

		bool operator != (const ConfigValue & other) const { return (mSavedValue.compare(other.mSavedValue) != 0); }

	private:
		void expandValueInternal()
		{

		}

		wstring mSavedValue;
		wstring mExpandedValue;
#if WITH_EDITOR
		mutable bool bRead;
#endif
	};


	typedef multimap<wstring, ConfigValue> ConfigSectionMap;

	class ConfigSection : public ConfigSectionMap
	{
	public:
	};

	class ConfigFile : public TMap<wstring, ConfigSection>
	{
	public:
		void read(const wstring& filename);

		void processInputFileContents(const wstring& contents);

		ConfigSection* findOrAddSection(const wstring& sectionName);
	};

	class CORE_API ConfigCacheIni : public TMap<wstring, ConfigFile>
	{
	public:
		bool getSection(const  TCHAR* section, TArray<wstring>& result, const wstring & fileName);
		ConfigFile* find(const wstring& filename, bool createIfNotFound);

		bool areFileOperationsDisabled();

	private:
		bool bAreFileOperationsDisabled;

		bool bIsReadyForUse;
		EConfigCacheType mType;
	};
}