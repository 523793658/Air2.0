#include "Classes/Components/SceneComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Engine/World.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "ObjectThreadContext.h"
#include "SimpleReflection.h"
namespace Air
{
	SceneComponent::SceneComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ActorComponent(objectInitializer)
	{
		bVisible = true;
		bAutoActivate = false;
		bUseAttachParentBound = false;
		mRelativeScale3D = float3(1.0f);
		bWantsOnUpdateTransform = false;
		mMobility = EComponentMobility::Movable;
	}

	Transform SceneComponent::getSocketTransform(wstring inSocketName, ERelativeTransformSpace transformSpace /* = RTS_World */) const
	{
		switch (transformSpace)
		{
		case Air::RTS_Actor:
		{
			return mComponentToWorld.getRelativeTransform(getOwner()->getTransform());
		}

			break;
		case Air::RTS_Component:
		case Air::RTS_ParentBoneSpace:
		{
			return Transform::identity;
		}
		default:
		{
			return mComponentToWorld;
		}
		}
	}

	void SceneComponent::setWorldTransform(const Transform& newTransform, bool bSweep /* = false */)
	{
		if (getAttachParent() != nullptr)
		{
			const Transform parentToWorld = getAttachParent()->getSocketTransform(getAttachSocketName());
			Transform relativeTM = newTransform.getRelativeTransform(parentToWorld);
			if (bAbsoluteLocation)
			{
				relativeTM.copyTranslation(newTransform);
			}
			if (bAbsoluteRotation)
			{
				relativeTM.copyRotation(newTransform);
			}

			if (bAbsoluteScale)
			{
				relativeTM.copyScale(newTransform);
			}
		}
		else
		{
			setRelativeTransform(newTransform, bSweep);
		}
	}

	void SceneComponent::setRelativeTransform(const Transform& newTransform, bool bSweep /* = false */)
	{
		setRelativeLocationAndRotation(newTransform.getTranslation(), newTransform.getRotation(), bSweep);
		setRelativeScale3D(newTransform.getScale3D());
	}

	void SceneComponent::setRelativeLocationAndRotation(float3 newLocation, const Quaternion& newRotation, bool bSweep /* = false */)
	{
		conditionalUpdateComponentToWorld();
		const bool bNaN = false;
		const Transform desiredRelTransform((bNaN ? Quaternion::identity : newRotation), newLocation);
		const Transform desiredWorldTransform = calcNewComponentToWorld(desiredRelTransform);
		const float3 desiredDelta = Transform::subtractTranslations(desiredWorldTransform, mComponentToWorld);
		moveComponent(desiredDelta, desiredWorldTransform.getRotation(), bSweep);
	}

	void SceneComponent::setRelativeLocationAndRotation(float3 newLocation, Rotator newRotation, bool bSweep /* = false */)
	{
		if (newLocation != mRelativeLocation)
		{
			setRelativeLocationAndRotation(newLocation, mRelativeRotationCache.rotatorToQuat_ReadOnly(newRotation), bSweep);
		}
		else if (!newRotation.equals(mRelativeRotation, SCENECOMPONENT_ROTATOR_TOLERANCE))
		{
			setRelativeLocationAndRotation(newLocation, newRotation.quaternion(), bSweep);
		}
	}


	void SceneComponent::setRelativeScale3D(float3 newScale3D)
	{
		if (newScale3D != mRelativeScale3D)
		{
			if (newScale3D.containsNaN())
			{
				newScale3D = float3(1.f);
			}

			mRelativeScale3D = newScale3D;
			updateComponentToWorld();
			if (isRegistered())
			{
				if (!isDeferringMovementUpdates())
				{
					
				}
				else
				{

				}
			}
		}
	}

	Transform SceneComponent::calcNewComponentToWorld_GeneralCase(const Transform& newRelativeTransform, const SceneComponent* parent, wstring socketName) const
	{
		if (parent != nullptr)
		{
			const Transform parentToWorld = parent->getSocketTransform(socketName);
			Transform newCompToWorld = newRelativeTransform * parentToWorld;

			if (bAbsoluteLocation)
			{
				newCompToWorld.copyTranslation(newRelativeTransform);
			}
			if (bAbsoluteRotation)
			{
				newCompToWorld.copyRotation(newRelativeTransform);
			}

			if (bAbsoluteScale)
			{
				newCompToWorld.copyScale(newRelativeTransform);
			}
			return newCompToWorld;
		}
		else
		{
			return newRelativeTransform;
		}
	}

	bool SceneComponent::shouldComponentAddToScene() const
	{
		return true;
	}

	bool SceneComponent::shouldRender() const
	{
		AActor* owner = getOwner();
		World* world = getWorld();

		if (owner)
		{
			if (auto parentComponent = owner->getParentComponent())
			{
				if (!parentComponent->shouldRender())
				{
					return false;
				}
			}
		}
		const bool bShowInEditor = false;
		const bool binGameWorld = world && world->usesGameHiddenFlags();
		const bool bShowInGame = isVisible() && (!owner || !owner->bHidden);
		return ((binGameWorld && bShowInGame) || (!binGameWorld && bShowInEditor));
	}

	SceneComponent* SceneComponent::getAttachmentRoot() const
	{
		const SceneComponent* top;
		for (top = this; top && top->getAttachParent(); top = top->getAttachParent());
		return const_cast<SceneComponent*>(top);
	}

	AActor* SceneComponent::getAttachmentRootActor() const
	{
		const SceneComponent* attachmentRootComponent = getAttachmentRoot();
		return attachmentRootComponent ? attachmentRootComponent->getOwner() : nullptr;
	}

	BoxSphereBounds SceneComponent::calcBounds(const Transform& localToWorld) const
	{
		BoxSphereBounds newBounds;
		newBounds.mOrigin = localToWorld.getLocation();
		newBounds.mBoxExtent = float3::Zero;
		newBounds.mSphereRadius = 0.f;
		return newBounds;
	}

	bool SceneComponent::isVisible() const
	{
		if (bHiddenInGame)
		{
			return false;
		}
		return bVisible;
	}

	void SceneComponent::updateBounds()
	{
		if (bUseAttachParentBound && getAttachParent() != nullptr)
		{
			mBounds = getAttachParent()->mBounds;
		}
		else
		{
			mBounds = calcBounds(mComponentToWorld);
		}
	}

	void SceneComponent::detachFromComponent(const DetachmentTransformRules& detachmentRules)
	{
		if (getAttachParent() != nullptr)
		{
			AActor* owner = getOwner();
			if (PrimitiveComponent* primComp = dynamic_cast<PrimitiveComponent*>(this))
			{
				primComp->unWeldFromParent();
			}

			BOOST_ASSERT(!bRegistered || getAttachParent()->getAttachChildren().contains(this));
			if (detachmentRules.bCallModify)
			{
				modify();
				getAttachParent()->modify();
			}

			mPrimaryComponentTick.removePrerequisite(getAttachParent(), getAttachParent()->mPrimaryComponentTick);


			getAttachParent()->mAttachChildren.remove(this);

			getAttachParent()->onChildDetached(this);

			mAttachParent = nullptr;
			mAttachSocketName = Name_None;

			onAttachmentChange();

			switch (detachmentRules.mLocationRule)
			{
			case EDetachmentRule::KeepRelative:
				break;
			case EDetachmentRule::KeepWorld:
				mRelativeLocation = mComponentToWorld.getTranslation();
				break;
			}

			switch (detachmentRules.mRotationRule)
			{
			case EDetachmentRule::KeepRelative:
				break;
			case EDetachmentRule::KeepWorld:
				mRelativeRotation = getComponentRotation();
				break;
			}

			switch (detachmentRules.mScaleRule)
			{
			case EDetachmentRule::KeepRelative:
				break;
			case EDetachmentRule::KeepWorld:
				mRelativeScale3D = getComponentScale();
				break;
			}

			updateComponentToWorld();

			if (isRegistered() && !bDisableDetachmentUpdateOverlaps)
			{
				
			}
		}

	}

	bool SceneComponent::attachToComponent(SceneComponent* inParent, const AttachmentTransformRules& attachmentRules, wstring inSocketName /* = Name_None */)
	{
		ObjectThreadContext& threadContext = ObjectThreadContext::get();
		if (threadContext.mIsInConstructor > 0)
		{
			setupAttachment(inParent, inSocketName);
			return true;
		}
		if (inParent != nullptr)
		{
			const bool bSameAttachParentAndSocket = (inParent == getAttachParent() && inSocketName == getAttachSocketName());
			if (bSameAttachParentAndSocket && inParent->getAttachChildren().contains(this))
			{
				return true;
			}

			if (inParent == this)
			{
				return false;
			}

			AActor* myActor = getOwner();
			AActor* theirActor = inParent->getOwner();

			if (myActor == theirActor && myActor && myActor->getRootComponent() == this)
			{
				return false;
			}
			if (inParent->isAttachedTo(this))
			{
				return false;
			}

			if (!inParent->canAttachAsChild(this, inSocketName))
			{
				return false;
			}

			if (inParent->isTemplate() != isTemplate())
			{
				return false;
			}

			const bool bSavedDisableDetachmentUpdateOverlaps = bDisableDetachmentUpdateOverlaps;
			bDisableDetachmentUpdateOverlaps = true;
			int32 lastAttachIndex = INDEX_NONE;
			inParent->getAttachChildren().find(this, lastAttachIndex);
			DetachmentTransformRules detachmentRules(attachmentRules, true);
			if (bSameAttachParentAndSocket && !isRegistered() && attachmentRules.mLocationRule == EAttachmentRule::KeepRelative && attachmentRules.mRotationRule == EAttachmentRule::KeepRelative && attachmentRules.mScaleRule == EAttachmentRule::KeepRelative && lastAttachIndex == INDEX_NONE)
			{

			}
			else
			{
				detachFromComponent(detachmentRules);
			}

			bDisableDetachmentUpdateOverlaps = bSavedDisableDetachmentUpdateOverlaps;
			{
				PrimitiveComponent* primitiveComponent = dynamic_cast<PrimitiveComponent*>(this);

			}

			mPrimaryComponentTick.addPrerequisite(inParent, inParent->mPrimaryComponentTick);
			mAttachParent = std::dynamic_pointer_cast<SceneComponent>(inParent->shared_from_this());
			mAttachSocketName = inSocketName;
			onAttachmentChange();

			if (lastAttachIndex != INDEX_NONE)
			{
				inParent->mAttachChildren.insert(this, lastAttachIndex);
			}
			else
			{
				inParent->mAttachChildren.add(this);
			}

			Transform socketTransform = getAttachParent()->getSocketTransform(getAttachSocketName());

			Transform relativeTM = mComponentToWorld.getRelativeTransform(socketTransform);

			switch (attachmentRules.mLocationRule)
			{
			case EAttachmentRule::KeepRelative:
				break;
			case EAttachmentRule::KeepWorld:
				if (bAbsoluteLocation)
				{
					mRelativeLocation = mComponentToWorld.getTranslation();
				}
				else
				{
					mRelativeLocation = relativeTM.getTranslation();
				}
				break;
			case EAttachmentRule::SnapToTarget:
				mRelativeLocation = float3::Zero;
				break;
			}

			switch (attachmentRules.mRotationRule)
			{
			case EAttachmentRule::KeepRelative:
				break;
			case EAttachmentRule::KeepWorld:
				if (bAbsoluteRotation)
				{
					mRelativeRotation = getComponentRotation();
				}
				else
				{
					mRelativeRotation = mRelativeRotationCache.QuatToRotator(relativeTM.getRotation());
				}
				break;
			case EAttachmentRule::SnapToTarget:
				mRelativeRotation = Rotator::ZeroRotator;
				break;
			}

			switch (attachmentRules.mScaleRule)
			{
			case EAttachmentRule::KeepRelative:
				break;
			case EAttachmentRule::KeepWorld:
				if(bAbsoluteScale)
				{
					mRelativeScale3D = mComponentToWorld.getScale3D();
				}
				else
				{
					mRelativeScale3D = relativeTM.getScale3D();
				}
				break;
			case EAttachmentRule::SnapToTarget:
				mRelativeScale3D = float3(1.0f, 1.0f, 1.0f);
				break;
			}
			getAttachParent()->onChildAttached(this);
			updateComponentToWorld(EUpdateTransformFlags::None, ETeleportType::TeleportPhysics);
			if (PrimitiveComponent* primitiveComponent = dynamic_cast<PrimitiveComponent*>(this))
			{
			}
			if (isRegistered())
			{
				
			}
			return true;
		}
		return false;
	}

	bool SceneComponent::isAttachedTo(class SceneComponent* testComp) const
	{
		if (testComp != nullptr)
		{
			for (const SceneComponent* comp = this->getAttachParent(); comp != nullptr; comp == comp->getAttachParent())
			{
				if (testComp == comp)
				{
					return true;
				}
			}
		}
		return false;
	}

	void SceneComponent::setupAttachment(SceneComponent* inParent, wstring inSocketName /* = Name_None */)
	{
		BOOST_ASSERT(!bRegistered);
		BOOST_ASSERT(mAttachParent == nullptr || !mAttachParent->mAttachChildren.contains(this));
		mAttachParent = std::dynamic_pointer_cast<SceneComponent>(inParent->shared_from_this());
		mAttachSocketName = inSocketName;
	}

	void SceneComponent::setWorldRotation(Rotator newRotation, bool bSweep /* = false */, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (getAttachParent() == nullptr)
		{
			setRelativeRotation(newRotation, bSweep, teleport);
		}
		else
		{
			setWorldRotation(newRotation, bSweep, teleport);
		}
	}

	void SceneComponent::setRelativeRotation(Rotator newRotation, bool bSweep, ETeleportType teleport)
	{
		if (!newRotation.equals(mRelativeRotation, SCENECOMPONENT_ROTATOR_TOLERANCE))
		{
			setRelativeLocationAndRotation(mRelativeLocation, newRotation.quaternion(), bSweep);
		}
	}

	void SceneComponent::detachFromParent(bool bMaintainWorldPosition /* = false */, bool bCallModify /* = true */)
	{
		DetachmentTransformRules detachmentRule(EDetachmentRule::KeepRelative, bCallModify);
		if (bMaintainWorldPosition)
		{
			detachmentRule.mLocationRule = EDetachmentRule::KeepWorld;
			detachmentRule.mRotationRule = EDetachmentRule::KeepWorld;
			detachmentRule.mScaleRule = EDetachmentRule::KeepWorld;
		}
		detachFromComponent(detachmentRule);
	}

	bool SceneComponent::moveComponentImpl(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (isPendingKill())
		{
			return false;
		}

		conditionalUpdateComponentToWorld();
		if (delta.isZero())
		{
			if (newRotation.equals(mComponentToWorld.getRotation(), SCENECOMPONENT_QUAT_TOLERANCE))
			{
				return true;
			}
		}
		const bool bMoved = internalSetWorldLocationAndRotation(getComponentLocation() + delta, newRotation, false, teleport);

		if (bMoved && !isDeferringMovementUpdates())
		{
			
		}
		return true;
	}

	bool SceneComponent::internalSetWorldLocationAndRotation(float3 newLocation, const Quaternion& newQuat, bool bNoPhysics /* = false */, ETeleportType teleport /* = ETeleportType::None */)
	{
		BOOST_ASSERT(bWorldToComponentUpdated);
		Quaternion newRotationQuat(newQuat);

		if (getAttachParent() != nullptr)
		{
			Transform const parentToWorld = getAttachParent()->getSocketTransform(getAttachSocketName());
			if (!bAbsoluteLocation)
			{
				newLocation = parentToWorld.inverseTransformPosition(newLocation);
			}

			if (!bAbsoluteRotation)
			{
				newRotationQuat = parentToWorld.getRotation().inverse() * newRotationQuat;
			}
		}

		const Rotator newRelativeRotation = mRelativeRotationCache.quatToRotato_readOnly(newRotationQuat);
		if (!newLocation.equals(mRelativeLocation) || !newRelativeRotation.equals(mRelativeRotation))
		{
			mRelativeLocation = newLocation;
			mRelativeRotation = newRelativeRotation;
			mRelativeRotationCache.rotatorToQuat(newRelativeRotation);

			updateComponentToWorldWithParent(getAttachParent(), getAttachSocketName(), EUpdateTransformFlags::SkipPhysicsUpdate, mRelativeRotationCache.getCachedQuat(), teleport);

			return true;
		}
		return false;
	}

	void SceneComponent::updateComponentToWorldWithParent(SceneComponent* parent, wstring socketName, EUpdateTransformFlags updateTransformFlags, const Quaternion& relativeRotationQuat, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (parent && !parent->bWorldToComponentUpdated)
		{
			parent->updateComponentToWorld();
			if (bWorldToComponentUpdated)
			{
				return;
			}
		}
		bWorldToComponentUpdated = true;
		Transform newTransform(NoInit);
		{
			const Transform relativeTransform(relativeRotationQuat, mRelativeLocation, mRelativeScale3D);
			newTransform = calcNewComponentToWorld(relativeTransform, parent, socketName);
		}
		bool bHasChanged;
		{
			bHasChanged = !mComponentToWorld.equals(newTransform, SMALL_NUMBER);
		}
		if (bHasChanged)
		{
			mComponentToWorld = newTransform;
			propagateTransformUpdate(true, updateTransformFlags, teleport);
		}
		else
		{
			propagateTransformUpdate(false);
		}
	}

	void SceneComponent::propagateTransformUpdate(bool bTransformChange, EUpdateTransformFlags updateTransformFlags /* = EUpdateTransformFlags::None */, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (isDeferringMovementUpdates())
		{

		}
		const TArray<SceneComponent*>& attachedChildren = getAttachChildren();
		PlatformMisc::prefetch(attachedChildren.getData());
		if (bTransformChange)
		{
			if (bRegistered)
			{
				if (bWantsOnUpdateTransform)
				{
					onUpdateTransform(updateTransformFlags, teleport);
				}
			}
		}
	}

	void SceneComponent::onUpdateTransform(EUpdateTransformFlags updateTransformFlags, ETeleportType teleport /* = ETeleportType::None */)
	{

	}

	DECLARE_SIMPLER_REFLECTION(SceneComponent);
}