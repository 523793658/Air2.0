#include "Classes/Components/PrimitiveComponent.h"
#include "SimpleReflection.h"
#include "Classes/Engine/World.h"
#include "SceneInterface.h"
namespace Air
{
	ThreadSafeCounter PrimitiveComponent::NextComponentId;


	PrimitiveComponent::PrimitiveComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:SceneComponent(objectInitializer),
		bRenderInMainPass(true)
	{
		mLastRenderTime = -1000.f;
		mLastRenderTimeOnScreen = -1000.f;
		bWantsOnUpdateTransform = true;

		mComponentId.mPrimIdValue = NextComponentId.increment();
	}
	void PrimitiveComponent::createRenderState_Concurrent()
	{
		ParentType::createRenderState_Concurrent();
		if (shouldComponentAddToScene())
		{
			getWorld()->mScene->addPrimitive(this);
		}
	}

	std::shared_ptr<MaterialInterface> PrimitiveComponent::getMaterial(int32 elementIndex) const
	{
		return std::shared_ptr<MaterialInterface>();
	}

	void PrimitiveComponent::setMaterial(int32 elementIndex, class MaterialInterface* material)
	{

	}

	bool PrimitiveComponent::shouldComponentAddToScene() const
	{
		bool bSceneAdd = SceneComponent::shouldComponentAddToScene();
		return bSceneAdd && (shouldRender() || bCastHiddenShadow);
	}

	Matrix PrimitiveComponent::getRenderMatrix() const
	{
		return mComponentToWorld.toMatrixWithScale();
	}

	void PrimitiveComponent::sendRenderTransform_Concurrent()
	{
		updateBounds();
		if (shouldRender() || bCastHiddenShadow)
		{
			getWorld()->mScene->updatePrimitiveTransform(this);
		}
		ParentType::sendRenderTransform_Concurrent();
	}

	bool PrimitiveComponent::weldToImplementation(SceneComponent* inParent, wstring parentSocketName /* = Name_None */, bool bWeldSimulatedChild /* = true */)
	{
		return true;
	}

	void PrimitiveComponent::unWeldFromParent()
	{

	}

	void PrimitiveComponent::unWeldChildren()
	{

	}

	DECLARE_SIMPLER_REFLECTION(PrimitiveComponent)
}