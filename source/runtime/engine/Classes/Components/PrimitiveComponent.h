#pragma once
#include "Classes/Components/SceneComponent.h"
#include "SceneTypes.h"
namespace Air
{
	class PrimitiveSceneProxy;
	class ENGINE_API PrimitiveComponent : public SceneComponent
	{
		GENERATED_RCLASS_BODY(PrimitiveComponent, SceneComponent)

	public:
	

		bool shouldComponentAddToScene() const;

		void createRenderState_Concurrent() override;

		virtual PrimitiveSceneProxy* createSceneProxy()
		{
			return nullptr;
		}

		virtual Matrix getRenderMatrix()const;

		virtual class MaterialInterface* getMaterial(int32 elementIndex) const;

		virtual void unWeldFromParent();

		virtual void unWeldChildren();

		virtual bool weldToImplementation(SceneComponent* inParent, wstring parentSocketName = Name_None, bool bWeldSimulatedChild = true);

		virtual void setMaterial(int32 elementIndex, class MaterialInterface* material);
	protected:
		virtual void sendRenderTransform_Concurrent() override;

	public:
		float mLastSubmitTime{ 0.0f};

		PrimitiveSceneProxy* mSceneProxy{ nullptr };

		PrimitiveComponentId mComponentId;

		ThreadSafeCounter mAttachmentCounter;

		float mLastRenderTime{ 0.0f };
		float mLastRenderTimeOnScreen{ 0.0f };
	public:
		uint32 bOnlyOwnerSee : 1;
		uint32 bOwnerNoSee : 1;
		uint32 bCastHiddenShadow : 1;
		uint32 bRenderInMainPass : 1;

		uint32 bCastShadow : 1;

		static ThreadSafeCounter NextComponentId;
	};
}