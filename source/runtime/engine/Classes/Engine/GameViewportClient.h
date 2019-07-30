#pragma once
#include "Classes/Engine/ScriptViewportClient.h"
#include "Viewport.h"
#include "Classes/Engine/World.h"
#include "ShowFlags.h"
#include <map>
namespace Air
{
	class GameInstance;
	class SWindow;
	class SOverlay;
	class IGameLayerManager;
	class LocalPlayer;
	class SceneView;
	class SceneViewFamily;
	

	class ENGINE_API GameViewportClient : public ScriptViewportClient
	{

	public:
		virtual bool layoutPlayers();

		virtual World* getWorld() const override;

	public:

		virtual void init(struct WorldContext& inWorldContext, GameInstance* owningGameInstance, bool bCreateNewAudioDevice = true);
		void setViewportOverlayWidget(std::shared_ptr<SWindow> inWindow, std::shared_ptr<SOverlay> inViewportOverLayWidget);

		void setGameLayerManager(std::shared_ptr<IGameLayerManager> layerManager);

		void setViewportFrame(ViewportFrame* inViewportFrame);

		void setViewport(Viewport* inViewport);

		virtual void draw(Viewport* viewport, Canvas* canvas) override;

		virtual std::shared_ptr<LocalPlayer> setInitialLocalPlayer(wstring& outError);

	private:
		SceneView* calcSceneView(SceneViewFamily* viewFamily, const EStereoscopicPass stereoPass);


	public:
		EngineShowFlags mEngineShowFlags;
	private:
		class World* mWorld;

		bool bDisableWorldRendering : 1;

		std::weak_ptr<SWindow> mWindow;
		std::weak_ptr<SOverlay> mViewportOverlayWidget;

		std::weak_ptr<IGameLayerManager> mGameLayerManagerPtr;

		ViewportFrame* mViewportFrame;

		std::map<LocalPlayer*, SceneView*> mPlayerViewMap;

	};
}