#pragma once
#include "EngineMininal.h"
#include "Classes/GameFramework/Actor.h"
#include "Math/Rotator.h"
namespace Air
{
	enum EStateName
	{
		Spectating,
		Inactive,
		Playing,
	};

	class APawn;
	class ENGINE_API AController : public AActor
	{
		GENERATED_RCLASS_BODY(AController, AActor)
	public:
		virtual bool isLocalController() const;


		FORCEINLINE_DEBUGGABLE bool isPlayerController() const
		{
			return bIsPlayerController;
		}

		FORCEINLINE_DEBUGGABLE bool isLocalPlayerController() const
		{
			return isPlayerController() && isLocalController();
		}

		virtual void initPlayerState();

		virtual void getPlayerViewPoint(float3& outLocation, Rotator& outRotation) const;

		FORCEINLINE APawn* getPawn() const { return mPawn; }

		EStateName getStateName() const;

		bool isInState(EStateName inStateName) const;

		//将controller与特定Pawn绑定
		virtual void possess(APawn* inPawn);
		//取消controller对pawn的控制
		virtual void unPossess();

		virtual void postInitializeComponents() override;

		virtual Rotator getControllerRotation() const;

		virtual void setPawn(APawn* inPawn);

		virtual void setControlRotation(const Rotator& newRotation);

		virtual void removePawnTickDependency(APawn* inOldPawn);

		virtual void addPawnTickDependency(APawn* inPawn);

		virtual void attachToPawn(APawn* inPawn);

		virtual void detachFromPawn();

		virtual bool isLookInputIgnored() const;

		virtual void changeState(EStateName);
	protected:
		virtual void beginInactiveState();

		virtual void endInactiveState();

	public:
		uint32 bIsPlayerController : 1;

		uint32 bAttachToPawn : 1;

		APawn* mPawn;

		EStateName mStateName;


		Rotator mControlRotation;
		
		class APlayerState* mPlayerState;
	
		class SceneComponent* mTransformComponent;
	};
}