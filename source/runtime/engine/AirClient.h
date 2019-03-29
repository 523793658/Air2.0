#pragma once
#include "EngineMininal.h"
#include "Viewport.h"
#include "Classes/Engine/World.h"
#include "GenericPlatform/GenericWindows.h"
#include "Input/Events.h"
#include "GenericPlatform/ICursor.h"
namespace Air
{

	class Canvas;
	enum EStereoscopicPass
	{
		eSSP_FULL,
		eSSP_LEFT_EYE,
		eSSP_RIGHT_EYE,
		eSSP_MONOSCROPIC_EYE
	};

	class ViewportFrame
	{
	public:
		virtual Viewport* getViewport() = 0;

		virtual void resizeFrame(int32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode) = 0;

		void resizeFrame(int32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode, int32, int32)
		{
			resizeFrame(newSizeX, newSizeY, newWindowMode);
		}
	};

	class ENGINE_API ViewportClient
	{
	public:
		struct ESoundShowFlags
		{

		};

		virtual void draw(Viewport* viewport, Canvas* canvas) {}

		virtual World* getWorld() const { return nullptr; }

		virtual void processScreenShots(Viewport* viewport) {}

		virtual void redrawRequested(Viewport* viewport) { viewport->draw(); }

		virtual bool inputKey(Viewport* viewport, int32 controllerId, Key key, EInputEvent e, float amountDepressed = 1.0f, bool bGamepad = false) { return false; }

		virtual bool inputAxis(Viewport* viewport, int32 controllerId, Key key, float delta, float deltaTime, int32 numSamples = 1, bool bGamepad = false) { return false; }

		virtual bool ignoreInput()
		{
			return false;
		}

		virtual EMouseCursor::Type getCursor(Viewport* viewport, int32 x, int32 y)
		{
			return EMouseCursor::Default;
		}

		virtual bool shouldAlwaysLockMouse() { return false; }

		virtual void mouseEnter(Viewport* viewport, int32 x, int32 y) {}

		virtual void mouseLeave(Viewport* viewport) {}

		virtual void mouseMove(Viewport* viewport, int32 x, int32 y){}

		virtual bool lockDuringCapture() { return true; }

		virtual bool hideCursorDuringCapture() { return false; }

		virtual EMouseCaptureMode captureMouseOnClick() { return EMouseCaptureMode::CapturePermanently; }

		virtual void capturedMouseMove(Viewport* inViewport, int32 inMouseX, int32 inMouseY) {}
	public:
		Viewport* mViewport;
	};

	class ENGINE_API CommonViewportClient : public ViewportClient
	{
	public:
		virtual EMouseCaptureMode captureMouseOnClick() override
		{
			return mMouseCaptureMode;
		}



		

	protected:

		EMouseCaptureMode mMouseCaptureMode;

	};
}
