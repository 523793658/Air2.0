#pragma once
#include "EngineMininal.h"
#include "Object.h"
#include "Classes/Engine/EngineType.h"
#include "Classes/Engine/EngineBaseTypes.h"
namespace Air
{
	class LocalPlayer;
	class Engine;
	class World;
	class ViewportClient;
	class APlayerController;
	class ENGINE_API GameInstance : public Object
	{
		GENERATED_RCLASS_BODY(GameInstance, Object)
	public:

	
		GameInstance(Engine* inEngine);

		struct WorldContext* getWorldContext() const { return mWorldContext; }

		const TArray<LocalPlayer*>& getLocalPlayers() const;

		TArray<class LocalPlayer*>::TConstIterator getLocalPlayerIterator() const
		{
			return mLocalPlayers.createConstIterator();
		}

		void initializeStandalone(EWorldType::Type inType = EWorldType::Game);

		Engine* getEngine() const;

		virtual void init();

		int32 addLocalPlayer(LocalPlayer* newLocalPlayer, int32 ControllerId);
		
		ViewportClient* getGameViewportClient() const;

		LocalPlayer* createInitialPlayer(wstring &outError);

		LocalPlayer* createLocalPlayer(int32 ControllerId, wstring& outError, bool bSpawnActor);

		LocalPlayer* findLocalPlayerFromControllerId(const int32 controllerID) const;

		virtual class GameModeBase* createGameModeForURL(URL inURL);

		virtual void startGameInstance();

		virtual class World* getWorld() const override;

		APlayerController* getFirstLocalPlayerController(World* inWorld = nullptr) const;

		virtual bool startByDemo(LocalPlayer* localPlayer);
	private:
		struct WorldContext*	mWorldContext;

		TArray<LocalPlayer*>	mLocalPlayers;

		Engine* mEngine;
	};
}