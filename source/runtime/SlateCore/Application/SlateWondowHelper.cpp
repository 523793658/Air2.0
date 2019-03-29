#include "Application/SlateWondowHelper.h"
namespace Air
{
	std::shared_ptr<SWindow> SlateWindowHelper::findWindowByPlatformWindow(const TArray<std::shared_ptr<SWindow>>& windowsToSearch, const std::shared_ptr<GenericWindow>& platformWindow)
	{
		for (int32 windowIndex = 0; windowIndex < windowsToSearch.size(); ++windowIndex)
		{
			std::shared_ptr<SWindow> someWindow = windowsToSearch[windowIndex];
			std::shared_ptr<GenericWindow> someNativeWindow = someWindow->getNativeWindow();
			if (someNativeWindow == platformWindow)
			{
				return someWindow;
			}
		}
		return std::shared_ptr<SWindow>();
	}

	bool SlateWindowHelper::findPathToWidget(const TArray<std::shared_ptr<SWindow>>& windowsToSearch, std::shared_ptr<const SWidget> inWidget, WidgetPath& outWidgetPath)
	{
		bool bFoundWidget = false;
		for (int32 windowIndex = 0; !bFoundWidget && windowIndex < windowsToSearch.size(); ++windowIndex)
		{
			std::shared_ptr<SWindow> curWindow = windowsToSearch[windowIndex];
			if (curWindow == inWidget)
			{
				bFoundWidget = true;
				
			}
		}
		return bFoundWidget;
	}
}