#pragma once
#include "EngineMininal.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Camera/CameraTypes.h"
namespace Air
{
	struct CameraCacheEntry
	{
	public:
		float mTimeStamp;
		MinimalViewInfo mPOV;
		CameraCacheEntry()
			:mTimeStamp(0.0f)
		{}
	};

	struct ENGINE_API TViewTarget
	{
	public:
		TViewTarget()
			:mTarget(nullptr)
			,mPlayerState(nullptr)
		{}
		
		void checkViewTarget(APlayerController* owningController);

		bool equal(const TViewTarget& otherTarget) const;

		void setNewTarget(AActor* newTarget);

		class APawn* getTargetPawn() const;


		class AActor* mTarget;

		struct MinimalViewInfo mPOV;

	protected:

		class APlayerState* mPlayerState;

		friend class APlayerController;
	};


	class ENGINE_API PlayerCameraManager : public AActor
	{
		GENERATED_RCLASS_BODY(PlayerCameraManager, AActor)
	public:
		AActor* getViewTarget() const;
		void setViewTarget(class AActor* newTarget);
		void assignViewTarget(AActor* newTarget, TViewTarget& vt);
		virtual void initializeFor(class APlayerController* pc);

		virtual void updateCamera(float deltaTime);

		class APawn* getViewTargetPawn() const;

		void fillCameraCache(const MinimalViewInfo& newInfo);

		virtual float getFOVAngle() const;

		void getCameraViewPoint(float3 & outCamLoc, Rotator& outCamRot) const;

		virtual void processViewRotation(float deltaTime, Rotator& outViewRotation, Rotator& outDeltaRot);
	protected:
		virtual void doUpdateCamera(float deltaTime);

		virtual void limitViewPitch(Rotator& viewRotation, float inViewPitchMin, float inViewPitchMax);

		virtual void limitViewYaw(Rotator& viewRotation, float inViewYawMin, float inViewYawMax);

		virtual void limitViewRoll(Rotator& viewRotation, float inViewRollMin, float inViewRollMax);

		virtual void udpateViewTarget(TViewTarget& outVT, float deltaTime);

		virtual void updateViewTargetInternal(TViewTarget& outVT, float deltaTime);
	public:
		float mViewPitchMin;
		float mViewPitchMax;

		float mViewYawMin;
		float mViewYawMax;

		float mViewRollMin;
		float mViewRollMax;


		uint32 bGameCameraCutThisFrame : 1;

		struct TViewTarget mViewTarget;
		struct TViewTarget mPendingViewTarget;
		std::shared_ptr<class APlayerController> mPCOwner;
		float mDefaultAspectRatio;
		float mDefaultOrthoWidth{ 512 };
		float mDefaultFOV;
		struct CameraCacheEntry	mCameraCache;
		struct CameraCacheEntry mLastCameraCache;

		uint32 bIsOrthographic : 1;

		wstring mCameraStyle;

	protected:
		float mLockedFOV{ 0.0f };

	private:
		std::shared_ptr<class SceneComponent> mTransformComponent;
	};
}