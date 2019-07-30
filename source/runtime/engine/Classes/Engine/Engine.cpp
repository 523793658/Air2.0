#include "Engine.h"
#include "Classes/Engine/GameInstance.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "Classes/Engine/LocalPlayer.h"
#include "Classes/GameFramework/PlayerController.h"
#include "RenderingThread.h"
#include "ResLoader/ResLoader.h"
#include "Misc/App.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformProperties.h"
#include "SimpleReflection.h"
namespace Air
{
	class LocalPlayer;

	static TArray<LocalPlayer*>	FakeEmptyLocalPlayers;

	Engine::Engine(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}

	WorldContext& Engine::createNewWorldContext(EWorldType::Type worldType)
	{
		WorldContext* newWorldContext = (new (mWorldList)WorldContext);
		newWorldContext->mWorldType = worldType;
		newWorldContext->mContextHandle = printf(TEXT("Context_%d"), mNextWorldContextHandle++);
		return *newWorldContext;
	}


	


	LocalPlayer* Engine::getFirstGamePlayer(World* inWorld)
	{
		const TArray<std::shared_ptr<class LocalPlayer>>& player = getGamePlayers(inWorld);
		return player.size() != 0 ? player[0].get() : nullptr;
	}


	EBrowseReturnVal::Type Engine::browse(WorldContext& worldContext, URL url)
	{
		wstring error;
		if (!url.valid)
		{
			/*if (!loadMap(worldContext, url, error))
			{
				return EBrowseReturnVal::Failure;
			}*/
		}
		if (url.isLocalInternal())
		{
			return loadMap(worldContext, url, nullptr, error) ? EBrowseReturnVal::Success : EBrowseReturnVal::Failure;
		}


		
		return EBrowseReturnVal::Success;
	}

	bool Engine::loadMap(WorldContext& worldContext, URL url, class APendingNetGame* pending, wstring& sError)
	{
		if (worldContext.getWorld())
		{
			worldContext.getWorld()->bIsLevelStreamingFrozen = false;
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND(FlushCommand, 
		{
			GRHICommandList.getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
		RHIFlushResource();
		GRHICommandList.getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
		});
		flushRenderingCommands();

		
		/*World* newWorld = nullptr;

		
		const wstring URLTrueMapName = url.mMap;

		if (newWorld == nullptr)
		{
			const wstring urlMapName = url.mMap;
			
		}*/
		worldContext.getWorld()->setGameInstance(worldContext.mOwningGameInstance);
		GWorld = worldContext.getWorld();
		if (worldContext.mWorldType == EWorldType::Game)
		{

		}

		

		if (!worldContext.getWorld()->bIsWorldInitialized)
		{
			worldContext.getWorld()->initWorld();
		}

		worldContext.getWorld()->initializeActorsForPlay(url);
		
		worldContext.getWorld()->beginPlay();

		redrawViewports(false);


		return true;
	}

	bool Engine::isAllowedFramerateSmoothing() const
	{
		return PlatformProperties::allowsFramerateSmoothing() && bSmoothFrameRate && bForceDisableFrameRateSmoothing;
	}

	void Engine::updateRunningAverageDeltaTime(float deltaTime, bool bAllowFrameRateSmoothing /* = true */)
	{
		if (bAllowFrameRateSmoothing &&isAllowedFramerateSmoothing())
		{
			if (deltaTime < 0.0f)
			{

			}
			mRunningAverageDeltaTime = Math::lerp<float>(mRunningAverageDeltaTime, Math::min<float>(deltaTime, 0.2f), 1 / 300.f);
		}
	}

	float Engine::getMaxTickRate(float deltaTime, bool bAllowFrameRateSmoothing /* = true */) const
	{
		float maxTickRate = 0;
		if (bAllowFrameRateSmoothing && isAllowedFramerateSmoothing())
		{
			maxTickRate = 1.f / mRunningAverageDeltaTime;
			if (mSmoothedFrameRateRange.hasLowerBound())
			{
				maxTickRate = Math::max(maxTickRate, mSmoothedFrameRateRange.getLowerBoundValue());
			}
			if (mSmoothedFrameRateRange.hasUpperBound())
			{
				maxTickRate = Math::min(maxTickRate, mSmoothedFrameRateRange.getUpperBoundValue());
			}
		}
		return maxTickRate;
	}
	void Engine::updateTimeAndHandleMaxTickRate()
	{
		static double lastTime = PlatformTime::seconds() - 0.0001;
		static bool bTimeWasManipulated = false;
		bool bTimeWasMaipulatedDebug = bTimeWasManipulated;

		const bool bUseFixedTimeStep = false;

		App::updateLastTime();

		if (bUseFixedTimeStep)
		{
			bTimeWasManipulated = true;
			const float frameRate = App::getFixedDeltaTime();
			App::setDeltaTime(frameRate);
			lastTime = App::getCurrentTime();
			App::setCurrentTime(App::getCurrentTime() + App::getDeltaTime());
		}
		else
		{
			App::setCurrentTime(PlatformTime::seconds());
			if (bTimeWasManipulated && !bUseFixedFrameRate)
			{
				lastTime = App::getCurrentTime() - App::getDeltaTime();
				bTimeWasManipulated = false;
			}
			float deltaTime = App::getCurrentTime() - lastTime;
			if (deltaTime < 0)
			{
				deltaTime = 0.01;
			}
			updateRunningAverageDeltaTime(deltaTime);

			const float givenMaxTickRate = getMaxTickRate(deltaTime);
			const float maxTickRate = givenMaxTickRate;
			float waitTime = 0;
			if (maxTickRate > 0)
			{
				waitTime = Math::max(1.f / maxTickRate - deltaTime, 0.f);
			}
			double actualWaitTime = 0.f;
			if (waitTime > 0)
			{
				double waitEndTime = App::getCurrentTime() + waitTime;
				if (waitTime > 5 / 1000.0f)
				{
					PlatformProcess::sleepNoStats(waitTime - 0.002f);
				}
				while (PlatformTime::seconds() < waitEndTime)
				{
					PlatformProcess::sleepNoStats(0);
				}
				App::setCurrentTime(PlatformTime::seconds());
			}
			else if (bUseFixedFrameRate && maxTickRate == mFixedFrameRate)
			{
				const float frameRate = 1.0f / mFixedFrameRate;
				App::setDeltaTime(frameRate);
				App::setCurrentTime(lastTime + App::getDeltaTime());
				bTimeWasManipulated = true;
			}

			double additionalWaitTimeInMs = (actualWaitTime - static_cast<double>(waitTime))*1000.0;
			App::setDeltaTime(App::getCurrentTime() - lastTime);
			App::setIdleTime(actualWaitTime);
			if (App::getDeltaTime() < 0)
			{
				App::setDeltaTime(0.01);
			}
			lastTime = App::getCurrentTime();
		}
	}

	void staticTick(float deltaTime, bool bUseFullTimeLimit, float asyncLoadingTime)
	{
		processAsyncLoading(true, bUseFullTimeLimit, asyncLoadingTime);
	}

	DECLARE_SIMPLER_REFLECTION(Engine);
}