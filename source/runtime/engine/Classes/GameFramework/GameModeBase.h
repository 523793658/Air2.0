#pragma once
#include "Classes/GameFramework/Info.h"
namespace Air
{
	class APlayerController;
	class Player;
	class AGameStateBase;
	class ASpectatorPawn;
	class ENGINE_API GameModeBase : public Info
	{
		GENERATED_RCLASS_BODY(GameModeBase, Info)
	public:
		virtual APlayerController* spawnPlayerController(ENetRole inRemoteRole, float3 const & spawnLocation, Rotator const& spawnRotation);

		virtual wstring initNewPlayer(APlayerController* newPlayerController);

		virtual APlayerController* login(Player* newPlayer, ENetRole inRemoteRole, const wstring& portal, const wstring& options);

		virtual void preInitializeComponents() override;

		virtual void initGameState();

		virtual void startPlay();
	public:
		TSubclassOf<AGameStateBase> mGameStateClass;

		AGameStateBase* mGameState;

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