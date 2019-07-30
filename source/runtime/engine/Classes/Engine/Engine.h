#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "Classes/Engine/LocalPlayer.h"
#include "Classes/Engine/WorldContext.h"
#include "Containers/IndirectArray.h"
namespace Air
{
	class World;
	class EngineLoop;
	class GameViewportClient;
	class ENGINE_API Engine : public Object
	{
		GENERATED_RCLASS_BODY(Engine, Object)
	public:
		virtual void tick(float deltaSeconds, bool bIdleMode) {}

		void cleanupGameViewport();

		virtual void redrawViewports(bool bShouldPresent = true) {}

		virtual void init(EngineLoop* inEngineLoop);

		virtual void start();

		virtual void updateRunningAverageDeltaTime(float deltaTiem, bool bAllowFrameRateSmoothing = true);

		virtual bool isAllowedFramerateSmoothing() const;

		void initializeRunningAverageDeltaTime();

		LocalPlayer* getFirstGamePlayer(World* inWorld);

		LocalPlayer* getGamePlayer(World* inWorld, int32 inPlayer);
		LocalPlayer* getGamePlayer(const ViewportClient* inViewport, int32 inPlayer);
		LocalPlayer* getLocalPlayerFromControllerId(const ViewportClient* inViewport, const int32 controllerId) const;


		const TArray<std::shared_ptr<class LocalPlayer>>& getGamePlayers(World* inWorld) const;
		const TArray<std::shared_ptr<class LocalPlayer>>& getGamePlayers(const ViewportClient* viewport) const;

		const WorldContext& getWorldContextFromWorldChecked(const World* inWorld) const;

		const WorldContext* getWorldContextFromWorld(const World* inWorld) const;

		const WorldContext* getWorldContextFromViewport(const ViewportClient* inViewport) const ;


		WorldContext* getWorldContextFromViewport(const ViewportClient* inViewport);

		WorldContext& getWorldContextFromViewportChecked(const ViewportClient* inViewportClient);

		const WorldContext& getWorldContextFromViewportChecked(const ViewportClient* inViewportClient) const;

		WorldContext& createNewWorldContext(EWorldType::Type worldType);

		TArray<std::shared_ptr<class LocalPlayer>>::TConstIterator getLocalPlayerIterator(World* world);

		void worldAdded(std::shared_ptr<World>& inWorld)
		{

		}

		virtual EBrowseReturnVal::Type browse(WorldContext& worldContext, URL url);

		bool isStereoscopic3D(Viewport* inViewport = nullptr);

		void updateTimeAndHandleMaxTickRate();

		World* getWorldFromContextObject(const Object* object, const bool bChecked = true) const;
	public:
	private:
		virtual bool loadMap(WorldContext& worldContext, URL url, class APendingNetGame* pending, wstring & sError);

	protected:
		virtual float getMaxTickRate(float deltaTime, bool bAllowFrameRateSmoothing = true) const;
	public:

		EngineLoop*	mEngineLoop;

		float mRunningAverageDeltaTime;

		float mDisplayGamma;

		bool bIsInitiailized{ false };

		TindirectArray<WorldContext> mWorldList;

		int32 mNextWorldContextHandle;

		uint32 bSmoothFrameRate : 1;
		uint32 bUseFixedFrameRate : 1;
		uint32 bForceDisableFrameRateSmoothing : 1;
		float mFixedFrameRate;

		RClass* mLocalPlayerClass;

		FloatRange mSmoothedFrameRateRange;

		TArray<std::shared_ptr<class ISceneViewExtension>> mViewExtensions;
	};

	extern ENGINE_API std::shared_ptr<class Engine>				GEngine;

	ENGINE_API void staticTick(float deltaTime, bool bUseFullTimeLimit = true, float asyncLoadingTime = 0.005f);
}