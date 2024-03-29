#include "Classes/Engine/World.h"
#include "Classes/GameFramework/GameModeBase.h"
#include "Engine.h"
#include "ObjectGlobals.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "Misc/App.h"
#include "Async/ParallelFor.h"
#include "HAL/PlatformTime.h"
#include "SimpleReflection.h"
#include "Classes/Engine/GameInstance.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/GameFramework/GameStateBase.h"
#include "TickTaskManagerInterface.h"
#include "Classes/Materials/MaterialParameterCollectionInstance.h"
#include "Class.h"
namespace Air
{
	WorldProxy GWorld;
	namespace EComponentMarkedForEndOfFrameUpdateState
	{
		enum Type
		{
			Unmarked,
			Marked,
			MarkedForGameThread,
		};
	}

	struct MarkComponentEndOfFrameUpdateState
	{
		friend class World;
	private:
		FORCEINLINE static void set(ActorComponent* component, EComponentMarkedForEndOfFrameUpdateState::Type updateState)
		{
			BOOST_ASSERT(updateState < 4);
			component->mMarkedForEndOfFrameUpdateState = updateState;
		}
	};

	TArray<World*> World::mAllWorlds;

	World::World(const ObjectInitializer& objectInitializer)
		:ParentType(objectInitializer)
		,mActiveLevelCollection(nullptr)
		,mFeatureLevel(GMaxRHIFeatureLevel)
		,bShouldTick(true)
		,mTickTaskLevel(TickTaskManagerInterface::get().allocateTickTaskLevel())
	{
		mAllWorlds.add(this);
	}

	World::~World()
	{
		mAllWorlds.remove(this);
	}

	std::shared_ptr<AActor> World::spawnActor(RClass* inClass, float3 const * location, Rotator const* rotation, const ActorSpawnParameters& spawnParameters)
	{
		Transform transform;
		if (location)
		{
			transform.setLocation(*location);
		}

		if (rotation)
		{
			transform.setRotation(*rotation);
		}

		return spawnActor(inClass, &transform, spawnParameters);
	}

	std::shared_ptr<AActor> World::spawnActor(RClass* inClass, Transform const* userTransformPtr, const ActorSpawnParameters& spawnParameters)
	{
		Level* levelToSpawnIn = spawnParameters.mOverrideLevel.get();
		if (levelToSpawnIn == nullptr)
		{
			levelToSpawnIn = (spawnParameters.mOwner) ? dynamic_cast<Level*>(spawnParameters.mOwner->getOuter()) : mCurrentLevel.get();
		}
		wstring newActorName = spawnParameters.Name;
		AActor* Template = spawnParameters.mTemplate.get();
		if (!Template)
		{
			Template = inClass->getDefaultObject<AActor>();
		}

		Transform const userTransform = userTransformPtr ? *userTransformPtr : Transform::identity;

		std::shared_ptr<AActor> actor = newObject<AActor>(levelToSpawnIn, inClass, newActorName, spawnParameters.objectFlags, Template);
		levelToSpawnIn->mActors.add(actor);
		actor->postSpawnInitialize(userTransform,   spawnParameters.mOwner);
		mActors.add(actor);
		return actor;
	}

	void World::initializeActorsForPlay(const URL& inURL, bool bResetTime /* = true */)
	{
		BOOST_ASSERT(bIsWorldInitialized);
		double startTime = PlatformTime::seconds();
		if (bResetTime)
		{
			mTimeSeconds = 0.0f;
			mUnpausedTimeSeconds = 0.0f;
			mRealTimeSeconds = 0.0f;
		}

		updateWorldComponents(false, true);

		for (int32 levelIndex = 0; levelIndex < mLevels.size(); levelIndex++)
		{
			std::shared_ptr<Level> level = mLevels[levelIndex];
		}

		if (!areActorsInitialized())
		{
			bActorsInitalized = true;

			for (int32 levelIndex = 0; levelIndex < mLevels.size(); levelIndex++)
			{
				std::shared_ptr<Level> const level = mLevels[levelIndex];
				level->routeActorInitialize();
			}
		}


		BOOST_ASSERT(mLevels.size());
		BOOST_ASSERT(mPersistentLevel);
		BOOST_ASSERT(mLevels[0] == mPersistentLevel);
		for (int32 levelIndex = 0; levelIndex < mLevels.size(); levelIndex++)
		{
			std::shared_ptr<Level> level = mLevels[levelIndex];
			level->sortActorList();
		}



	}

	bool World::containsActor(AActor* actor)
	{
		return (actor && actor->getWorld() == this);
	}

	void World::beginPlay()
	{
		std::shared_ptr<GameModeBase> const gameMode = getAuthGameMode();
		if (gameMode)
		{
			gameMode->startPlay();
		}
	}

	bool World::isCameraMoveable() const
	{
		bool bIsCameraMoveable = (!isPaused() || bIsCameraMoveableWhenPaused);
		return bIsCameraMoveable;
	}
	bool World::isPaused() const
	{
		return false;
	}

	bool World::isGameWorld() const
	{
		return mWorldType == EWorldType::Game || mWorldType == EWorldType::Demo;
	}

	bool World::usesGameHiddenFlags() const
	{
		return isGameWorld();
	}

	void World::updateLevelStreaming()
	{

	}

	void World::sendAllEndOfFrameUpdates()
	{
		TGuardValue<bool> GuardIsFlushedGlobal(bPostTickComponentUpdate, true);
		static TArray<ActorComponent*> localComponentsThatNeedEndFrameUpdate;
		{
			BOOST_ASSERT(isInGameThread() && !localComponentsThatNeedEndFrameUpdate.size());
			localComponentsThatNeedEndFrameUpdate.reserve(mComponentsThatNeedEndOfFrameUpdate.size());
			for (auto& elem : mComponentsThatNeedEndOfFrameUpdate)
			{
				localComponentsThatNeedEndFrameUpdate.add(elem);
			}
		}
		auto paralleWork = [](int32 index)
		{
			ActorComponent* nextComponent = localComponentsThatNeedEndFrameUpdate[index];
			if (nextComponent)
			{
				if (nextComponent->isRegistered() && !nextComponent->isTemplate() && !nextComponent->isPendingKill())
				{
					nextComponent->doDeferredRenderUpdates_Concurrent();
				}
				BOOST_ASSERT(nextComponent->getMarkedForEndOfFrameUpdateState() == EComponentMarkedForEndOfFrameUpdateState::Marked);
				MarkComponentEndOfFrameUpdateState::set(nextComponent, EComponentMarkedForEndOfFrameUpdateState::Unmarked);
			}
		};
		auto GTWork = 
			[this]()
		{
			for (ActorComponent* component : mComponentsThatNeedEndOfFrameUpdate_OnGameThread)
			{
				if (component->isRegistered() && !component->isTemplate() && !component->isPendingKill())
				{
					component->doDeferredRenderUpdates_Concurrent();
				}
				BOOST_ASSERT(component->getMarkedForEndOfFrameUpdateState() == EComponentMarkedForEndOfFrameUpdateState::MarkedForGameThread);
				MarkComponentEndOfFrameUpdateState::set(component, EComponentMarkedForEndOfFrameUpdateState::Unmarked);
			}
			mComponentsThatNeedEndOfFrameUpdate_OnGameThread.reset();
			mComponentsThatNeedEndOfFrameUpdate.reset();
		};
		{
			GTWork();
			parallelFor(localComponentsThatNeedEndFrameUpdate.size(), paralleWork);
		}
		localComponentsThatNeedEndFrameUpdate.reset();
	}

	bool World::hasEndOfFrameUpdates()
	{
		return mComponentsThatNeedEndOfFrameUpdate_OnGameThread.size() > 0 || mComponentsThatNeedEndOfFrameUpdate.size() > 0;
	}

	void World::initializeNewWorld(const InitializationValues ivs/* = InitializationValues() */)
	{
		if (!ivs.bTransactional)
		{
		}
		mPersistentLevel = newObject<Level>(this, TEXT("PersistentLevel"));
		mPersistentLevel->initialize(URL(nullptr));
		mPersistentLevel->mModel = newObject<Model>(mPersistentLevel.get());
		mPersistentLevel->mModel->initialize(nullptr);
		mPersistentLevel->mOwningWorld = std::dynamic_pointer_cast<World>(this->shared_from_this());

		if (ivs.bTransactional)
		{

		}

		ActorSpawnParameters spawnInfo;
		mCurrentLevel = mPersistentLevel;
		std::shared_ptr<WorldSettings> worldSettings = spawnActor<WorldSettings>(spawnInfo);
		mPersistentLevel->setWorldSettings(worldSettings);
		initWorld(ivs);
		updateWorldComponents(true, false);
	}

	void World::initWorld(const InitializationValues ivs /* = InitializationValues() */)
	{
		if (bIsWorldInitialized)
		{
			return;
		}
		if (ivs.bInitializeScenes)
		{
			if (ivs.bCreatePhysicsScene)
			{
				//创建物理场景
			}
			bShouldSimulatePhysics = ivs.bShouldSimulatePhysics;
			bRequiresHitProxies = ivs.bRequiresHitProxies;

			getRendererModule().allocateScene(this, bRequiresHitProxies, ivs.bCreateFXSystem, mFeatureLevel);
		}
		mLevels.empty(1);
		mLevels.add(mPersistentLevel);
		mPersistentLevel->mOwningWorld = std::dynamic_pointer_cast<World>(this->shared_from_this());
		mPersistentLevel->bIsVisible = true;

		mCurrentLevel = mPersistentLevel;

		conditionallyCreateDefaultLevelCollections();

		bIsWorldInitialized = true;
		
	}

	World* World::getWorld() const
	{
		return const_cast<World*>( this);
	}



	bool World::setGameMode(const URL& inURL)
	{
		if (isServer() && !mAuthorityGameMode)
		{
			mAuthorityGameMode = getGameInstance()->createGameModeForURL(inURL);
			if (mAuthorityGameMode != nullptr)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}

	void World::updateWorldComponents(bool bRerunConstructionScripts, bool bCurrentLevelOnly)
	{
		if (bCurrentLevelOnly)
		{
			BOOST_ASSERT(mCurrentLevel);
			mCurrentLevel->updateLevelComponents(bRerunConstructionScripts);
		}
		else
		{
			/*for (int32 levelIndex = 0; levelIndex < mLevels.size(); levelIndex++)
			{
				Level* level = mLevels[levelIndex];
				
			}*/
		}
	}
	std::shared_ptr<World> World::createWorld(const EWorldType::Type inWorldType, bool bInformEngineOfWorld, wstring worldName /* = TEXT("") */, ERHIFeatureLevel::Type inFeatureLevel /* = ERHIFeatureLevel::Num */)
	{
		if (inFeatureLevel >= ERHIFeatureLevel::Num)
		{
			inFeatureLevel = GMaxRHIFeatureLevel;
		}
		std::shared_ptr<World> newWorld = newObject<World>();
		newWorld->mWorldType = inWorldType;
		newWorld->mFeatureLevel = inFeatureLevel;
		newWorld->initializeNewWorld();

		if ((GEngine) && bInformEngineOfWorld == true)
		{
			GEngine->worldAdded(newWorld);
		}
		return newWorld;
	}

	WorldSettings* World::getWorldSettings(bool bCheckStreamingPesistent /* = false */, bool bChecked /* = true */) const
	{
		BOOST_ASSERT(!isInActualRenderinThread());
		WorldSettings* worldSettings;
		if (mPersistentLevel)
		{
			worldSettings = mPersistentLevel->getWorldSettings(bChecked);
		}
		return worldSettings;
	}

	bool World::isServer() const
	{
		return true;
	}

	std::shared_ptr<APlayerController> World::spawnPlayActor(class Player* player, ENetRole remoteRole)
	{
		const std::shared_ptr<GameModeBase>& gameMode = getAuthGameMode();

		std::shared_ptr<APlayerController> newPlayerController = gameMode->login(player, remoteRole, TEXT(""), TEXT(""));

		newPlayerController->mRole = ROLE_Authority;
		newPlayerController->setPlayer(player);
		return newPlayerController;
	}


	bool World::destroyActor(AActor* actor, bool bNetForce /* = false */, bool bShouldModifyLevel /* = true */)
	{
		BOOST_ASSERT(actor);
		if (actor->getWorld() == nullptr)
		{

		}
		if (actor->isPendingKill())
		{
			return true;
		}

		if (isGameWorld())
		{
			if (getWorldSettings() == actor)
			{
				return false;
			}

			if (actor->mRole != ROLE_Authority && !bNetForce)
			{
				return false;
			}


		}
		else
		{
			actor->modify();
		}

		MarkActorIsBeingDestroyed markActorIsBeingDestroyed(actor);

		actor->destroyed();

		TArray<AActor*> attachedActors;
		actor->getAttachedActors(attachedActors);
		if (attachedActors.size() > 0)
		{
			TInlineComponentArray<SceneComponent*> sceneComponents;
			actor->getComponents(sceneComponents);

			for (TArray<AActor*>::TConstIterator attachedActorIt(attachedActors); attachedActorIt; ++attachedActorIt)
			{
				AActor* childActor = *attachedActorIt;
				if (childActor != nullptr)
				{
					for (SceneComponent* sceneComponent : sceneComponents)
					{
						childActor->detachAllSceneComponents(sceneComponent, DetachmentTransformRules::keepWorldTransform);
					}
				}
			}
		}
		SceneComponent* rootComp = actor->getRootComponent();
		if (rootComp != nullptr && rootComp->getAttachParent() != nullptr)
		{
			AActor* oldParentActor = rootComp->getAttachParent()->getOwner();
			if (oldParentActor)
			{
				oldParentActor->modify();
			}
			actor->detachRootComponentFromParent();

		}

		if (actor->getOwner())
		{
			actor->setOwner(nullptr);
		}

		if (GEngine->getWorldContextFromWorld(this))
		{

		}

		removeActor(actor, bShouldModifyLevel);

		actor->unregisterAllComponents();

		actor->markPendingKill();
		actor->markComponentsAsPendingKill();

		const bool bRegisterTickFunctions = false;
		const bool bIncludeComponents = true;

		actor->registerAllActorTickFunctions(bRegisterTickFunctions, bIncludeComponents);

		return true;
	}


	void World::removeActor(AActor* actor, bool bShouldModifyLevel)
	{
		bool bSuccessfulRemoval = false;
		Level* checkLevel = actor->getLevel();
		int32 actorListIndex = checkLevel->mActors.find(std::dynamic_pointer_cast<AActor>(actor->shared_from_this()));
		if (actorListIndex != INDEX_NONE)
		{
			if (bShouldModifyLevel && false)
			{
			
			}
			if (!isGameWorld())
			{
				checkLevel->mActors[actorListIndex]->modify();
			}

			checkLevel->mActors[actorListIndex] = nullptr;
			bSuccessfulRemoval = true;
		}

		BOOST_ASSERT(checkLevel->mActors.size() >= 2);

		if (!bSuccessfulRemoval && !(actor->getFlags() & RF_Transactional))
		{
			bSuccessfulRemoval = true;
		}

		if (!bSuccessfulRemoval)
		{

		}
	}
	ConstPlayerControllerIterator World::getPlayerControllerIterator() const
	{
		auto result = mPlayerControllerList.createConstIterator();
		return (const ConstPlayerControllerIterator&)result;
	}

	

	bool World::areActorsInitialized() const
	{
		return bActorsInitalized && mPersistentLevel && mPersistentLevel->mActors.size();
	}

	bool World::hasBegunPlay() const
	{
		return bBegunPlay && mPersistentLevel && mPersistentLevel->mActors.size();
	}

	void World::markActorComponentForNeededEndOfFrameUpate(ActorComponent* component, bool bForceGameThread)
	{
		BOOST_ASSERT(!bPostTickComponentUpdate);
		uint32 currentState = component->getMarkedForEndOfFrameUpdateState();

		if (currentState == EComponentMarkedForEndOfFrameUpdateState::Marked && bForceGameThread)
		{
			mComponentsThatNeedEndOfFrameUpdate.remove(component);
			currentState = EComponentMarkedForEndOfFrameUpdateState::Unmarked;
		}
		if (currentState == EComponentMarkedForEndOfFrameUpdateState::Unmarked)
		{
			if (!bForceGameThread)
			{
				bool bAllowConcurrentUpdates = App::shouldUseThreadingForPerformance();
				bForceGameThread = !bAllowConcurrentUpdates;
			}
			if (bForceGameThread)
			{
				mComponentsThatNeedEndOfFrameUpdate_OnGameThread.add(component);
				MarkComponentEndOfFrameUpdateState::set(component, EComponentMarkedForEndOfFrameUpdateState::MarkedForGameThread);
			}
			else
			{
				mComponentsThatNeedEndOfFrameUpdate.add(component);
				MarkComponentEndOfFrameUpdateState::set(component, EComponentMarkedForEndOfFrameUpdateState::Marked);
			}
		}
	}

	void World::setGameState(AGameStateBase* newGameState)
	{
		if (newGameState == mGameState.get())
		{
			return;
		}

		mGameState = std::dynamic_pointer_cast<AGameStateBase>(newGameState->shared_from_this());

		const Level* const cachedLevel = newGameState->getLevel();
		LevelCollection* const foundCollection = newGameState ? cachedLevel->getCachedLevelCollection() : nullptr;
		if (foundCollection)
		{
			foundCollection->setGameState(newGameState);
		}
	}

	const TArray<std::shared_ptr<class Level>>& World::getLevels() const
	{
		return mLevels;
	}

	bool World::setCurrentLevel(class Level* inLevel)
	{
		bool bChanged = false;
		if (mCurrentLevel.get() != inLevel)
		{
			mCurrentLevel = std::dynamic_pointer_cast<Level>(inLevel->shared_from_this());
			bChanged = true;
		}
		return bChanged;
	}

	void World::setActiveLevelCollection(const LevelCollection* inCollection)
	{
		mActiveLevelCollection = inCollection;
		mPersistentLevel = std::dynamic_pointer_cast<Level>(const_cast<LevelCollection*>(inCollection)->getPersistentLevel()->shared_from_this());
		if (isGameWorld())
		{
			setCurrentLevel(const_cast<Level*>(inCollection->getPersistentLevel()));
		}
		mGameState = inCollection->getGameState();
	}

	ScopedLevelCollectionContextSwitch::ScopedLevelCollectionContextSwitch(const LevelCollection* const inLevelCollection, World* inWorld)
		:mWorld(inWorld),
		mSavedTickingCollection(inWorld ? inWorld->getActiveLevelCollection() : nullptr)
	{
		if (inLevelCollection && mWorld)
		{
			mWorld->setActiveLevelCollection(inLevelCollection);
		}
	}
	
	ScopedLevelCollectionContextSwitch::~ScopedLevelCollectionContextSwitch()
	{
		if (mWorld)
		{
			mWorld->setActiveLevelCollection(mSavedTickingCollection);
		}
	}
	LevelCollection* World::findCollectionByType(const ELevelCollectionType inType)
	{
		for (LevelCollection& lc : mLevelCollections)
		{
			if (lc.getType() == inType)
			{
				return &lc;
			}
		}
		return nullptr;
	}

	LevelCollection& World::findOrAddCollectionByType(const ELevelCollectionType inType)
	{
		for (LevelCollection& lc : mLevelCollections)
		{
			if (lc.getType() == inType)
			{
				return lc;
			}
		}
		LevelCollection newLC;
		newLC.setType(inType);
		mLevelCollections.add(std::move(newLC));
		return mLevelCollections.last();
	}

	void World::conditionallyCreateDefaultLevelCollections()
	{
		if (!findCollectionByType(ELevelCollectionType::DynamicSourceLevels))
		{
			LevelCollection& dynamicCollection = findOrAddCollectionByType(ELevelCollectionType::DynamicSourceLevels);
			dynamicCollection.setPersistentLevel(mPersistentLevel);

			if (mPersistentLevel->getCachedLevelCollection() == nullptr)
			{
				dynamicCollection.addLevel(mPersistentLevel.get());
			}
			mActiveLevelCollection = &dynamicCollection;
		}
		if (!findCollectionByType(ELevelCollectionType::StaticLevels))
		{
			LevelCollection& staticCollection = findOrAddCollectionByType(ELevelCollectionType::StaticLevels);
			staticCollection.setPersistentLevel(mPersistentLevel);
		}
	}

	void LevelCollection::addLevel(Level* const level)
	{
		if (level)
		{
			BOOST_ASSERT(level->getCachedLevelCollection() == nullptr);
			mLevels.add(std::dynamic_pointer_cast<Level>(level->shared_from_this()));
			level->setCachedLevelToCollection(this);
		}
	}

	LevelCollection::LevelCollection()
		:mCollectionType(ELevelCollectionType::DynamicSourceLevels)
		,mGameState(nullptr)
		,mPersistentLevel(nullptr)
		,bIsVisible(true)
	{}

	LevelCollection::~LevelCollection()
	{
		for (std::shared_ptr<Level>& level : mLevels)
		{
			if (level)
			{
				BOOST_ASSERT(level->getCachedLevelCollection() == this);
				level->setCachedLevelToCollection(nullptr);
			}
		}
	}

	LevelCollection::LevelCollection(LevelCollection&& other)
		:mCollectionType(other.mCollectionType)
		,mGameState(other.mGameState)
		,mPersistentLevel(other.mPersistentLevel)
		,mLevels(std::move(other.mLevels))
		,bIsVisible(other.bIsVisible)
	{
		for (std::shared_ptr<Level>& level : mLevels)
		{
			if (level)
			{
				BOOST_ASSERT(level->getCachedLevelCollection() == &other);
				level->setCachedLevelToCollection(this);
			}
		}
	}

	LevelCollection& LevelCollection::operator=(LevelCollection&& other)
	{
		if (this != &other)
		{
			mCollectionType = other.mCollectionType;
			mGameState = other.mGameState;
			mPersistentLevel = other.mPersistentLevel;
			mLevels = std::move(other.mLevels);
			bIsVisible = other.bIsVisible;
			for (std::shared_ptr<Level>& level : mLevels)
			{
				if (level)
				{
					BOOST_ASSERT(level->getCachedLevelCollection() == &other);
					level->setCachedLevelToCollection(this);
				}
			}
		}
		return *this;
	}

	void World::addController(AController* controller)
	{
		BOOST_ASSERT(controller);
		mControllerList.addUnique(controller);
		if (dynamic_cast<APlayerController*>(controller))
		{
			mPlayerControllerList.addUnique(std::dynamic_pointer_cast<APlayerController>(controller->shared_from_this()));
		}
	}

	void World::setSkyTexture(std::shared_ptr<class RTexture> tex)
	{
		mSkyTexture = tex;
	}

	void World::addParameterCollectionInstances(class MaterialParameterCollection* collection, bool bUpdateScene)
	{
		int32 existingIndex = INDEX_NONE;
		for (int32 instantceIndex = 0; instantceIndex < mParameterCollectionInstances.size(); instantceIndex++)
		{
			if (mParameterCollectionInstances[instantceIndex]->getCollection() == collection)
			{
				existingIndex = instantceIndex;
				break;
			}
		}
		std::shared_ptr<RMaterialParameterCollectionInstance> newInstance = newObject<RMaterialParameterCollectionInstance>();
		newInstance->setCollection(collection, this);
		if (existingIndex != INDEX_NONE)
		{
			mParameterCollectionInstances[existingIndex] = newInstance;
		}
		else
		{
			mParameterCollectionInstances.add(newInstance);
		}

		newInstance->updateRenderState(true);
		if (bUpdateScene)
		{
			updateParameterCollectionInstances(false, false);
		}
	}

	void World::updateParameterCollectionInstances(bool bUpdateInstanceConstantBuffers, bool bRecreateConstantBuffer)
	{
		if (mScene)
		{
			TArray<MaterialParameterCollectionInstanceResource*> instanceResources;
			for (int32 instanceIndex = 0; instanceIndex < mParameterCollectionInstances.size(); instanceIndex++)
			{
				RMaterialParameterCollectionInstance* instance = mParameterCollectionInstances[instanceIndex].get();
				if (bUpdateInstanceConstantBuffers)
				{
					instance->updateRenderState(bRecreateConstantBuffer);
				}
				instanceResources.add(instance->getResource());
			}
			mScene->updateParameterCollections(instanceResources);
		}
	}

	void LevelCollection::setGameState(class AGameStateBase* inGameState)
	{
		mGameState = std::dynamic_pointer_cast<class AGameStateBase>(inGameState->shared_from_this());
	}

	DECLARE_SIMPLER_REFLECTION(World);
}