#include "NullMoviePlayer.h"
namespace Air
{
	std::shared_ptr<NullGameMoviePlayer> NullGameMoviePlayer::mMoviePlayer;

	std::shared_ptr<NullGameMoviePlayer> NullGameMoviePlayer::get()
	{
		if (!mMoviePlayer)
		{
			mMoviePlayer = MakeSharedPtr<NullGameMoviePlayer>();
		}
		return mMoviePlayer;
	}
}