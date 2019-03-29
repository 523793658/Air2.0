#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/Player.h"
#include "Viewport.h"
#include "SceneView.h"
#include "Classes/Engine/GameViewportClient.h"
#include "Classes/Camera/CameraTypes.h"
namespace Air
{
	class SceneView;
	class ENGINE_API LocalPlayer : public Player
	{
		GENERATED_RCLASS_BODY(LocalPlayer, Player)
	public:

		bool calcSceneViewInitOptions(struct SceneViewInitOptions& outInitOptions, Viewport* viewport, class ViewElementDrawer* viewDrawer = nullptr, EStereoscopicPass stereopass = eSSP_FULL);

		SceneView* calcSceneView(class SceneViewFamily* viewFamily, float3& outViewLocation, Rotator& outViewRotation, Viewport* viewport, class ViewElementDrawer* viewDrawer = nullptr, EStereoscopicPass stereoPass = eSSP_FULL);

		void getViewPoint(MinimalViewInfo& outViewInfo, EStereoscopicPass stereoPass) const;

		bool getProjectionData(Viewport* viewport, EStereoscopicPass stereoPass, SceneViewProjectionData& projectionData) const;
		
		void playerAdded(ViewportClient* inViewportClient, int32 inControllerId)
		{
			mViewportClient = inViewportClient;
			mControllerId = inControllerId;
		}

		int32 getControllerId() const { return mControllerId; }

		virtual bool spawnPlayActor(const wstring& URL, wstring& outError, World* inWorld);
	private:
		float2 mOrigin;
		float2 mSize;

		TEnumAsByte<enum EAspectRatioAxisConstraint> mAspectRatioAxisConstraint{ EAspectRatioAxisConstraint::AspectRatio_MaintainXFOV };

		class ViewportClient* mViewportClient;

		float3 mLastViewLocation;

		int32 mControllerId;

		

	public:
		friend class GameViewportClient;
		friend class DemoViewportClient;
	private:
		SceneViewStateReference mViewState;
		SceneViewStateReference mStereoViewState;
		SceneViewStateReference mMonoViewState;
	};
}