#include "MoviePlayer.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "DefaultGameMoviePlayer.h"
#include "NullMoviePlayer.h"
namespace Air
{

	bool isMoviePlayerEnabled()
	{
		bool bEnable = !GIsEditor && !GIsDemo && GUseThreadedRendering;
		return bEnable;
	}

	std::shared_ptr<IGameMoviePlayer> getMoviePlayer()
	{
		if (!isMoviePlayerEnabled() || GUsingNullRHI)
		{
			return NullGameMoviePlayer::get();
		}
		else
		{
			return DefaultGameMoviePlayer::get();
		}
	}
}