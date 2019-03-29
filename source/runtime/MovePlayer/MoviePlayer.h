#pragma once
#include "MoviePlayerConfig.h"


namespace Air
{
	struct MOVIE_PLAYER_API LoadingScreenAttributes
	{
		LoadingScreenAttributes()
		{

		}
		std::shared_ptr<class SWidget> mWidgetLoadingScreen;

		TArray<wstring> mMoviePaths;
	};

	class SlateRenderer;
	class IGameMoviePlayer
	{
	public:
		virtual bool isMovieCurrentlyPlaying() const = 0;

		virtual void initialize() = 0;

		virtual bool playMovie() = 0;

		virtual void waitForMovieToFinish() = 0;

		virtual void setSlateRenderer(std::shared_ptr<SlateRenderer> inSlateRenderer) = 0;

		virtual void passLoadingScreenWindowBackToGame() const = 0;
	};

	std::shared_ptr<IGameMoviePlayer> MOVIE_PLAYER_API getMoviePlayer();
}