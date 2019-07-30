#pragma once
#include "Classes/GameFramework/Info.h"
namespace Air
{
	class APlayerController;
	class Player;
	class AGameStateBase;
	class ASpectatorPawn;
	class ENGINE_API GameModeBase : public AInfo
	{
		GENERATED_RCLASS_BODY(GameModeBase, AInfo)
	public:
		virtual std::shared_ptr<APlayerController> spawnPlayerController(ENetRole inRemoteRole, float3 const & spawnLocation, Rotator const& spawnRotation);

		virtual wstring initNewPlayer(APlayerController* newPlayerController);

		virtual std::shared_ptr<APlayerController> login(Player* newPlayer, ENetRole inRemoteRole, const wstring& portal, const wstring& options);

		virtual void preInitializeComponents() override;

		virtual void initGameState();

		virtual void startPlay();
	public:
		TSubclassOf<AGameStateBase> mGameStateClass;

		std::shared_ptr<AGameStateBase> mGameState;

		TSubclassOf<ASpectatorPawn> mSpectatorClass;

		TSubclassOf<APlayerController> mReplaySpectatorPlayerControllerClass;

		TSubclassOf<class APawn> mDefaultPawnClass;

		TSubclassOf<APlayerController> mPlayerControllerClass;

		TSubclassOf<class APlayerState> mPlayerStateClass;

	protected:
		uint32 bStartPlayersAsSpectator : 1;
		uint32 bPauseable : 1;
	};
}