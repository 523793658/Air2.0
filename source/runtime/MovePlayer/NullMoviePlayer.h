#pragma once
#include "MoviePlayer.h"
namespace Air
{
	class NullGameMoviePlayer : public IGameMoviePlayer, public std::enable_shared_from_this<NullGameMoviePlayer>
	{
	public:	
		static std::shared_ptr<NullGameMoviePlayer> get();

		bool isMovieCurrentlyPlaying() const override { return false; }

		virtual bool playMovie() override { return false; }

		virtual void waitForMovieToFinish() override {}

		virtual void initialize() override {}

		virtual void setSlateRenderer(std::shared_ptr<SlateRenderer> inSlateRenderer) override {}

		virtual void passLoadingScreenWindowBackToGame() const override {}
	private:
		static std::shared_ptr<NullGameMoviePlayer> mMoviePlayer;
	};
}
