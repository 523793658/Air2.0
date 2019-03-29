#pragma once
#include "CoreType.h"
#include "Math/Vector.h"
#include "Containers/String.h"
namespace Air
{
	enum class EWindowType
	{
		Normal,
		Menu,
		ToolTip,
		Notification,
		CursorDecorator,
		GameWindow
	};

	enum class EWindowTransparency
	{
		None,
		PerWindow,
		PerPixel
	};

	struct CORE_API GenericWindowDefinition
	{
		EWindowType mType;

		int2 DesiredPositionOnScree;

		int2 SizeDesiredOnScreen;

		EWindowTransparency TransparencySupport;

		bool HasOSWindowBorder;
		bool AppearsInTaskBar;
		bool IsTopmostWindow;
		bool AcceptsInput;

		bool ActivateWhenFirstShown;
		bool FocusWhenFirstShown;

		bool HasCloseButton;
		bool SupportsMinimize;

		bool SupportsMaximize;

		bool IsModalWindow;
		bool IsRegularWindow{ false };

		bool HasSizingFrame;

		bool SizeWillChangeOften;
		bool ShouldPreserveAspectRatio;

		int32 ExpectedMaxWidth;
		int32 ExpectedMaxHeight;

		wstring Title;

		float Opacity;

		int32 CornerRadius;
	};

}