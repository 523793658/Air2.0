#include "Classes/GameFramework/Actor.h"
#include "Classes/Engine/World.h"
#include "Classes/Engine/Level.h"
#include "SimpleReflection.h"
#include "Classes/Components/ActorComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "HAL/ThreadSingleton.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Components/ChildActorComponent.h"
namespace Air
{

	uint32 AActor::mBeginPlayCallDepth = 0;

	static SceneComponent* getUnregisteredParent(ActorComponent* component)
	{
		SceneComponent* parentComponent = nullptr;
		SceneComponent* sceneComponent = dynamic_cast<SceneComponent*>(component);
		while (sceneComponent && sceneComponent->getAttachParent() && sceneComponent->getAttachParent()->getOwner() == component->getOwner() && !sceneComponent->getAttachParent()->isRegistered())
		{
			sceneComponent = sceneComponent->getAttachParent();
			if (sceneComponent->bAutoRegister)
			{
				parentComponent = sceneComponent;
			}
		}
		return parentComponent;
	}

	static void diapatchOnComponentsCreated(AActor* newActor)
	{
		TInlineComponentArray<ActorComponent*> components;
		newActor->getComponents(components);
		for (ActorComponent* cmp : components)
		{
			if (cmp && !cmp->hasBeenCreated())
			{
				cmp->onComponentCreated();
			}
		}
	}

	static SceneComponent* fixupNativeActorCompoment(AActor* actor)
	{
		SceneComponent* sceneRootComponent = actor->getRootComponent();
		if (sceneRootComponent == nullptr)
		{
			TInlineComponentArray<SceneComponent*> sceneComponets;
			actor->getComponents(sceneComponets);
			if (sceneComponets.size() > 0)
			{
				for (SceneComponent* componet : sceneComponets)
				{
					if ((componet == nullptr) || (componet->getAttachParent() != nullptr) || (componet->mCreationMethod != EComponentCreationMethod::Native))
					{
						continue;
					}
					sceneRootComponent = componet;
					actor->setRootComponent(componet);
					break;
				}
			}
		}
		return sceneRootComponent;
	}

	AActor::AActor(const ObjectInitializer & objectInitializer)
		:ParentType(objectInitializer)
	{
		initializeDefaults();
	}

	void AActor::initializeDefaults()
	{
		mPrimaryActorTick.mTickGroup = TG_PrePhysics;

		mPrimaryActorTick.bCanEverTick = false;

		mPrimaryActorTick.bStartWithTickEnabled = true;

		mPrimaryActorTick.setTickFunctionEnable(false);

		mRole = ROLE_Authority;
		bHasDeferredComponentRegistration = false;

	}

	void AActor::addOwnedComponent(ActorComponent* component)
	{
		BOOST_ASSERT(component->getOwner() == this);
		bool bAlreadyInSet = false;
		mOwnedComponents.add(component, &bAlreadyInSet);
		

	}

	void AActor::postSpawnInitialize(Transform const& userSpawnTransform, AActor* inOwner)
	{
		World* const world = getWorld();
		bool const bActorsInitialized = world && world->areActorsInitialized();
		mCreationTime = (world ? world->getTimeSecondes() : 0.f);

		SceneComponent* const sceneRootComponent = fixupNativeActorCompoment(this);
		if (sceneRootComponent != nullptr)
		{
			const Transform rootTransform(sceneRootComponent->mRelativeRotation, sceneRootComponent->mRelativeLocation, sceneRootComponent->mRelativeScale3D);
			const Transform finalRootComponentTransform = rootTransform * userSpawnTransform;
			sceneRootComponent->setWorldTransform(finalRootComponentTransform);
		}

		diapatchOnComponentsCreated(this);

		registerAllComponents();

		setOwner(inOwner);
		
		postActorCreated();

		finishSpawning(userSpawnTransform, true);
	}

	WorldSettings* AActor::getWorldSettings() const
	{
		World* world = getWorld();
		return (world ? world->getWorldSettings() : nullptr);
	}

	World* AActor::getWorld() const
	{
		if (Level* level = getLevel())
		{
			return level->mOwningWorld;
		}
		return nullptr;
	}

	bool AActor::destroy(bool bNetForce /* = false */, bool bShouldModifyLevel /* = true */)
	{
		return true;
	}

	bool AActor::setActorRotation(Rotator newRotation, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (mRootComponent)
		{
			return mRootComponent->moveComponent(float3::Zero, newRotation, true);
		}
		return false;
	}

	void AActor::finishSpawning(const Transform& transfrom, bool bIsDefaultTransform /* = false */)
	{
		BOOST_ASSERT(!bhasFinishedSpawning);
		if (!bhasFinishedSpawning)
		{
			bhasFinishedSpawning = true;
			Transform finalRootComponentTransform = (mRootComponent ? mRootComponent->mComponentToWorld : transfrom);
			if (mRootComponent && !bIsDefaultTransform)
			{

			}

			{
				postActorConstruction();
			}
		}
	}

	void AActor::registerAllComponents()
	{
		BOOST_ASSERT(incrementalRegisterComponents(0));
		bHasDeferredComponentRegistration = false;
	}

	bool AActor::incrementalRegisterComponents(int32 numComponentsToRegister)
	{
		if (numComponentsToRegister == 0)
		{
			numComponentsToRegister = std::numeric_limits<int32>::max();
		}

		World* const world = getWorld();
		BOOST_ASSERT(world);
		if (bAllowTickBeforeBeginPlay || !world->isGameWorld())
		{
			registerAllActorTickFunctions(true, false);
		}

		if (mRootComponent != nullptr && !mRootComponent->isRegistered())
		{
			if (mRootComponent->bAutoRegister)
			{
				mRootComponent->registerComponentWithWorld(world);
			}
		}
		int32 numTotalRegisteredComponents = 0;
		int32 numRegisteredComponentsThisRun = 0;
		TInlineComponentArray<ActorComponent*> components;
		getComponents(components);
		TSet<ActorComponent*> registeredParemts;
		for (int32 compIdx = 0; compIdx < components.size() && numRegisteredComponentsThisRun < numComponentsToRegister; compIdx++)
		{
			ActorComponent* component = components[compIdx];
			if (!component->isRegistered() && component->bAutoRegister)
			{
				SceneComponent* unregisteredParentComponent = getUnregisteredParent(component);
				if (unregisteredParentComponent)
				{
					bool bParentAlreadyHandled = false;
					registeredParemts.add(unregisteredParentComponent, &bParentAlreadyHandled);
					if (bAllowTickBeforeBeginPlay)
					{

					}
					component = unregisteredParentComponent;
					compIdx--;
					numTotalRegisteredComponents--;
				}
				component->registerComponentWithWorld(world);
				numRegisteredComponentsThisRun++;
			}
			numTotalRegisteredComponents++;
		}
		if (components.size() == numTotalRegisteredComponents)
		{
			postRegisterAllComponents();
			return true;
		}
		return false;
	}

	void AActor::postRegisterAllComponents()
	{

	}

	void AActor::postActorConstruction()
	{
		World* const world = getWorld();
		bool const bActorsInitialized = world && world->areActorsInitialized();
		if (bActorsInitialized)
		{
			preInitializeComponents();
		}

		if (bActorsInitialized)
		{
			initializeComponents();
			if (world)
			{
			}

			if (!isPendingKill())
			{
				postInitializeComponents();
				if (!isPendingKill())
				{
					bool bRunBeginPlay = mBeginPlayCallDepth > 0 || world->hasBegunPlay();
					if (bRunBeginPlay)
					{
						if (AActor* parentActor = getParentActor())
						{
							bRunBeginPlay = (parentActor->hasActorBegunPlay() || parentActor->isActorBeginningPlay());
						}
					}
					if (bRunBeginPlay)
					{
						dispatchBeginPlay();
					}
				}
			}
		}
		else
		{
			markPendingKill();
			modify(false);
			clearPendingKill();
		}

		if (!isPendingKill())
		{

		}
	}

	void AActor::beginPlay()
	{
		BOOST_ASSERT(mActorHasBegunPlay == EActorBeginPlayState::HasNotBegunPlay);
		registerAllActorTickFunctions(true, false);




		TInlineComponentArray<ActorComponent*> components;
		getComponents(components);

		mActorHasBegunPlay = EActorBeginPlayState::BeginningPlay;
		for (ActorComponent* component : components)
		{
			if (component->isRegistered() && !component->hasBegunPlay())
			{
				component->registerAllComponentTickFunctions(true);
				component->beginPlay();
			}
			else
			{

			}
		}
		receiveBeginPlay();
		mActorHasBegunPlay = EActorBeginPlayState::HasBegunPlay;
	}

	void AActor::initializeComponents()
	{
		TInlineComponentArray<ActorComponent*> components;
		getComponents(components);
		for (ActorComponent* actorComp : components)
		{
			if (actorComp->isRegistered())
			{
				if (actorComp->bAutoActivate && !actorComp->isActive())
				{
					actorComp->activate(true);
				}
				if (actorComp->bWantsInitializeComponent && !actorComp->hasBeenInitialized())
				{
					actorComp->initializeComponent();
				}
			}
		}
	}

	bool AActor::modify(bool bAlwaysMrakDirty /* = true */)
	{
		return false;
	}

	std::shared_ptr<ChildActorComponent> AActor::getParentComponent() const
	{
		return mParentComponent.lock();
	}

	AActor* AActor::getParentActor() const
	{
		AActor* parentActor = nullptr;
		if (auto parentComponentPtr = getParentComponent())
		{
			parentActor = parentComponentPtr->getOwner();
		}
		return parentActor;
	}

	void AActor::preInitializeComponents()
	{
		if (mAutoReceiveInput != EAutoReceiveInput::Disabled)
		{
			const int32 playerIndex = int32(mAutoReceiveInput.getValue()) - 1;
			APlayerController* pc = GameplayStatics::getPlayerController(this, playerIndex);
			if (pc)
			{
				enableInput(pc);
			}
			else
			{
				getWorld()->mPersistentLevel->registerActorForAutoReceiveInput(this, playerIndex);
			}
		}
	}

	void AActor::enableInput(class APlayerController* playerController)
	{
		if (playerController)
		{
			if (!mInputComponent)
			{
				mInputComponent = newObject<InputComponent>(this);
				mInputComponent->registerComponent();
				mInputComponent->bBlockInput = bBlockInput;
				mInputComponent->mPriority = mInputPriority;


			}
			else
			{
				playerController->popInputComponent(mInputComponent);
			}

			playerController->pushInputComponent(mInputComponent);
		}
	}

	ENetMode AActor::getNetMode() const
	{
		return NM_Standalone;
	}

	void AActor::postInitializeComponents()
	{
		if (!isPendingKill())
		{
			bActorInitialized = true;
			updateAllReplicatedComponents();
		}
	}

	void AActor::updateAllReplicatedComponents()
	{
		mReplicatedComponents.reset();
		for (ActorComponent* component : mOwnedComponents)
		{
			if (component != nullptr)
			{
				updateReplicatedComponent(component);
			}
		}
	}

	class ActorThreadContext : public TThreadSingleton<ActorThreadContext>
	{
		friend TThreadSingleton<ActorThreadContext>;
		ActorThreadContext()
			:mTestRegisterTickFunctions(nullptr)
		{}
	public:
		AActor* mTestRegisterTickFunctions;
	};

	void AActor::registerAllActorTickFunctions(bool bRegister, bool bDoComponents)
	{
		if (!isTemplate())
		{
			if (bTickFunctionsRegistered != bRegister)
			{
				ActorThreadContext & threadContext = ActorThreadContext::get();
				BOOST_ASSERT(threadContext.mTestRegisterTickFunctions == nullptr);
				registerActorTickFunctions(bRegister);
				bTickFunctionsRegistered = bRegister;
				BOOST_ASSERT(threadContext.mTestRegisterTickFunctions == this);
				threadContext.mTestRegisterTickFunctions = nullptr;
			}
			if (bDoComponents)
			{
				for (ActorComponent* component : getComponents())
				{
					if (component)
					{
						component->registerAllComponentTickFunctions(bRegister);
					}
				}
			}
		}
	}

	void AActor::updateReplicatedComponent(ActorComponent* component)
	{
		BOOST_ASSERT(component->getOwner() == this);
		if (component->getIsReplicated())
		{
			mReplicatedComponents.add(component);
		}
		else
		{
			mReplicatedComponents.remove(component);
		}
	}

	void AActor::postActorCreated()
	{

	}


	static void markOwnerRelevantComponentsDirty(AActor* theActor)
	{
		TInlineComponentArray<PrimitiveComponent*> components;
		theActor->getComponents(components);
		for (int32 i = 0; i < components.size(); i++)
		{
			PrimitiveComponent* primitive = components[i];
			if (primitive->isRegistered() && (primitive->bOnlyOwnerSee || primitive->bOwnerNoSee))
			{
				primitive->markRenderStateDirty();
			}
		}

		for (int32 i = 0; i < theActor->mChildren.size(); i++)
		{
			AActor* child = theActor->mChildren[i];
			if (child != nullptr && !child->isPendingKill())
			{
				markOwnerRelevantComponentsDirty(child);
			}
		}
	}

	

	void AActor::setOwner(AActor* newOwner)
	{
		if (mOwner != newOwner && !isPendingKill())
		{
			if (newOwner != nullptr && newOwner->isOwnedBy(this))
			{
				return;
			}

			AActor* oldOwner = mOwner;
			if (mOwner != nullptr)
			{
				BOOST_ASSERT(mOwner->mChildren.remove(this) == 1);
			}
			mOwner = newOwner;
			if (mOwner != nullptr)
			{
				BOOST_ASSERT(!mOwner->mChildren.contains(this));
				mOwner->mChildren.add(this);
			}
			markOwnerRelevantComponentsDirty(this);
		}
	}


	bool AActor::setRootComponent(class SceneComponent* newRootComponent)
	{
		if (newRootComponent == nullptr || newRootComponent->getOwner() == this)
		{
			mRootComponent = newRootComponent;
			return true;
		}
		return false;
	}

	void AActor::registerActorTickFunctions(bool bRegister)
	{
		BOOST_ASSERT(!isTemplate());
		if (bRegister)
		{
			if (mPrimaryActorTick.bCanEverTick)
			{
				mPrimaryActorTick.mTarget = this;
				mPrimaryActorTick.setTickFunctionEnable(mPrimaryActorTick.bStartWithTickEnabled || mPrimaryActorTick.isTickFunctionEnabled());
				mPrimaryActorTick.registerTickFunction(getLevel());
			}
		}
		else
		{
			if (mPrimaryActorTick.isTickFunctionRegistered())
			{
				mPrimaryActorTick.unRegisterTickFunction();
			}
		}
		ActorThreadContext::get().mTestRegisterTickFunctions = this;
	}

	bool AActor::ownsComponent(ActorComponent* component) const
	{
		return mOwnedComponents.contains(component);
	}

	void AActor::tickActor(float deltaTime, enum ELevelTick tickType, ActorTickFunction& thisTickFunction)
	{
		const bool bShouldTick = ((tickType != LEVELTICK_ViewportsOnly) || shouldTickIfViewportsOnly());
		if (bShouldTick)
		{
			if (!isPendingKill() && getWorld())
			{
				tick(deltaTime);
			}
		}
	}

	void AActor::tick(float deltaSeconds)
	{
		
	}

	bool AActor::shouldTickIfViewportsOnly() const
	{
		return false;
	}

	void AActor::receiveBeginPlay()
	{

	}

	bool AActor::isChildActor() const
	{
		return !mParentComponent.expired();
	}
	wstring ActorTickFunction::diagnosticMessage()
	{
		return TEXT("ActorTickFunction");
	}

	void ActorTickFunction::executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
	{
		if (mTarget && !mTarget->isPendingKill())
		{
			mTarget->tickActor(deltaTime, tickType, *this);
		}
	}

	void AActor::becomeViewTarget(class APlayerController* pc)
	{

	}

	void AActor::endViewTarget(class APlayerController* pc)
	{

	}

	void AActor::setActorTickEnabled(bool bEnabled)
	{
		if (!isTemplate() && mPrimaryActorTick.bCanEverTick)
		{
			mPrimaryActorTick.setTickFunctionEnable(bEnabled);
		}
	}

	void AActor::routeEndPlay(const EEndPlayReason::Type endPlayReason)
	{
		if (bActorInitialized)
		{
			World* world = getWorld();
			if (world && world->hasBegunPlay())
			{
				endPlay(endPlayReason);
			}
		}
		unInitializeComponents();
	}
	void AActor::destroyed()
	{
		routeEndPlay(EEndPlayReason::Destroyed);
		World* actorWorld = getWorld();
		if (actorWorld)
		{
		}
	}

	void AActor::endPlay(const EEndPlayReason::Type endPlayReason)
	{
		if (mActorHasBegunPlay == EActorBeginPlayState::HasBegunPlay)
		{
			mActorHasBegunPlay = EActorBeginPlayState::HasNotBegunPlay;

			TInlineComponentArray<ActorComponent*> components;
			getComponents(components);
			for (ActorComponent* component : components)
			{
				if (component->hasBegunPlay())
				{
					component->endPlay(endPlayReason);
				}
			}
		}
		if (endPlayReason == EEndPlayReason::RemovedFromWorld)
		{
			bActorInitialized = false;
		}

		
	}

	void AActor::unInitializeComponents()
	{
		TInlineComponentArray<ActorComponent*> components;
		getComponents(components);
		for (ActorComponent* actorComp : components)
		{
			if (actorComp->hasBeenInitialized())
			{
				actorComp->unInitializeComponent();
			}
		}
	}

	void AActor::getAttachedActors(TArray<AActor*>& outActors) const
	{
		outActors.reset();
		if (mRootComponent != nullptr)
		{
			TInlineComponentArray<SceneComponent*> compsToCheck;
			TInlineComponentArray<SceneComponent*> checkedComps;

			compsToCheck.push(mRootComponent);
			while (compsToCheck.size() > 0)
			{
				const bool bAllowShrinking = false;
				SceneComponent* sceneComp = compsToCheck.pop(bAllowShrinking);
				if (!checkedComps.contains(sceneComp))
				{
					checkedComps.add(sceneComp);
					AActor* compOwner = sceneComp->getOwner();

					if (compOwner != nullptr)
					{
						if (compOwner != this)
						{
							outActors.addUnique(compOwner);
						}
						else
						{
							for (SceneComponent* childComp : sceneComp->getAttachChildren())
							{
								if ((childComp != nullptr) && !checkedComps.contains(childComp))
								{
									compsToCheck.push(childComp);
								}
							}
						}
					}
				}
			}
		}
	}

	void AActor::detachAllSceneComponents(class SceneComponent* inParentComponent, const DetachmentTransformRules& detachmentRules)
	{
		if (inParentComponent != nullptr)
		{
			TInlineComponentArray<SceneComponent*> components;
			getComponents(components);
			for (int32 index = 0; index < components.size(); ++index)
			{
				SceneComponent* sceneComp = components[index];
				if (sceneComp->getAttachParent() == inParentComponent)
				{
					sceneComp->detachFromComponent(detachmentRules);
				}
			}
		}
	}

	void AActor::detachRootComponentFromParent(bool bMaintainWorldPosition /* = true */)
	{
		if (mRootComponent)
		{
			mRootComponent->detachFromParent(bMaintainWorldPosition);
			mAttachmentReplication = RepAttachment();
		}

	}

	ActorComponent* AActor::findComponentByClass(const TSubclassOf<ActorComponent> componentClass) const
	{
		ActorComponent* foundComponent = nullptr;
		for (ActorComponent* component : mOwnedComponents)
		{
			if (component && component->isA(componentClass))
			{
				foundComponent = component;
				break;
			}
		}
		return foundComponent;
	}

	void AActor::unregisterAllComponents(bool bForReregister /* = false */)
	{
		TInlineComponentArray<ActorComponent*> components;
		getComponents(components);
		for (ActorComponent* component : components)
		{
			if (component->isRegistered() && (!bForReregister || component->allowReregistration()))
			{
				component->unregisterComponent();
			}
		}

		postUnregisterAllComponents();
	}

	void AActor::markComponentsAsPendingKill()
	{
		TInlineComponentArray<ActorComponent*> components(this);
		for (ActorComponent* component : components)
		{
			component->onComponentDestroyed(true);
			component->markPendingKill();
		}
	}

	void AActor::dispatchBeginPlay()
	{
		World* world = (!hasActorBegunPlay() && !isPendingKill() ? getWorld() : nullptr);
		if (world)
		{
			const uint32 currentCallDepth = mBeginPlayCallDepth++;
			beginPlay();
			BOOST_ASSERT(mBeginPlayCallDepth - 1 == currentCallDepth);
			mBeginPlayCallDepth = currentCallDepth;
		}
	}

	void AActor::getActorEyesViewPoint(float3& outLocation, Rotator& outRotation) const
	{
		outLocation = getActorLocation();
		outRotation = getActorRotation();
	}

	bool AActor::hasActiveCameraComponent()
	{
		if (bFindCameraComponentWhenViewTarget)
		{
		}
		return false;
	}

	bool AActor::hasActivePawnControlCameraComponent() const
	{
		if (bFindCameraComponentWhenViewTarget)
		{

		}
		return false;
	}

	void AActor::calcCamera(float delta, MinimalViewInfo& outResult)
	{
		if (bFindCameraComponentWhenViewTarget)
		{
		}
		getActorEyesViewPoint(outResult.mLocation, outResult.mRotation);
	}

	bool AActor::setActorLocationAndRotation(float3 newLocation, Rotator newRotation, bool bSweep /* = false */, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (mRootComponent)
		{
			const float3 delta = newLocation - getActorLocation();
			return mRootComponent->moveComponent(delta, newRotation, bSweep);
		}
		return false;
	}


	DECLARE_SIMPLER_REFLECTION(AActor)

}