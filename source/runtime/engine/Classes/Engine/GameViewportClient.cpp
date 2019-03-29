#include "Classes/Engine/GameViewportClient.h"
#include "Classes/Engine/WorldContext.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "AirEngine.h"
#include "CoreGlobals.h"
#include "Classes/Engine/Engine.h"
#include "SceneViewExtension.h"
#include "Classes/Engine/GameInstance.h"
#include "SceneManagement.h"
namespace Air
{



	bool GameViewportClient::layoutPlayers()
	{
		return true;
	}

	void GameViewportClient::init(struct WorldContext& inWorldContext, GameInstance* owningGameInstance, bool bCreateNewAudioDevice /* = true */)
	{
		inWorldContext.addRef(mWorld);
	}

	LocalPlayer* GameViewportClient::setInitialLocalPlayer(wstring& outError)
	{
		GameInstance* viewportGameInstance = GEngine->getWorldContextFromViewportChecked(this).mOwningGameInstance;

		if (viewportGameInstance == nullptr)
		{
			BOOST_ASSERT("invalid game instance");
			return nullptr;
		}

		return viewportGameInstance->createInitialPlayer(outError);
	}

	static void GatherViewExtension(Viewport* inViewport, TArray<std::shared_ptr<class ISceneViewExtension>>& outViewExtensions)
	{

	}

	class GameViewDrawer : public ViewElementDrawer
	{
	public:

	};

	SceneView* GameViewportClient::calcSceneView(SceneViewFamily* viewFamily, const EStereoscopicPass stereoPass)
	{
		SceneViewInitOptions viewInitOptions;
		viewInitOptions.mViewOrigin = float3(EForceInit::ForceInit);
		viewInitOptions.setViewRectangle(IntRect(0, 0, mViewport->getSizeXY().x, mViewport->getSizeXY().y));
		viewInitOptions.mWorldToMetersScale = 1.0f;

		viewInitOptions.mViewRotationMatrix = Matrix::Identity;
		const float matrixFOV = std::max<float>(0.001f, 45.f) *(float)PI / 360.0f;

		viewInitOptions.mProjectionMatrix = ReversedZPerspectiveMatrix(matrixFOV, 1.0, mViewport->getDesiredAspectRatio(), GNearClippingPlane, 1000.0f);
		viewInitOptions.mViewFamily = viewFamily;
		GameViewDrawer gameViewDrawer;

		viewInitOptions.mViewElementDrawer = &gameViewDrawer;
		viewInitOptions.mBackgroundColor = LinearColor::Red;
		SceneView* view = new SceneView(viewInitOptions);
		view->mViewLocation = float3(EForceInit::ForceInit);
		view->mViewRotation = Rotator::ZeroRotator;
		viewFamily->mViews.push_back(view);

		for (int viewExt = 0; viewExt < viewFamily->mViewExtensions.size(); viewExt++)
		{
			viewFamily->mViewExtensions[viewExt]->setupView(*viewFamily, *view);
		}
		return view;
	}

	

	void GameViewportClient::draw(Viewport* inViewport, Canvas* sceneCanvas)
	{

		World* myWorld = getWorld();

		GameViewDrawer gameViewDrawer;

		SceneViewFamilyContext viewFamily(SceneViewFamily::ConstructionValues(inViewport, myWorld->mScene, mEngineShowFlags).setRealtimeUpdates(true));

		GatherViewExtension(inViewport, viewFamily.mViewExtensions);

		for (auto viewExt : viewFamily.mViewExtensions)
		{
			viewExt->setupViewFamily(viewFamily);
		}

		std::map<LocalPlayer*, SceneView*> playerViewMap;

		for (LocalPlayerIterator it(GEngine, myWorld); it; ++it)
		{
			LocalPlayer* localPlayer = *it;
			if (localPlayer)
			{
				const int32 numViews = 1;
				for (int32 i = 0; i < numViews; ++i)
				{
					EStereoscopicPass passType = eSSP_FULL;
					float3 viewLocation;
					Rotator viewRotation;
					SceneView* view = calcSceneView(&viewFamily, passType);

					if (view)
					{
						if (false)
						{

						}
						else if(false)
						{

						}
						else if (false)
						{

						}
						if (false)
						{

						}
						if (false)
						{

						}

						view->mCameraConstrainedViewRect = view->mUnscaledViewRect;

						if (i == 0)
						{
							localPlayer->mLastViewLocation = viewLocation;

							playerViewMap.emplace(localPlayer, view);
						}
					}
				}
			}
		}




		if (!bDisableWorldRendering && playerViewMap.size() > 0)
		{
			getRendererModule().beginRenderingViewFamily(sceneCanvas, &viewFamily);
		}
	}

	World* GameViewportClient::getWorld() const
	{
		return mWorld;
	}

	void GameViewportClient::setViewportOverlayWidget(std::shared_ptr<SWindow> inWindow, std::shared_ptr<SOverlay> inViewportOverLayWidget)
	{
		mWindow = inWindow;
		mViewportOverlayWidget = inViewportOverLayWidget;
	}

	void GameViewportClient::setGameLayerManager(std::shared_ptr<IGameLayerManager> layerManager)
	{
		mGameLayerManagerPtr = layerManager;
	}

	void GameViewportClient::setViewportFrame(ViewportFrame* inViewportFrame)
	{
		mViewportFrame = inViewportFrame;
		setViewport(mViewportFrame ? mViewportFrame->getViewport() : nullptr);
	}

	void GameViewportClient::setViewport(Viewport* inViewport)
	{
		Viewport* previousViewport = mViewport;
		mViewport = inViewport;
		if (previousViewport == nullptr && mViewport != nullptr)
		{
			layoutPlayers();
		}
	}
}