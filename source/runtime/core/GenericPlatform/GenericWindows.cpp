#include "GenericPlatform/GenericWindows.h"
#include "HAL/PlatformMisc.h"
namespace Air
{
	GenericWindow::GenericWindow()
	{

	}

	GenericWindow::~GenericWindow()
	{

	}

	float GenericWindow::getDPIScaleFactor() const
	{
		return 1.0f;
	}

	int32 GenericWindow::getWindowTitleBarSize() const
	{
		return 0;
	}


	EWindowMode::Type GenericWindow::getWindowMode() const
	{
		return EWindowMode::Windowed;
	}

	void GenericWindow::setWindowMode(EWindowMode::Type inNewWindowMode)
	{

	}

	void GenericWindow::adjustCachedSize(uint2 size)
	{

	}

	void* GenericWindow::getOSWindowHandle() const
	{
		return PlatformMisc::getHardwareWindow();
	}
	void GenericWindow::reshapeWindow(int32 x, int32 y, int32 width, int32 height)
	{

	}

	bool GenericWindow::isMaximized() const
	{
		return false;
	}

	void GenericWindow::restore()
	{

	}


	void GenericWindow::setOpacity(const float inOpacity)
	{

	}

	int32 GenericWindow::getWindowBorderSize() const
	{
		return 0;
	}

	void GenericWindow::maximize()
	{

	}

	void GenericWindow::minimize()
	{

	}
	void GenericWindow::show()
	{

	}

	void GenericWindow::bringToFront(bool bForce)
	{

	}

	bool GenericWindow::isWindowMinimized() const
	{
		return false;
	}

	bool GenericWindow::isVisible() const
	{
		return true;
	}

	const GenericWindowDefinition& GenericWindow::getDefinition() const
	{
		return *mDefinition.get();
	}
}