#include "HAL/PlatformProcess.h"
#include "SlateRHIRenderer.h"
#include "Rendering/ElementBatcher.h"
#include "RenderingThread.h"
#include "SlateRHIRenderingPolicy.h"
#include "DynamicRHI.h"

namespace Air
{
	void SlateRHIRenderer::createViewport(const std::shared_ptr<SWindow> inWindow)
	{
		flushRenderingCommands();
		if (mWindowToViewportInfo.find(inWindow.get()) == mWindowToViewportInfo.end())
		{
			const uint2 windowSize = inWindow->getViewportSize();
			const uint32 width = std::max<uint32>(8, windowSize.x);
			const uint32 height = std::max<uint32>(8, windowSize.y);

			ViewportInfo* newInfo = new ViewportInfo();
			std::shared_ptr<GenericWindow> nativeWindow = inWindow->getNativeWindow();
			newInfo->mOSWindow = nativeWindow->getOSWindowHandle();
			newInfo->width = width;
			newInfo->height = height;
			newInfo->mDesiredWidth = width;
			newInfo->mDesiredHeight = height;
			newInfo->mPixelFormat = PF_B8G8R8A8;
			newInfo->mProjectionMatrix = createProjectionMatrix(width, height);
			bool fullscreen = isViewportFullscreen(*inWindow);
			newInfo->mViewportRHI = RHICreateViewport(newInfo->mOSWindow, width, height, fullscreen, newInfo->mPixelFormat);
			newInfo->mFullscreen = fullscreen;
			mWindowToViewportInfo.emplace(inWindow.get(), newInfo);
			beginInitResource(newInfo);
		}
	}

	void SlateRHIRenderer::updateFullscreenState(const std::shared_ptr<SWindow> inWindow, uint32 overrideResX /* = 0 */, uint32 overrideResY /* = 0 */)
	{

	}

	bool SlateRHIRenderer::initialize()
	{
		mRenderingPolicy = MakeSharedPtr<SlateRHIRenderingPolicy>();

		mElementBatcher = MakeSharedPtr<SlateElementBatcher>(mRenderingPolicy);
		return true;
	}

	void* SlateRHIRenderer::getViewportResource(const SWindow& window)
	{
		BOOST_ASSERT(isThreadSafeForSlateRendering());
		auto infoPtr = mWindowToViewportInfo.find(&window);
		if (infoPtr != mWindowToViewportInfo.end())
		{
			ViewportInfo* viewportInfo = infoPtr->second;
			if (!isValidRef(viewportInfo->mViewportRHI))
			{
				const bool fullScreen = isViewportFullscreen(window);
				viewportInfo->mViewportRHI = RHICreateViewport(viewportInfo->mOSWindow, viewportInfo->width, viewportInfo->height, viewportInfo->mFullscreen, viewportInfo->mPixelFormat);
			}
			return &viewportInfo->mViewportRHI;
		}
		else
		{
			return nullptr;
		}
		
	}

	Matrix SlateRHIRenderer::createProjectionMatrix(uint32 width, uint32 height)
	{
		const float left = 0;
		const float right = left + width;
		const float top = 0;
		const float bottom = top + height;
		const float zNear = -100.0f;
		const float ZFar = 100.0f;
		return adjustProjectionMatrixForRHI(Matrix(
			Plane(2.0f / (right - left), 0, 0, 0),
			Plane(0, 2.0f / (top - bottom), 0, 0),
			Plane(0, 0, 1 / (zNear - ZFar), 0),
			Plane((left + right) / (left - right), (top + bottom) / (bottom - top), zNear / (zNear - ZFar), 1.0)));
	}

	void SlateRHIRenderer::drawWindows()
	{

	}

	void SlateRHIRenderer::drawWindows(SlateDrawBuffer& windowDrawBuffer)
	{
		if (isInSlateThread())
		{
			
		}
		else
		{
			drawWindows_Private(windowDrawBuffer);
		}
	}

	void SlateRHIRenderer::drawWindows_Private(SlateDrawBuffer& windowDrawBuffer)
	{
		BOOST_ASSERT(isThreadSafeForSlateRendering());
		SlateRHIRenderingPolicy* policy = mRenderingPolicy.get();
		ENQUEUE_RENDER_COMMAND(
			slateBeginDrawingWindowsCommand)([policy](RHICommandListImmediate&)
			{
				policy->beginDrawingWindows();
			});

		TArray<std::shared_ptr<SlateWindowElementList>>& windowElementLists = windowDrawBuffer.getWindowElementLists();
		for (int32 listIndex = 0; listIndex < windowElementLists.size(); ++listIndex)
		{
			SlateWindowElementList& elementList = *windowElementLists[listIndex];
			std::shared_ptr<SWindow> window = elementList.getWindow();

			if (window)
			{
				const float2 windowSize = window->getViewportSize();
				if (windowSize.x > 0 && windowSize.y > 0)
				{
					mElementBatcher->addElements(elementList);
					bool requiresStencilTest = false;
					bool bLockToVsync = false;
					bLockToVsync = mElementBatcher->requirestVsync();
					bool bForceVsyncFromCVar = false;

					bLockToVsync |= bForceVsyncFromCVar;

					mElementBatcher->resetBatches();

					ViewportInfo* viewInfo = nullptr;
					auto it = mWindowToViewportInfo.find(window.get());
					if (it != mWindowToViewportInfo.end())
					{
						viewInfo = it->second;
					}
					if (window->isViewportSizeDrivenByWindow())
					{
						conditionalResizeViewport(viewInfo, viewInfo->mDesiredWidth, viewInfo->mDesiredHeight, isViewportFullscreen(*window));
					}
					if (mRequiresStencilTest)
					{
						
					}
					{
						struct SlateDrawWindowCommandParams 
						{
							SlateRHIRenderer* renderer;
							SlateRHIRenderer::ViewportInfo* viewportInfo;
							SlateWindowElementList* windowElementList;
							SWindow* slateWindow;
							bool lockToVsync;
							bool clear;
						}Params;
						Params.renderer = this;

						Params.viewportInfo = viewInfo;
						Params.windowElementList = &elementList;
						Params.lockToVsync = bLockToVsync;
						Params.clear = false;
						Params.slateWindow = window.get();
						if (GIsClient)
						{
							ENQUEUE_RENDER_COMMAND(
								slateDrawWindowsCommand)([Params](RHICommandListImmediate& RHICmdList)
								{
									Params.renderer->drawWindow_RenderThread(RHICmdList, *Params.viewportInfo, *Params.windowElementList, Params.lockToVsync, Params.clear);
								});
						}
					}
				}
			}
		}
		int x = 0;
		ENQUEUE_RENDER_COMMAND(
			slateEndDrawingWindowsCommand)([&windowDrawBuffer, policy](RHICommandListImmediate& RHICmdList)			
			{
				SlateEndDrawingWindowsCommand::endDrwingWindows(RHICmdList, &windowDrawBuffer, *policy);
			});

	}

	SlateDrawBuffer& SlateRHIRenderer::getDrawBuffer()
	{
		mFreeBufferIndex = (mFreeBufferIndex + 1) % NumDrawBuffers;
		SlateDrawBuffer* buffer = &mDrawBuffers[mFreeBufferIndex];
		while (!buffer->lock())
		{
			if (isInSlateThread())
			{
				PlatformProcess::sleep(0.001f);
			}
			else
			{
				flushCommands();
				mFreeBufferIndex = (mFreeBufferIndex + 1) % NumDrawBuffers;
			}

			buffer = &mDrawBuffers[mFreeBufferIndex];
		}
		buffer->clearBuffer();
		return *buffer;
	}

	void SlateRHIRenderer::flushCommands() const
	{
		if (isInGameThread())
		{
			flushRenderingCommands();
		}
	}

	void SlateEndDrawingWindowsCommand::endDrwingWindows(RHICommandListImmediate& RHICmdList, SlateDrawBuffer* drawBuffer, SlateRHIRenderingPolicy& policy)
	{
		if (!RHICmdList.bypass())
		{
			new (RHICmdList.allocCommand<SlateEndDrawingWindowsCommand>()) SlateEndDrawingWindowsCommand(policy, drawBuffer);
		}
		else
		{
			SlateEndDrawingWindowsCommand cmd(policy, drawBuffer);
			cmd.execute(RHICmdList);
		}
	}

	SlateEndDrawingWindowsCommand::SlateEndDrawingWindowsCommand(SlateRHIRenderingPolicy& inPolicy, SlateDrawBuffer* inDrawBuffer)
		:mPolicy(inPolicy),
		mDrawBuffer(inDrawBuffer)
	{

	}

	void SlateEndDrawingWindowsCommand::execute(RHICommandListBase& cmdList)
	{
		for (auto & elementList : mDrawBuffer->getWindowElementLists())
		{
			elementList->postDraw_ParallelThread();
		}
		mDrawBuffer->unlock();
		mPolicy.endDrawingWindows();

 	}

	void SlateRHIRenderer::conditionalResizeViewport(ViewportInfo* viewportInfo, uint32 width, uint32 height, bool bFullscreen)
	{

	}


	void SlateRHIRenderer::drawWindow_RenderThread(RHICommandListImmediate& RHICmdList, SlateRHIRenderer::ViewportInfo& viewportInfo, SlateWindowElementList& windowElementList, bool lockToVsync, bool bClear)
	{
		BOOST_ASSERT(isInRenderingThread());
		{
			SlateBatchData& batchData = windowElementList.getBatchData();
			ElementBatchMap& rootBatchMap = windowElementList.getRootDrawLayer().getElementBatchMap();
			windowElementList.postDraw_ParallelThread();
			{
			}
		}


		RHICmdList.endDrawingViewport(viewportInfo.mViewportRHI, true, lockToVsync);
	}

}