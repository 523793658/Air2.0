#pragma once
#include "MoviePlayer.h"
#include "TickableObjectRenderThread.h"

namespace Air
{
	class DefaultGameMoviePlayer : public IGameMoviePlayer, public TickableObjectRenderThread, public std::enable_shared_from_this<DefaultGameMoviePlayer>
	{
	public:
		static std::shared_ptr<DefaultGameMoviePlayer> get();
		~DefaultGameMoviePlayer();

		void initialize() override;

		bool isMovieCurrentlyPlaying() const override;

		virtual bool playMovie() override;

		bool loadingScreenIsPrepared() const;

		virtual void setSlateRenderer(std::shared_ptr<SlateRenderer> inSlateRenderer) override;

		virtual void passLoadingScreenWindowBackToGame() const override;

		virtual void waitForMovieToFinish() override;

	private:
		static std::shared_ptr<DefaultGameMoviePlayer> mMoviePlayer;

		bool mInitialized{ false };

		class SlateLoadingSynchronizationMechanism* mSyncMechanism;

		std::weak_ptr<SlateRenderer> mRendererPtr;

		std::weak_ptr<class SWindow> mLoadingScreenWindowPtr;
	};
}