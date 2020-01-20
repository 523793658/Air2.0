#include "Viewport.h"
#include "Classes/Engine/World.h"
#include "AirClient.h"
#include "RenderingThread.h"
#include "Classes/Engine/LocalPlayer.h"
#include "Classes/Engine/Engine.h"
#include "DynamicRHI.h"
#include "HAL/PlatformTime.h"
#include "CanvasType.h"
#include "RHI.h"
#include "RenderResource.h"
#include "DynamicRHI.h"
namespace Air
{
	struct EndDrawingCommandParams
	{
		Viewport* mViewport;
		bool bLockToVsync : 1;
		bool bShouldTriggerTimerEvent : 1;
		bool bShouldPresent : 1;
	};

	static void viewportEndDrawing(RHICommandListImmediate& RHICmdList, EndDrawingCommandParams parameters)
	{

	}

	

	Viewport::Viewport(ViewportClient* inViewportClient)
		:mViewportClient(inViewportClient)
	{

	}

	void Viewport::invalidateHitProxy()
	{
		mHitProxiesCached = false;
	}

	void Viewport::invalidate()
	{
		deferInvalidateHitProxy();
		invalidateDisplay();
	}

	void Viewport::deferInvalidateHitProxy()
	{
		invalidateHitProxy();
	}

	void Viewport::invalidateDisplay()
	{
		if (mViewportClient != nullptr)
		{
			mViewportClient->redrawRequested(this);
		}
	}

	void Viewport::draw(bool bShouldPresent /* = true */)
	{
		World* world = getClient()->getWorld();

		static std::unique_ptr<SuspendRenderingThread> GRenderingThreadSuspension;

		static bool bReentrant = false;
		if (!bReentrant)
		{
			if (world && world->isGameWorld() && !bIsGameRenderingEnable)
			{
				world->updateLevelStreaming();
			}
			else
			{
				if (mWidth > 0 && mHeight > 0)
				{
					bool bLockToVsync = false;

					LocalPlayer* player = (GEngine && world) ? GEngine->getFirstGamePlayer(world) : nullptr;
					if (player)
					{

					}

					enqueueBeginRenderFrame();

					{
						static uint32 lasttimestamp = 0;
						static bool bStarted = false;
						uint32 currentTime = PlatformTime::seconds();
						
					}

					World* viewportWorld = mViewportClient->getWorld();
					Canvas canvas(this, viewportWorld, viewportWorld ? viewportWorld->mFeatureLevel : GMaxRHIFeatureLevel);

					canvas.setRenderTargetRect(IntRect(0, 0, static_cast<int>(mWidth), static_cast<int>(mHeight)));
					{
						mViewportClient->draw(this, &canvas);
					}
					canvas.flush_GameThread();
					mViewportClient->processScreenShots(this);

					setRequiresVsync(false);
					EndDrawingCommandParams params = { this, bLockToVsync, false, bShouldPresent };
					ENQUEUE_RENDER_COMMAND(
						EndDrawingCommand)([params](RHICommandListImmediate& RHICmdList)
						{
							viewportEndDrawing(RHICmdList, params);
						});
					
				}
			}
			if (world)
			{

			}

			flushRenderingCommands();
			
		}
	}

	void Viewport::enqueueBeginRenderFrame()
	{
		advanceFrameRenderPrerequisite();
		ENQUEUE_RENDER_COMMAND(
			BeginDrawingCommand)([this](RHICommandListImmediate& RHICmdList)
			{
				this->beginRenderFrame(RHICmdList);
			});
	}

	void Viewport::beginRenderFrame(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(isInRenderingThread());
		RHICmdList.beginDrawingViewport(getViewportRHI(), TextureRHIRef());
		updateRenderTargetSurfaceRHIToCurrentBackBuffer();
	}

	void Viewport::updateRenderTargetSurfaceRHIToCurrentBackBuffer()
	{
		if (mViewportRHI)
		{
			mRenderTargetTextureRHI = RHIGetViewportBackBuffer(mViewportRHI);
		}
	}

	void Viewport::updateViewportRHI(bool bDestroyed, uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode, EPixelFormat preferredPixelFormat)
	{
		(*GFlushStreamingFunc)();
		{
			SCOPED_SUSPEND_RENDERING_THREAD(true);
			mWidth = newSizeX;
			mHeight = newSizeY;
			mWindowMode = newWindowMode;
			beginReleaseResource(this);
			if (bDestroyed)
			{
				if (isValidRef(mViewportRHI))
				{
					mViewportRHI.safeRelease();
				}
			}
			else
			{
				if (isValidRef(mViewportRHI))
				{
					RHIResizeViewpor(mViewportRHI, mWidth, mHeight, isFullscreen(),
						preferredPixelFormat);
				}
				else
				{
					mViewportRHI = RHICreateViewport(getWindow(), mWidth, mHeight, isFullscreen(), EPixelFormat::PF_Unknown);
				}
				beginInitResource(this);
			}
		}
	}

	float Viewport::getDesiredAspectRatio() const
	{
		int2 size = getSizeXY();
		return (float)size.x / (float)size.y;
	}

	IntRect Viewport::calculateViewExtents(float aspectRatio, const IntRect& viewRect)
	{
		IntRect result = viewRect;
		const float currentWidth = viewRect.width();
		const float currentHeight = viewRect.height();
		const float adjustedAspectRatio = aspectRatio / (getDesiredAspectRatio() / ((float)getSizeXY().x / (float)getSizeXY().y));
		const float aspectRatioDefference = adjustedAspectRatio - (currentWidth / currentHeight);

		if (std::abs(aspectRatioDefference) > 0.01f)
		{
			if (aspectRatioDefference > 0.0f)
			{
				const int32 newHeight = std::max<int32>(1, round(currentHeight / adjustedAspectRatio));
				result.min.y = round(0.5f * (currentHeight - newHeight));

				result.max.y = result.min.y + newHeight;
				result.min.y += viewRect.min.y;
				result.max.y += viewRect.min.y;
			}
			else
			{
				const int32 newWidth = std::max<int32>(1, round(currentHeight * adjustedAspectRatio));
				result.min.x = round(0.5f * (currentWidth - newWidth));
				result.max.x = result.min.x + newWidth;
				result.min.x += viewRect.min.x;
				result.max.x += viewRect.min.x;
			}
		}
		return result;
	}
}
