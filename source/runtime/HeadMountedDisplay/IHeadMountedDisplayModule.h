#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Features/IModularFeature.h"
#include "Features/IModularFeatures.h"
namespace Air
{
#ifdef HeadMountedDisplay_EXPORTS
#define HEADMOUNTEDDISPLAY_API DLLEXPORT
#else
#define HEADMOUNTEDDISPLAY_API DLLIMPORT
#endif

	class IHeadMountedDisplayModule : public IModuleInterface, public IModularFeature
	{
	public:
		static wstring getModularFeatureName()
		{
			static wstring HMDFeatureName = TEXT("HMD");
			return HMDFeatureName;
		}

		static inline bool isAvailable()
		{
			return IModularFeatures::get().isModularFeatureAvailable(getModularFeatureName());
		}

		virtual wstring getModuleKeyName() const = 0;

		float getModulePriority()const
		{
			float modulePriority = 0.0f;
			wstring keyName = getModuleKeyName();
			return modulePriority;
		}


		struct CompareModulePriority 
		{
			bool operator()(IHeadMountedDisplayModule& A, IHeadMountedDisplayModule& b) const
			{
				return A.getModulePriority() > b.getModulePriority();
			}
		};

		static inline IHeadMountedDisplayModule& get()
		{
			TArray<IHeadMountedDisplayModule*> HMDModules = IModularFeatures::get().getModularFeatureImplementations<IHeadMountedDisplayModule>(getModularFeatureName());
			HMDModules.sort(CompareModulePriority());
			return *HMDModules[0];
		}

		virtual int32 getGraphicsAdapter() { return -1; }
	};

	
}