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

	struct IniFilename
	{
		wstring mFilename;
		bool bRequired;
		wstring mCacheKey;

		IniFilename(const wstring& inFilename, bool inIsRequired, wstring inCacheKey = TEXT(""))
			:mFilename(inFilename)
			, bRequired(inIsRequired)
			, mCacheKey(inCacheKey)
		{

		}


	};

	enum class EConfigFileHierarchy : uint8
	{
		AbsoluteBase = 0,
		EngineDirBase,
	};

	typedef TMap<EConfigFileHierarchy, IniFilename> ConfigFileHierarchy;


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

		const wstring& getSavedValue() const
		{
#if WITH_EDITOR
			bRead = true;
#endif
			return mSavedValue;
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
		template<typename Allocator>
		void multiFind(const wstring& key, TArray<ConfigValue, Allocator>& outValues, const bool bMaintainOrder = false) const
		{
			auto pair = ConfigSectionMap::equal_range(key);
			for (auto& bg = pair.first; bg != pair.second; ++bg)
			{
				outValues.add(bg->second);
			}
		}

		int32 removeSingle(wstring key, ConfigValue inValue)
		{
			int32 numRemovedPairs = 0;
			for (auto it = this->begin(); it != this->end(); it++)
			{
				if (it->first == key && it->second == inValue)
				{
					erase(it);
					++numRemovedPairs;
					break;
				}
			}
			return numRemovedPairs;
		}

		ConfigValue& addUnique(wstring key, ConfigValue& value)
		{
			auto it = this->equal_range(key);
			for (auto & paireIt = it.first; paireIt != it.second; paireIt++)
			{
				if (paireIt->second == value)
				{
					return paireIt->second;
				}
			}
			return this->emplace(key, value)->second;
		}

		void handleAddCommand(wstring key, const wstring & value, bool bAppendValueIfNotArrayOfStructsKeyUsed);

		TMap<wstring, wstring> mArrayOfStructKeys;
	};

	class ConfigFile : public TMap<wstring, ConfigSection>
	{
	public:
		void read(const wstring& filename);

		void processInputFileContents(const wstring& contents);

		ConfigSection* findOrAddSection(const wstring& sectionName);


		bool combine(const wstring& filename);

		void combineFromBuffer(const wstring& buffer);

		void addMissingProperties(const ConfigFile& inSourceFile);

		CORE_API bool getString(const TCHAR* section, const TCHAR* key, wstring& value) const;


	private:
		TMap<wstring, TMap<wstring, wstring>> mPerObjectConfigArrayOfStructKeys;

	public:
		bool mDirty, NoSave;
		ConfigFileHierarchy mSourceIniHerarchy;

		ConfigFile* mSourceConfigFile;

		wstring mName;
	};

	class CORE_API ConfigCacheIni : public TMap<wstring, ConfigFile>
	{
	public:
		bool getSection(const  TCHAR* section, TArray<wstring>& result, const wstring & fileName);
		ConfigFile* find(const wstring& filename, bool createIfNotFound);

		int32 getArray(const TCHAR* section, const TCHAR* key, TArray<wstring>& outArr, const wstring& filename);

		bool areFileOperationsDisabled();

		static void loadLocalIniFile(ConfigFile& configFile, const TCHAR* iniName, bool bGenerateDestIni, const TCHAR* platform, const bool bForceReload = false);

		static void loadExternalIniFile(ConfigFile& configFile, const TCHAR* iniName, const TCHAR* engineConfigDir, const TCHAR* sourceConfigDir, bool bGenerateDestIni, const TCHAR* platform = nullptr, const bool bForceReload = false);
	private:
		bool bAreFileOperationsDisabled;

		bool bIsReadyForUse;
		EConfigCacheType mType;
	};

	
	
	
}