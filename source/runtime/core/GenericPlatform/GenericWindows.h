#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericWindowDefinition.h"
namespace Air
{

	namespace EWindowMode
	{
		enum Type
		{
			Fullscreen,

			WindowedFullscreen,

			Windowed,

			NumWindowModes
		};
	}

	class CORE_API GenericWindow
	{
	public:
		GenericWindow();

		virtual ~GenericWindow();

		virtual void setWindowMode(EWindowMode::Type inNewWindowMode);

		virtual EWindowMode::Type getWindowMode() const;

		virtual void adjustCachedSize(uint2 size);

		virtual void* getOSWindowHandle() const;

		virtual bool isMaximized() const;

		virtual void restore() ;

		virtual void reshapeWindow(int32 x, int32 y, int32 width, int32 height);

		virtual void setOpacity(const float inOpacity) ;

		virtual int32 getWindowBorderSize() const ;

		virtual void maximize();

		virtual void minimize();

		virtual void show();

		virtual void bringToFront(bool bForce = false);

		virtual float getDPIScaleFactor() const;

		virtual int32 getWindowTitleBarSize() const;

		virtual bool isVisible() const;

		virtual bool isWindowMinimized() const;

		virtual const GenericWindowDefinition& getDefinition() const;
	protected:
		std::shared_ptr<GenericWindowDefinition> mDefinition;
	};

}