#pragma once
#include "EngineMininal.h"
#include "Classes/GameFramework/Controller.h"
#include "GenericPlatform/ICursor.h"
#include "InputCoreType.h"
namespace Air
{
	class Player;
	class ENGINE_API APlayerController : public AController
	{
		GENERATED_RCLASS_BODY(APlayerController, AController)
	public:
		AActor * getViewTarget() const;

		virtual void setViewTarget(class AActor* newViewTarget);

		virtual bool inputKey(Key key, EInputEvent eventType, float amountDepressed, bool bGamepad);

		virtual bool inputAxis(Key key, float delta, float deltaTime, int32 numSamples, bool bGamepad);

		void setAsLocalPlayerController() { bIsLocalPlayerController = true; }

		virtual void setPlayer(Player* inPlayer);

		virtual void initInputSystem();


		virtual bool popInputComponent(InputComponent* inInputComponent);

		virtual void pushInputComponent(InputComponent* inInputComponent);

		virtual void postInitializeComponents() override;

		virtual void spawnPlayerCameraManager();

		virtual void resetCameraMode();

		virtual bool isLocalController() const;

		virtual void receivedPlayer();

		virtual void possess(APawn* inPawn) override;
		
		virtual void unPossess() override;

		virtual void addPitchInput(float val);

		virtual void addYawInput(float val);

		virtual void addRollInput(float val);

		virtual void autoManageActiveCameraTarget(AActor* suggestedTarget);

		void clientRestart(class APawn* newPawn);

		virtual void updateRotation(float deltaTime);

		virtual void receivedGameModeClass(TSubclassOf<class GameModeBase> gameModeClass);

		virtual void receivedSpectatorClass(TSubclassOf<class ASpectatorPawn> spectatorClass);

		virtual void tickActor(float deltaTime, enum ELevelTick tickType, ActorTickFunction& thisTickFunction) override;

		virtual void playerTick(float deltaTime);
		virtual void preProcessInput(const float deltaTime, const bool bGamePaused);

		virtual void postProcessInput(const float deltaTime, const bool bGamePaused);

		class APawn* getPawnOrSpectator() const;

		bool inputEnabled() const { return bInputEnabled; }
	
		virtual void updateCameraManager(float deltaSeconds);

		virtual void getPlayerViewPoint(float3& outLocation, Rotator& outRotation) const override;

		virtual EMouseCursor::Type getMouseCursor() const;

		virtual bool shouldShowMouseCursor() const;

	protected:
		void tickPlayerInput(const float deltaSeconds, const bool bGamePaused);

		virtual void setupInputComponent();

		virtual void destroySpectatorPawn();

		virtual ASpectatorPawn* spawnSpectatorPawn();

		virtual void setSpectatorPawn(class ASpectatorPawn* newSpectatorPawn);

		virtual void processPlayerInput(const float deltaTime, const bool bGamePaused);

		virtual void buildInputStack(TArray<InputComponent*>& inputStack);

		void processForceFeedbackAndHaptics(const float deltaTime, const bool bGamePaused) {  }

	
	protected:
		float3 mSpawnLocation;
	public:
		float3 getSpawnLocation() const { return mSpawnLocation; }
	public:
		class Player* mPlayer;

		class PlayerCameraManager* mPlayerCameraManager;

		class PlayerInput* mPlayerInput;

		float mLocalPlayerCachedLODDistanceFactor;

		uint8 mNetPlayerIndex{ 0 };

		mutable bool bIsLocalPlayerController{ false };

		uint32 bPlayerIsWaiting : 1;

		uint32 bInputEnabled : 1;

		uint32 bShouldShowCursor : 1;

		Rotator mTargetViewRotation;

	protected:
		TArray<InputComponent*> mCurrentInputStack;

	public:
		//Spectating

		class ASpectatorPawn* getSpectatorPawn() const { return mSpectatorPawn; }

	protected:
		virtual void beginSpectatingState();

	private:
		class ASpectatorPawn* mSpectatorPawn;

		Rotator mRotationInput;

		float mInputPitchScale{ 1.0f };
		float mInputYawScale{ 1.0f };
		float mInputRollScale{ 1.0f };
	public:
		bool bAutoManagerActiveCameraTarget{ false };
	};
}