#include "AirEngine.h"
#include "Classes/Engine/Engine.h"
#include "Classes/Engine/WorldContext.h"
#include "Classes/Engine/GameInstance.h"
#include "Modules/ModuleManager.h"
#include "ResLoader/ResLoader.h"
#include "EngineModule.h"

namespace Air
{
	ENGINE_API Engine* GEngine = nullptr;

	SystemResolution GSystemResolution;

	static bool GHDROutputEnabled = false;

	static TArray<LocalPlayer*> FakeEmptyLocalPlayers;

	const TArray<class LocalPlayer*>& handleFakeLocalPlayerList()
	{
		return FakeEmptyLocalPlayers;
	}

	static CachedSystemScalabilityCVars GCachedScalabilityCVars;

	LocalPlayer* getLocalPlayerFromControllerId_Local(const TArray<class LocalPlayer*>& gamePlayers, const int32 controllerId)
	{
		for (LocalPlayer* const player : gamePlayers)
		{
			if (player && player->getControllerId() == controllerId)
			{
				return player;
			}
		}
		return nullptr;
	}

	CachedSystemScalabilityCVars::CachedSystemScalabilityCVars()
		:bInitialized(true),
		mMaterialQualityLevel(EMaterialQualityLevel::High)
	{
	}

	const CachedSystemScalabilityCVars& getCachedScalabilityCVars()
	{
		BOOST_ASSERT(GCachedScalabilityCVars.bInitialized);
		return GCachedScalabilityCVars;
	}

	WorldContext& handleInvalidWorldContext()
	{
		return GEngine->createNewWorldContext(EWorldType::None);
	}

	void SystemResolution::requestResolutionChange(int32 InResX, int32 InResY, EWindowMode::Type inWindowMode)
	{

	}

	void systemResolutionSinkCallback()
	{
		GSystemResolution.mResX = 1024;
		GSystemResolution.mResY = 768;
		GSystemResolution.mWindowType = EWindowMode::Windowed;
		GSystemResolution.mForceRefresh = false;
	}

	ENGINE_API void initializeRenderingCVarsCaching()
	{
		systemResolutionSinkCallback();
	}


	void FrameEndSync::sync(bool bAllowOneFrameThreadLag)
	{
		mFence[mEventIndex].beginFence();
		bool bEmptyGameThreadTask = !TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::GameThread);

		if (bEmptyGameThreadTask)
		{
			TaskGraphInterface::get().processThreadUntilIdle(ENamedThreads::GameThread);
		}
		if (bAllowOneFrameThreadLag)
		{
			mEventIndex = (mEventIndex + 1) % 2;
		}
		mFence[mEventIndex].wait(bEmptyGameThreadTask);
	}

	void Engine::initializeRunningAverageDeltaTime()
	{
		mRunningAverageDeltaTime = 1 / 100.0f;
	}

	void Engine::init(class EngineLoop* inEngineLoop)
	{
		mEngineLoop = inEngineLoop;

		initializeRunningAverageDeltaTime();

		if (GIsEditor)
		{
			EWorldType::Type type = GIsEditor ? EWorldType::Editor : EWorldType::Demo;
			WorldContext& initialWorldContext = createNewWorldContext(type);
			initialWorldContext.setCurrentWorld(World::createWorld(type, true));
			GWorld = initialWorldContext.getWorld();
		}
		bSmoothFrameRate = false;
		bUseFixedFrameRate = false;
		bForceDisableFrameRateSmoothing = false;
		mSmoothedFrameRateRange = FloatRange(10, 120);
	}

	LocalPlayer* Engine::getLocalPlayerFromControllerId(const ViewportClient* inViewport, const int32 controllerId) const
	{
		if (getWorldContextFromViewport(inViewport) != nullptr)
		{
			const TArray<class LocalPlayer*>& gamePlayers = getGamePlayers(inViewport);
			return getLocalPlayerFromControllerId_Local(gamePlayers, controllerId);
		}
		return nullptr;
	}

	const TArray<class LocalPlayer*>& Engine::getGamePlayers(const ViewportClient* viewport) const
	{
		const WorldContext& context = getWorldContextFromViewportChecked(viewport);
		if (context.mOwningGameInstance == nullptr)
		{
			return handleFakeLocalPlayerList();
		}
		return context.mOwningGameInstance->getLocalPlayers();
	}

	void Engine::start()
	{

	}

	void Engine::cleanupGameViewport()
	{

	}

	const TArray<class LocalPlayer*>& Engine::getGamePlayers(World* inWorld) const
	{
		const WorldContext& context = getWorldContextFromWorldChecked(inWorld);
		if (context.mOwningGameInstance == nullptr)
		{
			return handleFakeLocalPlayerList();
		}
		return context.mOwningGameInstance->getLocalPlayers();
	}

	const WorldContext* Engine::getWorldContextFromViewport(const ViewportClient* inViewport) const
	{
		for (const WorldContext& worldContext : mWorldList)
		{
			if (worldContext.mGameViewport == inViewport)
			{
				return &worldContext;
			}
		}
		return nullptr;
	}

	WorldContext* Engine::getWorldContextFromViewport(const ViewportClient* inViewport)
	{
		for (WorldContext& worldContext : mWorldList)
		{
			if (worldContext.mGameViewport == inViewport)
			{
				return &worldContext;
			}
		}
		return nullptr;
	}


	WorldContext& Engine::getWorldContextFromViewportChecked(const ViewportClient* inViewportClient)
	{
		if (WorldContext* worldContext = getWorldContextFromViewport(inViewportClient))
		{
			return *worldContext;
		}
		return handleInvalidWorldContext();
	}

	const WorldContext& Engine::getWorldContextFromViewportChecked(const ViewportClient* inViewportClient) const
	{
		if (const WorldContext* worldContext = getWorldContextFromViewport(inViewportClient))
		{
			return *worldContext;
		}
		return handleInvalidWorldContext();
	}

	const WorldContext& Engine::getWorldContextFromWorldChecked(const World* inWorld)const
	{
		if (const WorldContext* worldContext = getWorldContextFromWorld(inWorld))
		{
			return *worldContext;
		}
		return handleInvalidWorldContext();
	}

	const WorldContext* Engine::getWorldContextFromWorld(const World* inWorld) const
	{
		for (int i = 0; i < mWorldList.size(); i++)
		{
			if (mWorldList[i].getWorld() == inWorld)
			{
				return &mWorldList[i];
			}
		}
		return nullptr;
	}

	TArray<class LocalPlayer*>::TConstIterator Engine::getLocalPlayerIterator(World* world)
	{
		return getGamePlayers(world).createConstIterator();
	}

	bool Engine::isStereoscopic3D(Viewport* inViewport /* = nullptr */)
	{
		return false;
	}

	bool isInAsyncLoadingThreadCoreObjectInternal()
	{
		return ResLoader::instance().isInAsyncLoadThread();
	}

	bool isAsyncLoadingEngineInternal()
	{
		return ResLoader::instance().isAsyncLoading();
	}


	void EngineModule::startupModule()
	{
		isInAsyncLoadingThread = &isInAsyncLoadingThreadCoreObjectInternal;
		isAsyncLoading = &isAsyncLoadingEngineInternal;
	}

	World* Engine::getWorldFromContextObject(const Object* object, bool bChecked /* = true */) const
	{
		if (!bChecked && object == nullptr)
		{
			return nullptr;
		}
		BOOST_ASSERT(object);

		bool bSupported = true;
		World* world = ((bChecked && isInGameThread()) ? object->getWorldChecked(bSupported) : object->getWorld());
		return (bSupported ? world : GWorld);
	}

	ScopedConditionalWorldSwitcher::ScopedConditionalWorldSwitcher(class ViewportClient* inViewportClient)
		:mViewportClient(inViewportClient)
		, mWorld(nullptr)
	{

	}

	ScopedConditionalWorldSwitcher::~ScopedConditionalWorldSwitcher()
	{

	}

	IMPLEMENT_MODULE(EngineModule, Engine);
}