#include "ActorComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "SimpleReflection.h"
#include "Classes/Engine/World.h"
#include "Misc/App.h"
#include "Classes/Components/PrimitiveComponent.h"
namespace Air
{

	static ActorComponent* GTestRegisterComponentTickFunctions = nullptr;

	ActorComponent::ActorComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
	{
		mOwnerPrivate = getTypedOuter<AActor>();
		mPrimaryComponentTick.mTickGroup = TG_DuringPhysics;
		mPrimaryComponentTick.bStartWithTickEnabled = true;
		mPrimaryComponentTick.bCanEverTick = false;

		bAutoRegister = true;
	}

	void ActorComponent::initializeComponent()
	{
		BOOST_ASSERT(bRegistered);
		BOOST_ASSERT(!bHasBeenInitialized);
		bHasBeenInitialized = true;
	}

	void ActorComponent::unInitializeComponent()
	{
		if (!isRegistered())
		{
			return;
		}

		BOOST_ASSERT(mWorldPrivate != nullptr);

		const PrimitiveComponent* primitive = dynamic_cast<const PrimitiveComponent*>(this);
		if (primitive)
		{

		}
		registerAllComponentTickFunctions(false);
		executeUnregisterEvents();
		mWorldPrivate = nullptr;
	}

	bool ActorComponent::shouldActivate() const
	{
		return !bIsActive;
	}

	void ActorComponent::activate(bool bReset /* = false */)
	{
		if (bReset || shouldActivate() == true)
		{
			setComponentTickEnabled(true);
			bIsActive = true;
			
		}
	}


	void ActorComponent::postInitProperties()
	{
		Object::postInitProperties();
		if (mOwnerPrivate)
		{
			mOwnerPrivate->addOwnedComponent(this);
		}
	}

	void ActorComponent::onComponentCreated()
	{
		BOOST_ASSERT(!bHasBeenCreated);
		bHasBeenCreated = true;
	}

	void ActorComponent::onComponentDestroyed(bool bDestroyingHierarchy)
	{
		bHasBeenCreated = false;
	}

	bool ActorComponent::isCreateByConstructionScript() const
	{
		return ((mCreationMethod == EComponentCreationMethod::SimpleConstructionScript) || (mCreationMethod == EComponentCreationMethod::UserConstructionScript));
	}

	void ActorComponent::registerComponentWithWorld(World* inWorld)
	{
		if (isPendingKill())
		{
			return;
		}

		if (isRegistered())
		{
			return;
		}

		if (inWorld == nullptr)
		{
			return;
		}

		BOOST_ASSERT(mWorldPrivate == nullptr);
		AActor* myOwner = getOwner();

		BOOST_ASSERT(myOwner == nullptr || myOwner->ownsComponent(this));

		if (!bHasBeenCreated)
		{
			onComponentCreated();
		}

		mWorldPrivate = inWorld;

		executeRegisterEvents();

		if (!inWorld->isGameWorld())
		{
			registerAllComponentTickFunctions(true);
		}
		else if (myOwner == nullptr)
		{
			if (!bHasBeenInitialized && bWantsInitializeComponent)
			{
				initializeComponent();
			}
		}
		else
		{
			if (!bHasBeenInitialized && bWantsInitializeComponent && myOwner->isActorInitialized())
			{
				initializeComponent();
			}
			if (myOwner->hasActorBegunPlay() || myOwner->isActorBeginningPlay())
			{
				registerAllComponentTickFunctions(true);
				if (!bHasBegunPlay)
				{
					beginPlay();
				}
			}
		}
		if (isCreateByConstructionScript())
		{
			
		}
	}

	void ActorComponent::setComponentTickEnabled(bool bEnable)
	{
		if (mPrimaryComponentTick.bCanEverTick && !isTemplate())
		{
			mPrimaryComponentTick.setTickFunctionEnable(bEnable);
		}
	}

	void ActorComponent::executeRegisterEvents()
	{
		if (!bRegistered)
		{
			onRegister();
			BOOST_ASSERT(bRegistered);
		}
		if (App::canEverRender() && !bRenderStateCreated && mWorldPrivate->mScene && shouldCreateRenderState())
		{
			createRenderState_Concurrent();
		}
	}

	void ActorComponent::executeUnregisterEvents()
	{
		if (bRenderStateCreated)
		{
			BOOST_ASSERT(bRegistered);
			destroyRenderState_Concurrent();
			BOOST_ASSERT(!bRenderStateCreated);
		}
		if (bRegistered)
		{
			onUnregister();
			BOOST_ASSERT(!bRegistered);
		}
	}

	void ActorComponent::onUnregister()
	{
		BOOST_ASSERT(bRegistered);
		bRegistered = false;
	}

	void ActorComponent::onRegister()
	{
		BOOST_ASSERT(!getOuter()->isTemplate());
		BOOST_ASSERT(!isTemplate());
		BOOST_ASSERT(mWorldPrivate);
		BOOST_ASSERT(!bRegistered);
		BOOST_ASSERT(!isPendingKill());
		bRegistered = true;
		updateComponentToWorld();
		if (bAutoActivate)
		{
			AActor* owner = getOwner();
			if (!mWorldPrivate->isGameWorld() || owner == nullptr || owner->isActorInitialized())
			{
				activate(true);
			}
		}
	}

	bool ActorComponent::isActive() const
	{
		return bIsActive;
	}

	void ActorComponent::beginPlay()
	{
		BOOST_ASSERT(bRegistered);
		BOOST_ASSERT(!bHasBegunPlay);
		BOOST_ASSERT(bTickFunctionsRegistered);
		receiveBeginPlay();
		bHasBegunPlay = true;
	}

	void ActorComponent::endPlay(const EEndPlayReason::Type endPlayReason)
	{
		BOOST_ASSERT(bHasBegunPlay);
		if (!hasAnyFlags(RF_BeginDestroyed))
		{
			
		}
		bHasBegunPlay = false;
	}

	void ActorComponent::registerAllComponentTickFunctions(bool bRegister)
	{
		BOOST_ASSERT(GTestRegisterComponentTickFunctions == nullptr);
		if (bRegistered)
		{
			if (bTickFunctionsRegistered != bRegister)
			{
				registerComponentTickFunctions(bRegister);
				bTickFunctionsRegistered = bRegister;
				BOOST_ASSERT(GTestRegisterComponentTickFunctions == this);
				GTestRegisterComponentTickFunctions = nullptr;
			}
		}
	}

	bool ActorComponent::setupActorComponentTickFunction(struct TickFunction* tickFunction)
	{
		if (tickFunction->bCanEverTick && !isTemplate())
		{
			AActor* myOwner = getOwner();
			if (!myOwner || !myOwner->isTemplate())
			{
				Level* componentLevel = (myOwner ? myOwner->getLevel() : getWorld()->mPersistentLevel);
				tickFunction->setTickFunctionEnable(tickFunction->bStartWithTickEnabled || tickFunction->isTickFunctionEnabled());
				tickFunction->registerTickFunction(componentLevel);
				return true;
			}
		}
		return false;
	}

	void ActorComponent::registerComponentTickFunctions(bool bRegister)
	{
		if (bRegister)
		{
			if (setupActorComponentTickFunction(&mPrimaryComponentTick))
			{
				mPrimaryComponentTick.mTarget = this;
			}
		}
		else
		{
			if (mPrimaryComponentTick.isTickFunctionRegistered())
			{
				mPrimaryComponentTick.unRegisterTickFunction();
			}
		}
		GTestRegisterComponentTickFunctions = this;
	}

	void ActorComponent::markRenderStateDirty()
	{
		if (isRegistered() && bRenderStateCreated && (!bRenderStateDirty || !getWorld()))
		{
			bRenderStateDirty = true;
			markForNeededEndOfFrameRecreate();
		}
	}

	void ActorComponent::doDeferredRenderUpdates_Concurrent()
	{
		BOOST_ASSERT(!isTemplate());
		BOOST_ASSERT(!isPendingKill());
		if (!isRegistered())
		{
			return;
		}
		if (bRenderStateDirty)
		{
			recreateRenderState_Concurrent();
			BOOST_ASSERT(!bRenderStateDirty);
		}
		else
		{
			if (bRenderTransformDirty)
			{
				sendRenderTransform_Concurrent();
			}
			if (bRenderDynamicDataDirty)
			{
				sendRenderDynammicData_Concurrent();
			}
		}
	}

	void ActorComponent::sendRenderDynammicData_Concurrent()
	{
		BOOST_ASSERT(bRenderStateCreated);
		bRenderDynamicDataDirty = false;

	}

	void ActorComponent::sendRenderTransform_Concurrent()
	{
		BOOST_ASSERT(bRenderStateCreated);
		bRenderTransformDirty = false;
	}

	void ActorComponent::recreateRenderState_Concurrent()
	{
		if (bRenderStateCreated)
		{
			BOOST_ASSERT(isRegistered());
			destroyRenderState_Concurrent();
			BOOST_ASSERT(!bRenderStateCreated);
		}
		if (isRegistered() && mWorldPrivate->mScene)
		{
			createRenderState_Concurrent();
			BOOST_ASSERT(bRenderStateCreated);
		}
	}

	void ActorComponent::destroyRenderState_Concurrent()
	{
		BOOST_ASSERT(bRenderStateCreated);
		bRenderStateCreated = false;
	}

	bool ActorComponent::requiresGameThreadEndOfFrameRecreate() const
	{
		return true;
	}
	void ActorComponent::markForNeededEndOfFrameRecreate()
	{
		if (bNeverNeedsRenderUpdate)
		{
			return;
		}
		World* componentWorld = getWorld();
		if (componentWorld)
		{
			componentWorld->markActorComponentForNeededEndOfFrameUpate(this, requiresGameThreadEndOfFrameRecreate());
		}
		else
		{
			doDeferredRenderUpdates_Concurrent();
		}
	}

	void ActorComponent::createRenderState_Concurrent()
	{
		BOOST_ASSERT(isRegistered());
		BOOST_ASSERT(mWorldPrivate->mScene);
		BOOST_ASSERT(!bRenderStateCreated);
		bRenderStateCreated = true;
		bRenderStateDirty = false;
		bRenderTransformDirty = false;
		bRenderDynamicDataDirty = false;
	}

	World* ActorComponent::getWorld_Uncached() const
	{
		World* componentWorld = nullptr;
		AActor* myOwner = getOwner();
		if (myOwner && !myOwner->hasAnyFlags(RF_ClassDefaultObject))
		{
			componentWorld = myOwner->getWorld();
		}
		if (componentWorld == nullptr)
		{
			componentWorld = dynamic_cast<World*>(getOuter());
		}
		return componentWorld;
	}

	void ActorComponent::tickComponent(float deltaTime, enum ELevelTick tickType, ActorComponentTickFunction* thisTickFunction)
	{
		BOOST_ASSERT(bRegistered);
		
	}

	void ActorComponent::receiveBeginPlay()
	{

	}
	wstring ActorComponentTickFunction::diagnosticMessage()
	{
		return TEXT("ActorComponentTickFunction");
	}

	void ActorComponentTickFunction::executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
	{
		ExecuteTickHelper(mTarget, false, deltaTime, tickType, [this, tickType](float dilatedTime)
		{
			mTarget->tickComponent(dilatedTime, tickType, this);
		});
	}

	void ActorComponent::registerComponent()
	{
		AActor* myOwner = getOwner();
		World* myOwnerWorld = (myOwner ? myOwner->getWorld() : nullptr);
		BOOST_ASSERT(myOwnerWorld);
		if (myOwnerWorld)
		{
			registerComponentWithWorld(myOwnerWorld);
		}
	}

	void ActorComponent::unregisterComponent()
	{
		if (!isRegistered())
		{
			return;
		}

		BOOST_ASSERT(mWorldPrivate != nullptr);

		const PrimitiveComponent* primitive = dynamic_cast<const PrimitiveComponent*>(this);
		if (primitive)
		{
			
		}
		registerAllComponentTickFunctions(false);
		executeUnregisterEvents();
		mWorldPrivate = nullptr;
	}

	SceneInterface* ActorComponent::getScene() const
	{
		return (mWorldPrivate ? mWorldPrivate->mScene : nullptr);
	}

	DECLARE_SIMPLER_REFLECTION(ActorComponent)
}