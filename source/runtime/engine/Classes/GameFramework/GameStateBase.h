#pragma once
#include "CoreMinimal.h"
#include "Classes/GameFramework/GameModeBase.h"
#include "Classes/GameFramework/Info.h"
#include "Templates/SubclassOf.h"
namespace Air
{
	class ASpectatorPawn;
	class AController;
	class ENGINE_API AGameStateBase : public Info
	{
		GENERATED_RCLASS_BODY(AGameStateBase, Info)
	public:
		TSubclassOf<GameModeBase> mGameModeClass;
		GameModeBase* mAuthorityGameMode;
		TSubclassOf<ASpectatorPawn> mSpectatorClass;

		const AGameStateBase* getDefaultGameMode() const;

		template<class T>
		const T* getDefaultGameMode() const
		{
			return dynamic_cast<T*>(getDefaultGameMode());
		}

		virtual void receiveGameModeClass();

		virtual void receiveSpectatorClass();


		friend class APlayerController;
	
		virtual void handleBeginPlay();

	};

}