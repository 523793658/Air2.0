#include "DefaultGameMoviePlayer.h"
#include "HAL/PlatformProperties.h"
#include "RenderingThread.h"
#include "Widgets/SWindow.h"
#include "Classes/Engine/GameEngine.h"
#include "Widgets/SViewport.h"
namespace Air
{
	std::shared_ptr<DefaultGameMoviePlayer> DefaultGameMoviePlayer::mMoviePlayer;

	std::shared_ptr<DefaultGameMoviePlayer> DefaultGameMoviePlayer::get()
	{
		if (!mMoviePlayer)
		{
			mMoviePlayer = MakeSharedPtr<DefaultGameMoviePlayer>();
		}
		return mMoviePlayer;
	}

	DefaultGameMoviePlayer::~DefaultGameMoviePlayer()
	{

	}

	void DefaultGameMoviePlayer::initialize()
	{
		if (mInitialized)
		{
			return;
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(RegisterMoviePlayerTickable, DefaultGameMoviePlayer*, mMoviePlayer, this, {
			mMoviePlayer->registerTickable();
		});
		mInitialized = true;

		if (!PlatformProperties::requiresCookedData())
		{
			TArray<int32> mShaderMapIds;
			//mShaderMapIds.push_back(Globa)
		}

		std::shared_ptr<SWindow> GameWindow = GameEngine::createGameWindow();

		std::shared_ptr<SViewport> movieViewport;

		mLoadingScreenWindowPtr = GameWindow;
	}

	bool DefaultGameMoviePlayer::isMovieCurrentlyPlaying() const
	{
		return mSyncMechanism != nullptr;
	}

	bool DefaultGameMoviePlayer::loadingScreenIsPrepared() const
	{
		return false;
	}

	bool DefaultGameMoviePlayer::playMovie() 
	{
		bool beganPlaying = false;
		return true;
	}
	void DefaultGameMoviePlayer::setSlateRenderer(std::shared_ptr<SlateRenderer> inSlateRenderer)
	{
		mRendererPtr = inSlateRenderer;
	}

	void DefaultGameMoviePlayer::passLoadingScreenWindowBackToGame() const
	{
		GameEngine* gameEngine = dynamic_cast<GameEngine*>(GEngine.get());
		if (mLoadingScreenWindowPtr.lock())
		{
			gameEngine->mGameViewportWindow = mLoadingScreenWindowPtr;
		}
		else
		{

		}
	}

	void DefaultGameMoviePlayer::waitForMovieToFinish()
	{
		GameEngine* gameEngine = dynamic_cast<GameEngine*>(GEngine.get());
		if (gameEngine)
		{
			gameEngine->switchGameWindowToUseGameViewport();
		}
	}
}