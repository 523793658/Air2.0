#pragma once 
#include "EngineMininal.h"
#include "Object.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "Classes/Engine/EngineType.h"
namespace Air
{
	class AActor;
	enum class EUpdateTransformFlags : int32
	{
		None = 0x0,
		SkipPhysicsUpdate = 0x1,
		PropagateFromParent = 0x2,
	};


	enum class EComponentCreationMethod : uint8
	{
		Native,
		SimpleConstructionScript,
		UserConstructionScript,
		Instance,
	};

	
	class ENGINE_API ActorComponent : public Object
	{
		GENERATED_RCLASS_BODY(ActorComponent, Object)
	public:
		
		virtual void postInitProperties() override;

		class AActor* getOwner() const;

		virtual World* getWorld() const override final { return (mWorldPrivate ? mWorldPrivate : getWorld_Uncached()); }

		bool hasBeenCreated()const { return bHasBeenCreated; }

		virtual void onComponentDestroyed(bool bDestroyingHierarchy);

		virtual void onComponentCreated();

		FORCEINLINE bool isRegistered() const { return bRegistered; }

		bool isRenderStateCreated() const
		{
			return bRenderStateCreated;
		}

		void registerComponentWithWorld(World* inWorld);

		bool hasBegunPlay() const { return bHasBegunPlay; }

		class SceneInterface* getScene() const;

		void registerAllComponentTickFunctions(bool bRegister);

		void markRenderStateDirty();

		void markForNeededEndOfFrameRecreate();

		virtual void beginPlay();

		bool isActive() const;

		virtual void activate(bool bReset = false);

		bool hasBeenInitialized() const { return bHasBeenInitialized; }

		virtual void initializeComponent();

		virtual void unInitializeComponent();

		FORCEINLINE bool getIsReplicated() const
		{
			return bReplicates;
		}

		bool isCreateByConstructionScript() const;

		virtual void setComponentTickEnabled(bool bEnable);

		virtual void updateComponentToWorld(EUpdateTransformFlags updateTransformFlags = EUpdateTransformFlags::None, ETeleportType teleport = ETeleportType::None) {}

		void receiveBeginPlay();

		bool setupActorComponentTickFunction(struct TickFunction* tickFunction);

		virtual bool requiresGameThreadEndOfFrameRecreate() const;

		void doDeferredRenderUpdates_Concurrent();

		void recreateRenderState_Concurrent();

		void destroyRenderState_Concurrent();

		void registerComponent();

		void unregisterComponent();

		virtual void tickComponent(float deltaTime, enum ELevelTick tickType, ActorComponentTickFunction* thisTickFunction);

		uint32 getMarkedForEndOfFrameUpdateState() const { return mMarkedForEndOfFrameUpdateState; }

		virtual void endPlay(const EEndPlayReason::Type endPlayReason);

		FORCEINLINE bool allowReregistration() const {
			return bAllowReregistration;
		}

	protected:
		virtual bool shouldActivate() const;
		virtual void onRegister();
		virtual void onUnregister();
		virtual bool shouldCreateRenderState() const
		{
			return false;
		}
		virtual void createRenderState_Concurrent();

		virtual void registerComponentTickFunctions(bool bRegister);

		virtual void sendRenderTransform_Concurrent();

		virtual void sendRenderDynammicData_Concurrent();


	private:
		void executeRegisterEvents();
		void executeUnregisterEvents();
		World* getWorld_Uncached()const;

	public:
		EComponentCreationMethod mCreationMethod;

		struct ActorComponentTickFunction mPrimaryComponentTick;
	private:
		mutable AActor* mOwnerPrivate;

		World* mWorldPrivate{ nullptr };

		bool bHasBeenCreated{ false };

		uint32 mMarkedForEndOfFrameUpdateState : 2;

		uint32 bHasBegunPlay : 1;

		uint32 bHasBeenInitialized : 1;

		uint32 bIsActive : 1;

		uint32 bReplicates : 1;

		uint32 bRenderStateDirty : 1;

		uint32 bRenderTransformDirty : 1;

		uint32 bRenderDynamicDataDirty : 1;

		friend struct MarkComponentEndOfFrameUpdateState;
		friend struct ActorComponentTickFunction;
	protected:
		uint32 bRegistered : 1;

		uint32 bRenderStateCreated : 1;

		uint32 bAllowReregistration : 1;
	public:
		uint32 bAutoRegister : 1;
		uint32 bAutoActivate : 1;
		uint32 bWantsInitializeComponent : 1;
		uint32 bTickFunctionsRegistered : 1;
		uint32 bNeverNeedsRenderUpdate : 1;
	};
}