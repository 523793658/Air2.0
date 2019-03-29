#pragma once
#include "Object.h"
#include "EngineMininal.h"
#include "ObjectGlobals.h"
#include "Containers/Set.h"
#include "Template/PointerIsConvertibleFromTo.h"
#include "Containers/EnumAsByte.h"
#include "Classes/Engine/Level.h"
#include "Classes/Engine/EngineType.h"
#include "Classes/Components/ChildActorComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "Classes/Camera/CameraTypes.h"
#include "Templates/SubclassOf.h"
namespace Air
{
	class World;
	class WorldSettings;
	class Level;
	class SceneComponent;
	class ActorComponent;
	class ChildActorComponent;

	class ENGINE_API AActor : public Object
	{
		GENERATED_RCLASS_BODY(AActor, Object)
	public:

		virtual World* getWorld() const override;

		WorldSettings* getWorldSettings() const;

		Level* getLevel() const { return dynamic_cast<Level*>(getOuter()); }

		bool destroy(bool bNetForce = false, bool bShouldModifyLevel = true);

		void addOwnedComponent(ActorComponent* component);

		template<typename ComponentType>
		ComponentType* addComponent()
		{
			ComponentType* cmp = newObject<ComponentType>(this);
			addOwnedComponent(cmp);
			return cmp;
		}

		FORCEINLINE bool canEverTick() const { return mPrimaryActorTick.bCanEverTick; }


		FORCEINLINE SceneComponent* getRootComponent() const { return mRootComponent; }

		void postSpawnInitialize(Transform const& userSpawnTransform, AActor* inOwner);

		template<class T, class AllocatorType>
		void getComponents(TArray<T*, AllocatorType>& outComponents, bool bIncludeFromChildActors = false) const
		{
			static_assert(TPointerIsConvertibleFromTo<T, const ActorComponent>::Value, "'T' template parameter to getComponents must be derived from ActorComponent");
			outComponents.reset(mOwnedComponents.size());
			TArray<ChildActorComponent*> childActorComponents;
			for (ActorComponent* ownedComponent : mOwnedComponents)
			{
				if (T* component = dynamic_cast<T*>(ownedComponent))
				{
					outComponents.add(component);
				}
				else if (bIncludeFromChildActors)
				{
					if (ChildActorComponent* childActorComponent = dynamic_cast<ChildActorComponent*>(ownedComponent))
					{
						childActorComponents.add(childActorComponent);
					}
				}
			}

			if (bIncludeFromChildActors)
			{
				TArray<T*, AllocatorType> componentsInChildActor;
				for (ChildActorComponent* childActorComponent : childActorComponents)
				{
					if (AActor* childActor = childActorComponent->getChildActor())
					{
						childActor->getComponents(componentsInChildActor, true);
						outComponents.append(std::move(componentsInChildActor));
					}
				}
			}
		}

		template<class AllocatorType>
		void getComponents(TArray<ActorComponent*, AllocatorType>& outComponents, bool bIncludeFromChildActors = false) const
		{
			outComponents.reset(mOwnedComponents.size());
			TArray<ChildActorComponent*> childActorComponents;
			for (ActorComponent* component : mOwnedComponents)
			{
				if (component)
				{
					outComponents.add(component);
				}
				else if (bIncludeFromChildActors)
				{
					if (ChildActorComponent* childActorComponent = dynamic_cast<ChildActorComponent*>(component))
					{
						childActorComponents.add(childActorComponent);
					}
				}
			}
			if (bIncludeFromChildActors)
			{
				TArray<ActorComponent*, AllocatorType> componentsInChildActor;
				for (ChildActorComponent* childActorComponent : childActorComponents)
				{
					if (AActor* childActor = childActorComponent->getChildActor())
					{
						childActor->getComponents(componentsInChildActor, true);
						outComponents.append(std::move(componentsInChildActor));
					}
				}
			}
		}

		const TSet<ActorComponent*>& getComponents() const
		{
			return mOwnedComponents;
		}

		bool incrementalRegisterComponents(int32 numComponentsToRegister);

		bool setRootComponent(class SceneComponent* newRootComponent);

		ENetMode getNetMode() const;
		
		virtual void registerAllComponents();

		virtual void setOwner(AActor* newOwner);

		virtual void postActorCreated();

		void finishSpawning(const Transform& transfrom, bool bIsDefaultTransform = false);

		void registerAllActorTickFunctions(bool bRegister, bool bDoComponents);

		virtual void postRegisterAllComponents();

		virtual void preInitializeComponents();


		//actors初始化自己的组件
		virtual void postInitializeComponents();

		AActor* getParentActor() const;

		bool hasActorBegunPlay() const { return mActorHasBegunPlay == EActorBeginPlayState::HasBegunPlay; }

		bool isActorBeginningPlay() const { return mActorHasBegunPlay == EActorBeginPlayState::BeginningPlay; }

		void dispatchBeginPlay();

		virtual bool modify(bool bAlwaysMrakDirty = true) override;

		void initializeComponents();


		std::shared_ptr<ChildActorComponent> getParentComponent() const;

		void updateAllReplicatedComponents();

		void updateReplicatedComponent(ActorComponent* component);

		inline bool isOwnedBy(const AActor* testOwner) const
		{
			for (const AActor* arg = this; arg; arg = arg->mOwner)
			{
				if (arg == testOwner)
				{
					return true;
				}
			}
			return false;
		}

		virtual bool shouldTickIfViewportsOnly() const;

		virtual void tick(float deltaSeconds);

		virtual void tickActor(float deltaTime, enum ELevelTick tickType, ActorTickFunction& thisTickFunction);

		bool ownsComponent(ActorComponent* component) const;
	
		bool isActorInitialized() const { return bActorInitialized; }
		FORCEINLINE Transform actorToWorld() const
		{
			if (mRootComponent != nullptr)
			{
				return mRootComponent->mComponentToWorld;
			}
			return Transform::identity;
		}

		Transform getTransform() const
		{
			return actorToWorld();

		}

		FORCEINLINE float3 getActorLocation()const
		{
			return templateGetActorLocation(mRootComponent);
		}
		FORCEINLINE Rotator getActorRotation() const
		{
			return templateGetActorRotation(mRootComponent);
		}

		virtual void enableInput(class APlayerController* playerController);

		virtual void endViewTarget(class APlayerController* pc);

		virtual void becomeViewTarget(class APlayerController* pc);
		void setActorTickEnabled(bool bEnabled);

		FORCEINLINE_DEBUGGABLE bool hasAuthority() const
		{
			return mRole == ROLE_Authority;
		}

		void routeEndPlay(const EEndPlayReason::Type endPlayReason);

		virtual void endPlay(const EEndPlayReason::Type endPlayReason);

		void unInitializeComponents();

		virtual void destroyed();

		void getAttachedActors(TArray<AActor*>& outActors) const;

		void detachAllSceneComponents(class SceneComponent* inParentComponent, const DetachmentTransformRules& detachmentRules);

		void detachRootComponentFromParent(bool bMaintainWorldPosition = true);

		FORCEINLINE_DEBUGGABLE AActor* getOwner() const
		{
			return mOwner;
		}

		virtual void unregisterAllComponents(bool bForReregister = false);

		virtual void markComponentsAsPendingKill();

		virtual void postUnregisterAllComponents() {}

		virtual ActorComponent* findComponentByClass(const TSubclassOf<ActorComponent> componentClass) const;

		template<class T>
		T* findComponentByClass() const
		{
			static_assert(std::is_base_of<ActorComponent, T>::value, "T is not derived from ActorComponent.");
			return (T*)findComponentByClass(T::StaticClass());
		}

		bool isChildActor() const;

		virtual void getActorEyesViewPoint(float3& outLocation, Rotator& outRotation) const;

		virtual bool hasActiveCameraComponent();

		virtual bool hasActivePawnControlCameraComponent() const;
	
		virtual bool setActorRotation(Rotator newRotation, ETeleportType teleport = ETeleportType::None);

		bool setActorLocationAndRotation(float3 newLocation, Rotator newRotation, bool bSweep = false, ETeleportType teleport = ETeleportType::None);

		virtual void calcCamera(float delta, MinimalViewInfo& outResult);
	protected:
		virtual void beginPlay();

		virtual void registerActorTickFunctions(bool bRegister);


		void receiveBeginPlay();

		

	private:
		void postActorConstruction();

		template<class T>
		static FORCEINLINE float3 templateGetActorLocation(const T* rootComponent)
		{
			return (rootComponent != nullptr) ? rootComponent->getComponentLocation() : float3::Zero;
		}

		template<class T>
		static FORCEINLINE Rotator templateGetActorRotation(const T* rootComponent)
		{
			return (rootComponent != nullptr) ? rootComponent->getComponentRotation() : Rotator::ZeroRotator;
		}

		static uint32 mBeginPlayCallDepth;

		enum class EActorBeginPlayState : uint8
		{
			HasNotBegunPlay,
			BeginningPlay,
			HasBegunPlay,
		};
	private:
		void initializeDefaults();

	private:

		std::weak_ptr<ChildActorComponent> mParentComponent;

		EActorBeginPlayState mActorHasBegunPlay : 2;
	private:
		TSet<ActorComponent*> mOwnedComponents;

		TSet<ActorComponent*> mReplicatedComponents;

		uint8 bhasFinishedSpawning : 1;

		uint8 bHasDeferredComponentRegistration : 1;

		uint8 bActorInitialized : 1;

		uint8 bTickFunctionsRegistered : 1;

		uint8 bActorIsBeingDestroyed : 1;

		friend struct MarkActorIsBeingDestroyed;

		struct RepAttachment mAttachmentReplication;

		AActor* mOwner;
	protected:
		SceneComponent* mRootComponent;

		class InputComponent* mInputComponent;
		float mCreationTime;

	public:
		Level* mLevel;
		TEnumAsByte<enum ENetRole> mRole;

		TEnumAsByte<EAutoReceiveInput::Type> mAutoReceiveInput;

		struct ActorTickFunction	mPrimaryActorTick;

		uint8 bAllowTickBeforeBeginPlay : 1;

		uint8 bFindCameraComponentWhenViewTarget : 1;

		uint8 bHidden : 1;

		uint8 bBlockInput : 1;

		int32 mInputPriority;

		TArray<AActor*> mChildren;
	};

	template<typename ExecuteTickLambda>
	void ActorComponentTickFunction::ExecuteTickHelper(ActorComponent* target, bool bTickInEditor, float deltaTime, ELevelTick tickType, const ExecuteTickLambda& executeTickFunc)
	{
		if (target && !target->isPendingKill())
		{
			if (target->bRegistered)
			{
				AActor* myOwner = target->getOwner();
				if (tickType != LEVELTICK_ViewportsOnly || (myOwner && myOwner->shouldTickIfViewportsOnly()))
				{
					const float timeDilation = 1.0f;
					executeTickFunc(deltaTime * timeDilation);
				}
			}
		}
	}

	template<class T, uint32 numElements = NumInlinedActorComponents>
	class TInlineComponentArray : public TArray<T, TInlineAllocator<numElements>>
	{
		typedef TArray<T, TInlineAllocator<numElements>> Supper;
	public:
		TInlineComponentArray() :Supper() {}

		TInlineComponentArray(const class AActor* actor) : Supper()
		{
			if (actor)
			{
				actor->getComponents(*this);
			}
		}
	};

	struct MarkActorIsBeingDestroyed
	{
	private:
		MarkActorIsBeingDestroyed(AActor* inActor)
		{
			inActor->bActorIsBeingDestroyed = true;
		}
		friend World;
	};

	FORCEINLINE_DEBUGGABLE AActor* ActorComponent::getOwner() const
	{
		if (mOwnerPrivate == nullptr)
		{
			return getTypedOuter<AActor>();
		}
		return mOwnerPrivate;
	}

}