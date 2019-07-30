#pragma once
#include "EngineMininal.h"
#include "RHIDefinitions.h"
#include "ObjectMacros.h"
#include "Classes/GameFramework/WorldSettings.h"
#include "ObjectGlobals.h"
#include "RHI.h"
#include "Classes/Engine/EngineType.h"
#include "Classes/Engine/Level.h"
#include "Model.h"
#include "Classes/GameFramework/GameModeBase.h"
#include <set>
namespace Air
{
	class ActorComponent;
	class GameInstance;
	class AActor;
	class GameModeBase;
	class APlayerController;
	class TickTaskLevel;
	class AGameStateBase;
	class AController;

	struct InitializationValues
	{
		InitializationValues()
			:bInitializeScenes(true)
			,bAllowAudioPlayBack(false)
			,bRequiresHitProxies(false)
			,bCreatePhysicsScene(false)
			,bCreateNavigation(false)
			,bCreateAISystem(false)
			,bShouldSimulatePhysics(false)
			,bEnbleTraceCollision(false)
			,bTransactional(false)
			,bCreateFXSystem(false)
		{

		}

		uint32 bInitializeScenes: 1;
		uint32 bAllowAudioPlayBack : 1;
		uint32 bRequiresHitProxies : 1;
		uint32 bCreatePhysicsScene : 1;
		uint32 bCreateNavigation : 1;
		uint32 bCreateAISystem : 1;
		uint32 bShouldSimulatePhysics : 1;
		uint32 bEnbleTraceCollision : 1;
		uint32 bTransactional : 1;
		uint32 bCreateFXSystem : 1;

		InitializationValues& InitializeScenes(const bool bInitialize)
		{
			bInitializeScenes = bInitialize;
			return *this;
		}
		InitializationValues& allowAudioPlayBack(const bool allow)
		{
			bAllowAudioPlayBack = allow;
			return *this;
		}
		InitializationValues& requiresHitProxies(const bool requires)
		{
			bRequiresHitProxies = requires;
			return *this;
		}
		InitializationValues& createPhysicsScene(const bool create)
		{
			bCreatePhysicsScene = create;
			return *this;
		}

		InitializationValues& createNavigation(const bool create)
		{
			bCreateNavigation = create;
			return *this;
		}
		InitializationValues& createAISystem(const bool create)
		{
			bCreateAISystem = create;
			return *this;
		}
		InitializationValues& shouldSimulatePhysics(const bool should)
		{
			bShouldSimulatePhysics = should;
			return *this;
		}
		InitializationValues& enableTraceCollision(const bool enable)
		{
			bEnbleTraceCollision = enable;
			return *this;
		}

		InitializationValues& setTransactional(const bool btrans)
		{
			bTransactional = btrans;
			return *this;
		}
		InitializationValues& createFXSystem(const bool create)
		{
			bCreateFXSystem = create;
			return *this;
		}
	};

	struct ENGINE_API ActorSpawnParameters
	{
		ActorSpawnParameters(){}
		wstring Name{ TEXT("") };
		std::shared_ptr<AActor> mTemplate;
		std::shared_ptr<AActor> mOwner;
		EObjectFlags objectFlags{ RF_NoFlags };
		std::shared_ptr<class Level> mOverrideLevel;
	};

	struct ENGINE_API LevelCollection
	{
		LevelCollection();

		LevelCollection(const LevelCollection&) = delete;

		LevelCollection& operator=(const LevelCollection&) = delete;

		LevelCollection(LevelCollection&& other);
		LevelCollection& operator=(LevelCollection&& other);

		~LevelCollection();

		ELevelCollectionType getType() const { return mCollectionType; }

		void setGameState(class AGameStateBase* inGameState);

		const TSet<std::shared_ptr<Level>>& getLevels() const { return mLevels; }

		const Level* getPersistentLevel() const { return mPersistentLevel.get(); }

		std::shared_ptr<Level>& getPersistentLevel() { return mPersistentLevel; }

		const std::shared_ptr<AGameStateBase> getGameState() const { return mGameState; }

		void setType(const ELevelCollectionType inType) { mCollectionType = inType; }

		void setPersistentLevel(std::shared_ptr<Level> const level)
		{
			mPersistentLevel = level;
		}

		void addLevel(Level* const level);

	public:


		std::shared_ptr<AGameStateBase> mGameState;

		TSet<std::shared_ptr<Level>> mLevels;

		ELevelCollectionType mCollectionType;

		std::shared_ptr<class Level> mPersistentLevel;

		bool bIsVisible;
	};

	typedef TArray<std::shared_ptr<APlayerController>>::TConstIterator ConstPlayerControllerIterator;

	struct StartPhysicsTickFunction : public TickFunction
	{
		class World* mTarget;
		virtual void executeTick(float deltaTime, enum ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef & myCompletionGraphEvent) override;

		virtual wstring diagnosticMessage() override;
	};

	struct EndPhysicsTickFunction : public TickFunction
	{
		class World* mTarget;

		virtual void executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent) override;

		virtual wstring diagnosticMessage() override;
	};

	class ENGINE_API World final : public Object
	{
		GENERATED_RCLASS_BODY(World, Object)
	public:

		~World();

		bool isGameWorld() const;

		void updateLevelStreaming();

		void sendAllEndOfFrameUpdates();

		bool isCameraMoveable() const;

		bool isPaused() const;

		bool hasEndOfFrameUpdates();

		bool usesGameHiddenFlags() const;

		void initializeNewWorld(const InitializationValues = InitializationValues());

		static std::shared_ptr<World> createWorld(const EWorldType::Type inWorldType, bool bInformEngineOfWorld, wstring worldName = TEXT(""), ERHIFeatureLevel::Type inFeatureLevel = ERHIFeatureLevel::Num);

		WorldSettings* getWorldSettings(bool bCheckStreamingPesistent = false, bool bChecked = true) const;

		inline void setGameInstance(std::shared_ptr<GameInstance>& newGI)
		{
			mOwningGameIntance = newGI;
		}

		bool containsActor(AActor* actor);

		bool isServer() const;

		const std::shared_ptr<GameModeBase>& getAuthGameMode() const { return mAuthorityGameMode; }

		std::shared_ptr<AActor> spawnActor(RClass* inClass, float3 const * location, Rotator const* rotation, const ActorSpawnParameters& spawnParameters);

		std::shared_ptr<AActor> spawnActor(RClass* inClass, Transform const* userTransformPtr, const ActorSpawnParameters& spawnParameters);



		

		template<class T>
		std::shared_ptr<T> spawnActor(RClass* inClass, float3 const& location, Rotator const & rotation, const ActorSpawnParameters& spawnParamters = ActorSpawnParameters())
		{
			return std::dynamic_pointer_cast<T>(spawnActor(inClass, &location, &rotation, spawnParamters));
		}

		template<class T>
		std::shared_ptr<AActor> spawnActor(float3 const * location, Rotator const * rotation, const ActorSpawnParameters& spawnParameters)
		{
			Transform transform;
			if (location)
			{
				transform.setLocation(*location);
			}
			if (rotation)
			{
				transform.setRotation(Quaternion(*rotation));
			}
			return std::dynamic_pointer_cast<T>(spawnActor(T::StaticClass(), &transform, spawnParameters));
		}

		template<class T>
		std::shared_ptr<T> spawnActor(RClass* inClass, const ActorSpawnParameters& spawnParameters = ActorSpawnParameters())
		{
			return std::dynamic_pointer_cast<T>(spawnActor(inClass, nullptr, nullptr, spawnParameters));
		}

		template<class T>
		std::shared_ptr<T> spawnActor(const ActorSpawnParameters& spawnParameters = ActorSpawnParameters())
		{
			return std::dynamic_pointer_cast<T>(spawnActor<T>(nullptr, nullptr, spawnParameters));
		}

		std::shared_ptr<APlayerController> spawnPlayActor(class Player* player, ENetRole remoteRole);

		void initWorld(const InitializationValues ivs = InitializationValues());

		void updateWorldComponents(bool bRerunConstructionScripts, bool bCurrentLevelOnly);

		virtual World* getWorld() const override;

		bool setGameMode(const URL& inURL);

		inline std::shared_ptr<GameInstance> getGameInstance() const
		{
			return mOwningGameIntance;
		}

		FORCEINLINE_DEBUGGABLE float getUnpausedTimeSeconds() const
		{
			return mUnpausedTimeSeconds;
		}

		FORCEINLINE_DEBUGGABLE float getTimeSecondes() const

		{
			return mTimeSeconds;
		}

		AGameStateBase* getGameState() const { return mGameState.get(); }
		const LevelCollection* getActiveLevelCollection() const { return mActiveLevelCollection; }

		bool areActorsInitialized() const;

		bool hasBegunPlay() const;

		void beginPlay();

		void initializeActorsForPlay(const URL& inURL, bool bResetTime = true);

		void markActorComponentForNeededEndOfFrameUpate(ActorComponent* component, bool bForceGameThread);

		FORCEINLINE_DEBUGGABLE float getRealTimeSeconds() const
		{
			BOOST_ASSERT(!isInActualRenderinThread());
			return mRealTimeSeconds;
		}

		ConstPlayerControllerIterator getPlayerControllerIterator() const;

		bool destroyActor(AActor* actor, bool bNetForce = false, bool bShouldModifyLevel = true);

		void removeActor(AActor* actor, bool bShouldModifyLevel);

		void setGameState(AGameStateBase* newGameState);

		FORCEINLINE_DEBUGGABLE float getDeltaSeconds() const
		{
			return mDeltaTimeSeconds;
		}

		const TArray<std::shared_ptr<class Level>>& getLevels() const;

		bool setCurrentLevel(class Level* inLevel);

		void setShouldTick(const bool bInShouldTick) { bShouldTick = bInShouldTick; }

		bool shouldTick() const { return bShouldTick; }

		void tick(ELevelTick tickType, float deltaTime);

		void setupPhysicsTickFunctions(float deltaSeconds);

		void runTickGroup(ETickingGroup group, bool bBlockTillComplete = true);

		void setActiveLevelCollection(const LevelCollection* inCollection);

		LevelCollection* findCollectionByType(const ELevelCollectionType inType);

		LevelCollection& findOrAddCollectionByType(const ELevelCollectionType inType);

		const TArray<std::shared_ptr<AActor>> & getActors() const { return mActors; }


		void addController(AController* controller);

		void setSkyTexture(std::shared_ptr<class RTexture> tex);

		FORCEINLINE static TArray<World*>& getAllWorlds()
		{
			return mAllWorlds;
		}

		void addParameterCollectionInstances(class MaterialParameterCollection* collection, bool bUpdateScene);

		void updateParameterCollectionInstances(bool bUpdateInstanceConstantBuffers);

	private:
		void conditionallyCreateDefaultLevelCollections();
	public:
		class SceneInterface*		mScene{ nullptr };

		ERHIFeatureLevel::Type		mFeatureLevel;

		EWorldType::Type			mWorldType;

		std::shared_ptr<GameInstance>				mOwningGameIntance;

		std::shared_ptr<class RTexture> mSkyTexture;

		float3 mOriginOffsetThisFrame;

		uint32 bIsCameraMoveableWhenPaused : 1;

		uint32 bActorsInitalized : 1;

		uint32 bStartup : 1;

		uint32 bMatchStarted : 1;

		std::shared_ptr<Level> mCurrentLevel;

		std::shared_ptr<Level> mPersistentLevel;

		const LevelCollection*		mActiveLevelCollection{ nullptr };

		TArray<LevelCollection>			mLevelCollections;

		bool	bIsLevelStreamingFrozen;

		bool	bPostTickComponentUpdate;

		bool	bInTick;

		uint32 bBegunPlay : 1;

		class TickTaskLevel*			mTickTaskLevel{ nullptr };

		TArray<std::shared_ptr<APlayerController>> mPlayerControllerList;

		TArray<AController*> mControllerList;

		std::shared_ptr<class AGameStateBase>			mGameState;

		ETickingGroup					mTickGroup;

		StartPhysicsTickFunction		mStartPhysicsTickFunction;
	private:
		TSet<ActorComponent*> mComponentsThatNeedEndOfFrameUpdate;
		TSet<ActorComponent*> mComponentsThatNeedEndOfFrameUpdate_OnGameThread;

		TArray<std::shared_ptr<class RMaterialParameterCollectionInstance>> mParameterCollectionInstances;

		bool bShouldTick{ true };

		TArray<std::shared_ptr<AActor>> mActors;
	public:
		std::shared_ptr<GameModeBase>			mAuthorityGameMode;
		TArray<std::shared_ptr<Level>>			mLevels;
		bool bIsWorldInitialized{ false };
		bool bShouldSimulatePhysics{ false };
		bool bRequiresHitProxies{ false };

		float mUnpausedTimeSeconds;
		float mTimeSeconds;
		float mRealTimeSeconds;
		float mDeltaTimeSeconds;

		static TArray<World*> mAllWorlds;
	};

	class WorldProxy
	{
	public:
		WorldProxy() {}

		inline WorldProxy& operator =(World* inWorld)
		{
			mWorld = std::dynamic_pointer_cast<World>(inWorld->shared_from_this());
			return *this;
		}

		inline operator World* () const
		{
			BOOST_ASSERT(isInGameThread());
			return mWorld.get();
		}

	private:
		std::shared_ptr<World> mWorld;
	};

	class ENGINE_API ScopedLevelCollectionContextSwitch
	{
	public:
		ScopedLevelCollectionContextSwitch(const LevelCollection* const inLevelCollection, World* inWorld);

		~ScopedLevelCollectionContextSwitch();

	private:
		class World* mWorld;
		const LevelCollection* mSavedTickingCollection;
	};

	extern ENGINE_API class WorldProxy GWorld;
}