#pragma once
#include "CoreMinimal.h"
#include "Classes/Components/ActorComponent.h"
#include "Classes/Components/SceneComponent.h"
namespace Air
{


	class ENGINE_API MovementComponent : public ActorComponent
	{
		GENERATED_RCLASS_BODY(MovementComponent, ActorComponent)
	public:
		virtual void stopMovementImmediately();

		virtual void updateComponentVelocity();
	
		virtual float getMaxSpeed() const;

		virtual bool isExceedingMaxSpeed(float maxSpeed) const;
		
		virtual bool shouldSkipUpdate(float deltaTime) const;

		bool safeMoveUpdatedComponent(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport = ETeleportType::None);

		bool safeMoveUpdatedComponent(const float3& delta, const Rotator& newRotation, bool bSweep, ETeleportType teleport = ETeleportType::None);

		bool moveUpdatedComponent(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport = ETeleportType::None);

		bool moveUpdatedComponent(const float3& delta, const Rotator& newRotation, bool bSweep, ETeleportType teleport = ETeleportType::None);

		virtual void registerComponentTickFunctions(bool bRegister) override;

		virtual void updateTickRegistration();

		virtual void tickComponent(float deltaTime, enum ELevelTick tickType, ActorComponentTickFunction* thisTickFunction) override;

		virtual void setUpdatedComponent(SceneComponent* newUpdatedComponent);

		virtual void onRegister() override;


	protected:

		virtual bool moveUpdatedComponentImpl(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport = ETeleportType::None);
	public:
		uint32 bTickBeforeOwner : 1;

		uint32 bAutoUpdateTickRegistration : 1;
		uint32 bAutoRegisterUpdatedComponent : 1;
	public:
		float3 mVelocity;

		std::shared_ptr<SceneComponent> mUpdatedComponent;

		std::shared_ptr<class PrimitiveComponent> mUpdatedPrimitive;
	};

	inline void MovementComponent::stopMovementImmediately()
	{
		mVelocity = float3::Zero;
		updateComponentVelocity();
	}

	inline float MovementComponent::getMaxSpeed() const
	{
		return 0.f;
	}

	FORCEINLINE_DEBUGGABLE bool MovementComponent::safeMoveUpdatedComponent(const float3& delta, const Rotator& newRotation, bool bSweep, ETeleportType teleport /* = ETeleportType::None */)
	{
		return safeMoveUpdatedComponent(delta, newRotation.quaternion(), bSweep, teleport);
	}

	FORCEINLINE_DEBUGGABLE bool MovementComponent::moveUpdatedComponent(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport /* = ETeleportType::None */)
	{
		return moveUpdatedComponentImpl(delta, newRotation, bSweep, teleport);
	}

	FORCEINLINE_DEBUGGABLE bool MovementComponent::moveUpdatedComponent(const float3& delta, const Rotator& newRotation, bool bSweep, ETeleportType teleport /* = ETeleportType::None */)
	{
		return moveUpdatedComponentImpl(delta, newRotation.quaternion(), bSweep, teleport);
	}
}