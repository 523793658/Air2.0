#pragma once
#include "core/DemoConfig.h"
#include "AirClient.h"
#include "ShowFlags.h"
#include "SceneTypes.h"
#include "Input/Events.h"
namespace Air
{
	class SceneView;
	class SceneViewFamily;
	class SWidget;
	class LocalPlayer;
	class GameInstance;
	struct DEMO_API ViewportCameraTransform 
	{
	public:
		ViewportCameraTransform();
		void setLocation(const float3& position)
		{
			mViewLocation = position;
			mDesiredLocation = mViewLocation;
		}

		void setRotation(const Rotator& rotation)
		{
			mViewRotation = rotation;
		}
		void setLookAt(const float3 & inLookAt)
		{
			mLookAt = inLookAt;
		}

		void setOrthoZoom(float inOrthoZoon)
		{
			mOrthoZoom = inOrthoZoon;
		}

		bool isPlaying();

		FORCEINLINE const float3& getLocation() const { return mViewLocation; }
		FORCEINLINE const Rotator& getRotation() const { return mViewRotation; }
		FORCEINLINE const float3& getLookAt() const { return mLookAt; }
		FORCEINLINE float getOrthoZoom() const { return mOrthoZoom; }
		void transitionToLocation(const float3 & inDesiredLocation, std::weak_ptr<SWidget> editorViewportWidget, bool bInstant);
		bool updateTransition();
		Matrix computeOrbitMatrix() const;

	private:
		float3 mViewLocation;
		Rotator mViewRotation;
		float3 mDesiredLocation;
		float3 mLookAt;
		float3 mStartLocation;
		float mOrthoZoom;
	};

	class DemoViewportClient :public Object, public CommonViewportClient
	{
		GENERATED_RCLASS_BODY(DemoViewportClient, Object)
	public:
		
		void init(struct WorldContext& inWorldContext, GameInstance* owningGameInstance);

		virtual void draw(Viewport* viewport, Canvas* canvas) override;

		virtual World* getWorld() const  override { return mWorld; }

		SceneView* calcSceneView(SceneViewFamily* viewFamily, const EStereoscopicPass stereoPass = eSSP_FULL);

		virtual void setupViewForRendering(SceneViewFamily& viewFamily, SceneView& view);
		ViewportCameraTransform& getViewTransform()
		{
			return mViewTransformPerspective;
		}

		virtual bool inputKey(Viewport* viewport, int32 controllerId, Key key, EInputEvent e, float amountDepressed = 1.0f, bool bGamepad = false) override;
		
		virtual bool inputAxis(Viewport* viewport, int32 controllerId, Key key, float delta, float deltaTime, int32 numSamples = 1, bool bGamepad = false) override;

		virtual bool ignoreInput() override
		{
			return bIgnoreInput;
		}

		LocalPlayer* setupInitialLocalPlayer(wstring& outError);

		virtual void finalizeViews(class SceneViewFamily* viewFamily, const TMap<LocalPlayer*, SceneView*>& playerViewMap) {}

		EMouseCursor::Type getCursor(Viewport* viewport, int32 x, int32 y);
	private:

		

		SceneInterface* getScene() const;

		float getNearClipPlane() const;

		LinearColor getBackgroundColor() const;

	private:
		World* mWorld;

		EngineShowFlags mEngineShowFlags;

		double mTimeForForceRedraw;

		bool bUseControllingActorViewInfo{ false };

		bool bUseingOrbitCamera{ false };

		bool bIgnoreInput{ false };

		float mViewFOV;

		float mNearPlane;

		float mFarPlane;

		ViewportCameraTransform mViewTransformPerspective;

		SceneViewStateReference mViewState;

		SceneViewStateReference mStereoViewState;

		GameInstance* mGameInstance;
	};
}