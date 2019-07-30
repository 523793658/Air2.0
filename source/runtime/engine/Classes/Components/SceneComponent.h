#pragma once
#include "Classes/Engine/EngineType.h"
#include "Classes/Components/ActorComponent.h"
namespace Air
{
	enum ERelativeTransformSpace
	{
		RTS_World,
		RTS_Actor,
		RTS_Component,
		RTS_ParentBoneSpace,
	};

#define SCENECOMPONENT_ROTATOR_TOLERANCE	(1.e-4f)
#define SCENECOMPONENT_QUAT_TOLERANCE	(1.e-8f)


	class ENGINE_API SceneComponent : public ActorComponent
	{
		GENERATED_RCLASS_BODY(SceneComponent, ActorComponent)
	public:

		FORCEINLINE SceneComponent* getAttachParent() const
		{
			return mAttachParent.get();
		}

		virtual void detachFromComponent(const DetachmentTransformRules& detachmentRules);

		bool attachToComponent(SceneComponent* inParent, const AttachmentTransformRules& attachmentRules, wstring inSocketName = Name_None);

		FORCEINLINE const TArray<SceneComponent*>& getAttachChildren() const
		{
			return mAttachChildren;
		}

		void setWorldRotation(Rotator newRotation, bool bSweep = false, ETeleportType teleport = ETeleportType::None);

		void setRelativeRotation(Rotator newRotation, bool bSweep, ETeleportType teleport);

		void setWorldTransform(const Transform& newTransform, bool bSweep = false);

		void setRelativeTransform(const Transform& newTransform, bool bSweep = false);

		void setRelativeLocationAndRotation(float3 newLocation, Rotator newRotation, bool bSweep = false);

		void setRelativeLocationAndRotation(float3 newLocation, const Quaternion& newRotation, bool bSweep = false);

		virtual void setRelativeScale3D(float3 newScale3D);

		bool isDeferringMovementUpdates() const;

		void conditionalUpdateComponentToWorld();

		virtual Transform getSocketTransform(wstring inSocketName, ERelativeTransformSpace transformSpace = RTS_World) const;
		wstring getAttachSocketName() const;

		bool moveComponent(const float3& delta, const Quaternion& newRotation, bool bSweep);
		bool moveComponent(const float3& delta, const Rotator& newRotation, bool bSweep);

		bool shouldComponentAddToScene() const;

		bool shouldRender() const;

		virtual bool isVisible() const;

		SceneComponent* getAttachmentRoot() const;

		AActor* getAttachmentRootActor() const;

		FORCEINLINE float3 getComponentLocation()const
		{
			return mComponentToWorld.getLocation();
		}

		FORCEINLINE Rotator getComponentRotation() const
		{
			return mWorldRotationCache.normalizedQuatToRotator(mComponentToWorld.getRotation());
		}

		FORCEINLINE Quaternion getComponentQuat() const
		{
			return mComponentToWorld.getRotation();
		}

		FORCEINLINE float3 getComponentScale() const
		{
			return mComponentToWorld.getScale3D();
		}

		virtual BoxSphereBounds calcBounds(const Transform& localToWorld) const;

		virtual bool shouldCreateRenderState() const override
		{
			return true;
		}

		FORCEINLINE bool areDynamicDataChangeAllowed(bool bIgnoreStationary = true) const
		{
			return !(isRegistered() && (mMobility == EComponentMobility::Static || (!bIgnoreStationary &&EComponentMobility::Stationary)));
		}

		void updateBounds();
		virtual void detachFromParent(bool bMaintainWorldPosition = false, bool bCallModify = true);
	protected:
		FORCEINLINE Transform calcNewComponentToWorld(const Transform& newRelativeTransform, const SceneComponent* parent = nullptr, wstring scoketName = TEXT(""))const
		{
			scoketName = parent ? scoketName : getAttachSocketName();
			parent = parent ? parent : getAttachParent();
			if (parent)
			{
				const bool bGeneral = bAbsoluteLocation || bAbsoluteRotation || bAbsoluteScale;
				if (!bGeneral)
				{
					return newRelativeTransform * parent->getSocketTransform(scoketName);
				}
				return calcNewComponentToWorld_GeneralCase(newRelativeTransform, parent, scoketName);
			}
			else
			{
				return newRelativeTransform;
			}
		}

		Transform calcNewComponentToWorld_GeneralCase(const Transform& newRelativeTransform, const SceneComponent* parent, wstring socketName) const;

		virtual void onChildAttached(SceneComponent* childComponent) {}

		virtual void onChildDetached(SceneComponent* childComponent){}

		virtual void onAttachmentChange() {}

		void setupAttachment(SceneComponent* inParent, wstring inSocketName = Name_None);

		bool isAttachedTo(class SceneComponent* testComp) const;

		virtual bool canAttachAsChild(SceneComponent* childComponent, wstring socketName)const { return true; }

		virtual void updateComponentToWorld(EUpdateTransformFlags updateTransformFlags = EUpdateTransformFlags::None, ETeleportType teleport = ETeleportType::None) override
		{
			updateComponentToWorldWithParent(getAttachParent(), getAttachSocketName(), updateTransformFlags, mRelativeRotationCache.rotatorToQuat(mRelativeRotation), teleport);
		}

	protected:
		virtual bool moveComponentImpl(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport = ETeleportType::None);

		bool internalSetWorldLocationAndRotation(float3 newLocation, const Quaternion& newQuat, bool bNoPhysics = false, ETeleportType teleport = ETeleportType::None);

		virtual void onUpdateTransform(EUpdateTransformFlags updateTransformFlags, ETeleportType teleport = ETeleportType::None);

	private:
		void updateComponentToWorldWithParent(SceneComponent* parent, wstring socketName, EUpdateTransformFlags updateTransformFlags, const Quaternion& relativeRotationQuat, ETeleportType teleport = ETeleportType::None);

		void propagateTransformUpdate(bool bTransformChange, EUpdateTransformFlags updateTransformFlags = EUpdateTransformFlags::None, ETeleportType teleport = ETeleportType::None);

	private:  
		std::shared_ptr<SceneComponent> mAttachParent{ nullptr };
		TArray<SceneComponent*> mAttachChildren;

		RotationConversionCache mRelativeRotationCache;

		RotationConversionCache mWorldRotationCache;

		wstring mAttachSocketName;
	public:
		float3	mRelativeLocation;
		Rotator mRelativeRotation;
		float3	mRelativeScale3D;

		Transform mComponentToWorld;

		uint32 bAbsoluteLocation : 1;
		uint32 bAbsoluteRotation : 1;
		uint32 bAbsoluteScale : 1;

		uint32 bHiddenInGame : 1;

		uint32 bVisible : 1;

		uint32 bUseAttachParentBound : 1;

		float3 mComponentVelocity;

		TEnumAsByte<EComponentMobility::Type> mMobility;

	public:
		BoxSphereBounds mBounds;

	protected:
		uint32 bWorldToComponentUpdated : 1;

		uint32 bDisableDetachmentUpdateOverlaps : 1;

		uint32 bWantsOnUpdateTransform : 1;
	};

	FORCEINLINE_DEBUGGABLE bool SceneComponent::isDeferringMovementUpdates() const
	{
		return false;
	}

	FORCEINLINE_DEBUGGABLE bool SceneComponent::moveComponent(const float3& delta, const Rotator& newRotation, bool bSweep)
	{
		if (getAttachParent() == nullptr)
		{
			if (delta.isZero() && newRotation.equals(mRelativeRotation, SCENECOMPONENT_ROTATOR_TOLERANCE))
			{
				return true;
			}
			return moveComponentImpl(delta, mRelativeRotationCache.rotatorToQuat_ReadOnly(newRotation), bSweep);
		}
		return moveComponentImpl(delta, newRotation.quaternion(), bSweep);
	}

	FORCEINLINE_DEBUGGABLE bool SceneComponent::moveComponent(const float3& delta, const Quaternion& newRotation, bool bSweep)
	{
		return moveComponentImpl(delta, newRotation, bSweep);
	}

	FORCEINLINE_DEBUGGABLE wstring SceneComponent::getAttachSocketName() const
	{
		return mAttachSocketName;
	}

	FORCEINLINE_DEBUGGABLE void SceneComponent::conditionalUpdateComponentToWorld()
	{
		if (!bWorldToComponentUpdated)
		{
			updateComponentToWorld();
		}
	}
}