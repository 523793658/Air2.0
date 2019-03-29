#pragma once
#include "Slate.h"
#include "Widgets/SCompoundWidget.h"
#include "Rendering/RenderingCommon.h"
namespace Air
{

	class SLATE_API SViewport
		: public SCompoundWidget
	{
	public:
		void setViewportInterface(std::shared_ptr<ISlateViewport> inViewportInterface)
		{
			mViewportInterface = inViewportInterface;
		}

		std::weak_ptr<ISlateViewport> getViewportInterface()
		{
			return mViewportInterface;
		}

		void setActive(bool bActive);

		virtual Reply onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseWheel(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual void onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual void onMouseLeave(const PointerEvent& mouseEvent) override;

		virtual Reply onKeyDown(const Geometry& myGeometry, const KeyEvent& inKeyEvent) override;

		virtual Reply onKeyUp(const Geometry& myGeometry, const KeyEvent& inKeyEvent) override;

		virtual void onFinishedPointerInput() override;

	private:
		std::weak_ptr<ISlateViewport> mViewportInterface;
	};
}