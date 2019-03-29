#pragma once
#include "EngineMininal.h"
#include "GenericPlatform/GenericWindows.h"
#include "RenderCommandFence.h"
#include "Classes/Engine/Engine.h"
namespace Air
{

	class LocalPlayerIterator
	{
	protected:
		TArray<class LocalPlayer*>::TConstIterator iter;
		void getCurrent()
		{
			while (iter && *iter == nullptr)
			{
				++iter;
			}
		}
	public:
		LocalPlayerIterator(Engine* inEngine, class World* inWorld)
			:iter(inEngine->getLocalPlayerIterator(inWorld))
		{
			getCurrent();
		}

		void operator ++()
		{
			++iter;
			getCurrent();
		}

		LocalPlayer* operator*()
		{
			return *iter;
		}

		LocalPlayer* operator->()
		{
			return *iter;
		}
		operator bool()
		{
			return (bool)iter;
		}
	};


	ENGINE_API void initializeRenderingCVarsCaching();

	class FrameEndSync
	{
		RenderCommandFence mFence[2];
		int32 mEventIndex;
	public:
		ENGINE_API void sync(bool bAllowOneFrameThreadLag);
	};


	struct ENGINE_API SystemResolution
	{
		int32 mResX{ 0 };
		int32 mResY{ 0 };
		EWindowMode::Type mWindowType{ EWindowMode::Windowed };
		bool mForceRefresh{ false };

		static void requestResolutionChange(int32 InResX, int32 InResY, EWindowMode::Type inWindowMode);

		void forceRefresh()
		{
			requestResolutionChange(mResX, mResY, mWindowType);
			mForceRefresh = true;
		}
	};

	struct CachedSystemScalabilityCVars 
	{
		bool bInitialized;
		int32 mDetailMode;
		EMaterialQualityLevel::Type mMaterialQualityLevel;
		int32 mMaxShadowResolution;

		CachedSystemScalabilityCVars();
	protected:
	};

	ENGINE_API extern SystemResolution GSystemResolution;

	ENGINE_API const CachedSystemScalabilityCVars& getCachedScalabilityCVars();

	class IEngineLoop
	{
	public:
		virtual int32 init() = 0;

		virtual void tick() = 0;

		virtual void clearPendingCleanupObjects() = 0;
	};

	class ENGINE_API ScopedConditionalWorldSwitcher
	{
	public:
		ScopedConditionalWorldSwitcher(class ViewportClient* inViewportClient);
		~ScopedConditionalWorldSwitcher();

	private:
		ViewportClient * mViewportClient;
		World* mWorld;
	};
}