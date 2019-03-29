#include "Classes/Engine/GameInstance.h"
#include "Classes/Engine/Engine.h"
#include "Classes/Engine/World.h"
#include "Classes/GameMapsSetting.h"
#include "Classes/GameFramework/PlayerController.h"
#include "SimpleReflection.h"
#include "Classes/GameFramework/GameModeBase.h"
namespace Air
{
	GameInstance::GameInstance(const ObjectInitializer& initializer)
		:ParentType(initializer)
	{

	}

	GameInstance::GameInstance(Engine* inEngine)
		:mEngine(inEngine)
	{
	}

	const TArray<LocalPlayer*>& GameInstance::getLocalPlayers() const
	{
		return mLocalPlayers;
	}

	Engine* GameInstance::getEngine() const
	{
		return dynamic_cast<Engine*>(getOuter());
	}

	void GameInstance::init()
	{

	}

	void GameInstance::initializeStandalone(EWorldType::Type inType)
	{
		mWorldContext = &getEngine()->createNewWorldContext(inType);
		mWorldContext->mOwningGameInstance = this;

		World* dummyWorld = World::createWorld(inType, false);
		dummyWorld->setGameInstance(this);
		mWorldContext->setCurrentWorld(dummyWorld);
		init();
	}


	ViewportClient* GameInstance::getGameViewportClient() const
	{
		WorldContext* const wc = getWorldContext();
		return wc ? wc->mGameViewport : nullptr;
	}

	int32 GameInstance::addLocalPlayer(LocalPlayer* newLocalPlayer, int32 ControllerId)
	{
		if (newLocalPlayer == nullptr)
		{
			return INDEX_NONE;
		}
		const int32 insertIndex = mLocalPlayers.size();
		mLocalPlayers.push_back(newLocalPlayer);

		newLocalPlayer->playerAdded(getGameViewportClient(), ControllerId);

		if (getGameViewportClient() != nullptr)
		{
			//getGameViewportClient();
		}
		return insertIndex;
	}

	LocalPlayer* GameInstance::createInitialPlayer(wstring &outError)
	{
		return createLocalPlayer(0, outError, false);
	}

	LocalPlayer* GameInstance::createLocalPlayer(int32 ControllerId, wstring& outError, bool bSpawnActor)
	{
		//BOOST_ASSERT(getEngine()->mlocal)
		LocalPlayer* newPlayer = nullptr;
		int32 insertIndex = INDEX_NONE;
		const int32 maxSplitesscreenPlayers = 1;
		if (findLocalPlayerFromControllerId(ControllerId) != nullptr)
		{
			outError = L"A local player already exists for controller ID " + ControllerId;
		}
		else if (mLocalPlayers.size() < maxSplitesscreenPlayers)
		{
			if (ControllerId < 0)
			{
				for (ControllerId = 0; ControllerId < maxSplitesscreenPlayers; ++ControllerId)
				{
					if (findLocalPlayerFromControllerId(ControllerId) == nullptr)
					{
						break;
					}
				}
			}
			else if(ControllerId >= maxSplitesscreenPlayers)
			{
				outError = L"Controller ID " + ControllerId;
				outError += L"is unlikely to map to any physical device, so this player will not receive input";
			}
			newPlayer = newObject<LocalPlayer>(getEngine(), getEngine()->mLocalPlayerClass);
			insertIndex = addLocalPlayer(newPlayer, ControllerId);
			if (bSpawnActor && insertIndex != INDEX_NONE && getWorld() != nullptr)
			{
				//if(newPlayer->spa)
			}
		}
		else
		{
			outError = TEXT("Maximum number of players (%d) already created.  Unable to create more.");
		}

		if (outError != TEXT(""))
		{

		}
		return newPlayer;
	}

	LocalPlayer* GameInstance::findLocalPlayerFromControllerId(const int32 controllerID) const
	{
		for (LocalPlayer* lp : mLocalPlayers)
		{
			if (lp && (lp->getControllerId() == controllerID))
			{
				return lp;
			}
		}
		return nullptr;
	}

	World* GameInstance::getWorld() const
	{
		return mWorldContext ? mWorldContext->getWorld() : nullptr;
	}

	void GameInstance::startGameInstance()
	{
		Engine* const engine = getEngine();

		URL defaultURL;
		defaultURL.loadURLConfig(TEXT("DefualtPlayer"), GGameIni);

		const GameMapsSettings* gameMapsSettings = GameMapsSettings::getDefault();
		const wstring& defaultMap = gameMapsSettings->getGameDefaultMap();

		wstring packageName = defaultMap;


		EBrowseReturnVal::Type browseRet = EBrowseReturnVal::Failure;
		URL url(&defaultURL, packageName.c_str(), TRAVEL_Partial);
		if (url.valid)
		{
			browseRet = engine->browse(*mWorldContext, url);
		}
	}

	bool GameInstance::startByDemo(LocalPlayer* localPlayer)
	{
		World* const playWorld = getWorld();
		if (localPlayer)
		{
			wstring error;
			if (!localPlayer->spawnPlayActor(TEXT(""), error, playWorld))
			{

			}
		}
		return true;
	}

	GameModeBase* GameInstance::createGameModeForURL(URL inURL)
	{
		World* world = getWorld();

		RClass* gameClass = GameModeBase::StaticClass();
		ActorSpawnParameters spawnInfo;
		spawnInfo.objectFlags |= RF_Transient;
		return world->spawnActor<GameModeBase>(gameClass, spawnInfo);
	}

	APlayerController* GameInstance::getFirstLocalPlayerController(World* inWorld /* = nullptr */) const
	{
		if (inWorld == nullptr)
		{
			for (LocalPlayer* player : mLocalPlayers)
			{
				if (player && player->mPlayerComtroller)
				{
					return player->mPlayerComtroller;
				}
			}
		}
		else
		{
			for (ConstPlayerControllerIterator iterator = inWorld->getPlayerControllerIterator(); iterator; ++iterator)
			{
				if (*iterator != nullptr && (*iterator)->isLocalController())
				{
					return *iterator;
				}
			}
		}
		return nullptr;
	}

	DECLARE_SIMPLER_REFLECTION(GameInstance);
}