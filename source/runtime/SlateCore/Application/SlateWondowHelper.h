#pragma once
#include "Widgets/SWindow.h"
#include "Layout/WidgetPath.h"
namespace Air
{
	class SLATE_CORE_API SlateWindowHelper
	{
	public:
		static std::shared_ptr<SWindow> findWindowByPlatformWindow(const TArray<std::shared_ptr<SWindow>>& windowsToSearch, const std::shared_ptr<GenericWindow>& platformWindow);
	
		static bool findPathToWidget(const TArray<std::shared_ptr<SWindow>>& windowsToSearch, std::shared_ptr<const SWidget> inWidget, WidgetPath& outWidgetPath);
	};
}