#pragma once
#include "CoreType.h"
#include "Misc/CoreMisc.h"
#include "HAL/StringView.h"
#include "Containers/Map.h"
#include <mutex>
#include <map>
namespace Air
{

	enum class EModuleLoadResult
	{
		Success,
FileNotFound,
FileIncompatible,
CouldNotBeLoadedByOS,
FailedToInitialize
	};




	class IModuleInterface;


	typedef std::function<IModuleInterface*(void)> InitializerOfStaticLinkedModule;

	class CORE_API ModuleManager : private SelfRegisteringExec
	{
	public:
		~ModuleManager();

		static ModuleManager& get();

	protected:
		class ModuleInfo
		{
		public:
			wstring mOriginalFilename;
			wstring mFileName;
			void* mHandle;
			std::shared_ptr<IModuleInterface> mModule;
			bool bWasUnloadedAtShutdown;

			int32 mLoadOrder;
			static int32 mCurrentLoadOrder;

		public:
			ModuleInfo()
				:mHandle(nullptr),
				bWasUnloadedAtShutdown(false),
				mLoadOrder(mCurrentLoadOrder++)
			{}
		};

		typedef std::shared_ptr<ModuleInfo> ModuleInfoPtr;

		typedef TMap<wstring, ModuleInfoPtr>  ModuleMap;

	public:
		void abandonModule(const wstring inModuleName);

		void addModule(const wstring inModuleName);

		ModuleInfoPtr findModule(std::wstring_view name);

		void findModules(const TCHAR* wildCardWithoutExtension, TArray<wstring>& outModules) const;

		std::shared_ptr<IModuleInterface> getModule(const wstring name);

		bool isModuleLoaded(const wstring inModuleName);
		std::shared_ptr<IModuleInterface> loadModule(const wstring inModuleName, const bool bWasReleaded = false);

		std::shared_ptr<IModuleInterface> loadModuleChecked(const wstring name, const bool bWasReloaded = false);

		//bool loadModuleWithCallback(const wstring name, ou)

		void addBinariesDirectory(std::wstring_view inDirector, bool bIsGameDirectory);

		template<typename TModuleInterface>
		static TModuleInterface& loadModuleChecked(const wstring ModuleName)
		{
			ModuleManager& moduleManager = ModuleManager::get();
			if (!moduleManager.isModuleLoaded(ModuleName))
			{
				moduleManager.loadModule(ModuleName);
			}
			return getModuleChecked<TModuleInterface>(ModuleName);
		}


		template<typename TModuleInterface>
		static TModuleInterface& getModuleChecked(const wstring ModuleName)
		{
			ModuleManager& manager = ModuleManager::get();
			return static_cast<TModuleInterface&>(*manager.getModule(ModuleName));
		}

		static void getModuleFilenameFormat(bool bGameModule, wstring & outPrefix, wstring & outSuffix);

		std::shared_ptr<IModuleInterface> loadModuleWithFailureReason(const wstring inModuleName, EModuleLoadResult& reason, bool bWasReloaded = false);

		void addModuleToModulesList(const wstring name, ModuleManager::ModuleInfoPtr& inModuleInfo);

		void findModulePaths(std::wstring_view namePattern, TMap<wstring, wstring> & outModulePaths, bool bCanUseCache = true) const;

		void findModulePathsInDirectory(std::wstring_view inDirectoryName, bool bIsGameDirectory, std::wstring_view NamePattern, TMap<wstring, wstring> & outModulePaths) const;


		void registerStaticallyLinkedModule(const wstring inModuleName, const InitializerOfStaticLinkedModule InInitializer)
		{
			mStaticallyLinkedModuleInitializers.emplace(inModuleName, InInitializer);
		}

		template<typename TModuleInterface>
		static TModuleInterface* getModulePtr ( const wstring moduleName)
		{
			ModuleManager& moduleManager = ModuleManager::get();
			if (!moduleManager.isModuleLoaded(moduleName))
			{
				return nullptr;
			}

			return static_cast<TModuleInterface*>(moduleManager.getModule(moduleName).get());
		}

		template<typename TModuleInterface>
		static TModuleInterface* loadModulePtr(const wstring ModuleName)
		{
			ModuleManager& moduleManager = ModuleManager::get();
			if (!moduleManager.isModuleLoaded(ModuleName))
			{
				moduleManager.loadModule(ModuleName);
			}
			return getModulePtr<TModuleInterface>(ModuleName);
		}

	private:
		static bool checkModuleCompatibility(const TCHAR* filename);
		
	public:
		std::mutex mModulesCriticalSection;
	private:
		TArray<wstring> mGameBinariesDirectories;
		TArray<wstring> mEngineBinariesDirectories;

		mutable std::unique_ptr<TMap<wstring, wstring>> mModulePathsCache;
		
		ModuleMap mModules;

		typedef TMap<wstring, InitializerOfStaticLinkedModule> StaticallyLinkedModuleInitializerMap;

		StaticallyLinkedModuleInitializerMap mStaticallyLinkedModuleInitializers;
	};

	template<class ModuleClass>
	class StaticallyLinkedModuleRegistrant
	{
	public:
		StaticallyLinkedModuleRegistrant(wstring name)
		{
			InitializerOfStaticLinkedModule initializer = [this]()->IModuleInterface* { return new ModuleClass(); };

			ModuleManager::get().registerStaticallyLinkedModule(name, initializer);
		}
	};

#if IS_MONOLITHIC
#define IMPLEMENT_MODULE(ModuleImplClass, ModuleName)	\
		static StaticallyLinkedModuleRegistrant<ModuleImplClass> ModuleRegistrant##ModuleName(L#ModuleName); \
		void EmptyLinkFunctionForStaticInitialization##ModuleName(){}
#else
#define IMPLEMENT_MODULE(ModuleImplClass, ModuleName)	\
		extern "C" DLLEXPORT IModuleInterface* initializeModule()	\
		{	\
			return new ModuleImplClass();	\
		}
#endif

}